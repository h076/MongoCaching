
#include "cache/Requests.hpp"
#include "redis/TimeSeriesService.hpp"
#include <cache/TimeSeriesCache.hpp>
#include <future>
#include <sys/types.h>

using namespace hjw::cache;
using namespace hjw::redis;

TimeSeriesCache::~TimeSeriesCache() {

}

// taking the request as an rvalue and moving it to the queue
// the request must be moved as it contains std::promise which cannot be copied
void TimeSeriesCache::enque(RequestTypeVariant&& r)  {
    if (m_accepting) {
        m_active.fetch_add(1, std::memory_order_relaxed);
        m_reqQueue.push_back(std::move(r));
    }
}

void TimeSeriesCache::run() {
    m_running = true;
    m_accepting = true;

    // Get context executor
    net::any_io_executor exec = m_ioc.get_executor();

    // create redis connection pool
    m_redisPool = new connectionPool(exec, "127.0.0.1", "6379", 5);

    // Use void promise to ensure safe cache stop
    m_reqHandlerPromise = std::make_shared<std::promise<void>>();
    std::shared_ptr<std::promise<void>> p_ptr = m_reqHandlerPromise;

    // Spawn the request handler loop
    net::co_spawn(m_ioc, [this, p_ptr] () -> net::awaitable<void> {
        co_await requestHandler();
        try {
            p_ptr->set_value(); // set value after handler has finished, unblock wait in stop()
        } catch (...) {}
        co_return;
    }, net::detached);

    // Run io_context on dedicated thread
    m_ctxThread = std::thread([this]() {
        m_ioc.run();
    });
}

void TimeSeriesCache::stop() {
    // Stop accepting requests
    m_accepting = false;

    // Wait for all active requests to finish
    std::unique_lock<std::mutex> lk(m_activeMtx);
    m_activeCv.wait(lk, [this] {return m_active.load(std::memory_order_relaxed) == 0;});

    // send false request to unwait queue
    TimeSeriesRequest<RequestType::STOP_CACHE> fr;
    m_reqQueue.push_back(std::move(fr));

    // wait for handler to finish
    if (m_reqHandlerPromise) {
        std::future<void> f = m_reqHandlerPromise->get_future();
        f.wait(); // blocks
        m_reqHandlerPromise.reset();
    }

    // teardown io_context / guard
    m_ctxGuard.reset();
    m_ioc.stop();

    // join the thread
    if (m_ctxThread.joinable())
        m_ctxThread.join();
}

auto TimeSeriesCache::requestHandler() -> net::awaitable<void> {
    RequestTypeVariant req;
    bool rtn = false;

    while (true) {
        // handle requests

        // use wait function rather than busy spining
        m_reqQueue.wait();

        req = m_reqQueue.pop_front();

        co_await std::visit([this, &rtn](auto&& r) -> net::awaitable<void> {
            using req = std::decay_t<decltype(r)>;
            if constexpr (req::type == RequestType::GET)
                co_await handleGet(std::move(r));
            else if constexpr (req::type == RequestType::SET)
                co_await handleSet(std::move(r));
            else if constexpr (req::type == RequestType::INFO)
                co_await handleInfo(std::move(r));
            else if constexpr (req::type == RequestType::STOP_CACHE)
                rtn = true;
        }, std::move(req));

        if (rtn)
            co_return;
    }
}

auto TimeSeriesCache::handleGet(TimeSeriesRequest<RequestType::GET>&& r) -> net::awaitable<void> {

    // handle request counter
    auto onExit = gsl::finally([this] {
        if (m_active.fetch_sub(1, std::memory_order_relaxed) == 1) {
            std::lock_guard<std::mutex> lk(m_activeMtx);
            m_activeCv.notify_all();
        }
    });

    auto redisConn = co_await m_redisPool->acquire();
    redis::TimeSeriesService tsService(redisConn);

    uint64_t from = r.from;
    uint64_t to = r.to;

    utils::series * s;

    // check if the series exists in cache
    bool exists = co_await tsService.co_exists(r.symbol);
    if (!exists) {
        std::cout << "Cache Miss : " << r.symbol << ", " << r.from << " : " << r.to << std::endl;
        s = co_await handleMiss(r.symbol, from, to, &tsService);
        r.series->set_value(s);
        m_redisPool->release(std::move(redisConn));
        co_return;
    }

    constexpr uint64_t epoch_12_hours = 43200000;

    // check that the request time is within existing bounds
    uint64_t leftBound = co_await tsService.co_first_ts(r.symbol) - epoch_12_hours;
    uint64_t rightBound = co_await tsService.co_latest_ts(r.symbol) + epoch_12_hours;

    // handle misses ....
    if (leftBound > to || rightBound < from)
    {
        // Complete miss : leftBound > to || rightBound < from
        std::cout << "Cache Miss : " << r.symbol << ", " << from << " : " << to << std::endl;
        s = co_await handleMiss(r.symbol, r.from, r.to, &tsService);
    } else if (leftBound > from)
    {
        // Partial miss at series start : leftBound > from
        std::cout << "Partial Miss at start : " << r.symbol << ", " << from << " : " << leftBound << std::endl;
        co_await handleMiss(r.symbol, from - epoch_12_hours, leftBound, &tsService);
        s = co_await tsService.co_getSeries(r.symbol, from, to);
    } else if (rightBound < to)
    {
        // Partial miss at series end : rightBount < to
        std::cout << "Partial Miss at end : " << r.symbol << ", " << rightBound << " : " << to << std::endl;
        co_await handleMiss(r.symbol, rightBound, to + epoch_12_hours, &tsService);
        s = co_await tsService.co_getSeries(r.symbol, from, to);
    } else
    {
        std::cout << "Cache Hit : " << r.symbol << ", " << from << " : " << to << std::endl;
        s = co_await tsService.co_getSeries(r.symbol, from, to);
    }

    r.series->set_value(s);
    m_redisPool->release(std::move(redisConn));
    co_return;
}

auto TimeSeriesCache::handleSet(TimeSeriesRequest<RequestType::SET>&& req) -> net::awaitable<void> {
    // Yet to implement
    co_return;
}

auto TimeSeriesCache::handleInfo(TimeSeriesRequest<RequestType::INFO>&& req) -> net::awaitable<void> {
    // Want to query the cache to see if a ticket exists
    // handle request counter
    auto onExit = gsl::finally([this] {
        if (m_active.fetch_sub(1, std::memory_order_relaxed) == 1) {
            std::lock_guard<std::mutex> lk(m_activeMtx);
            m_activeCv.notify_all();
        }
    });

    auto redisConn = co_await m_redisPool->acquire();
    redis::TimeSeriesService tsService(redisConn);

    // check if the series exists
    bool cacheExists = co_await tsService.co_exists(req.symbol);

    if (!cacheExists) {
        req.exists->set_value(co_await handleMissInfo(req.symbol));
    }else {
        req.exists->set_value(true);
    }
    co_return;
}

auto TimeSeriesCache::handleMissInfo(const std::string& symbol) -> net::awaitable<bool> {
    // Check if the series exists in the mongo cluster
    utils::seriesInfo * si = m_mongoSpotService.info(symbol);
    if (si != nullptr) {
        free(si);
        co_return true;
    }
    co_return false;
}

auto TimeSeriesCache::handleMiss(const std::string& symbol, const uint64_t from, const uint64_t to,
                                 redis::TimeSeriesService * tss) -> net::awaitable<utils::series *> {
    // Use lock to enforce mutual exclusion
    std::scoped_lock lock(m_spotServiceMutex);

    // retreive data from mongoDB
    utils::series * s = m_mongoSpotService.get(symbol, from, to);

    if (s == nullptr)
        co_return s;

    co_await tss->co_addSeries(s);

    co_return s;
}
