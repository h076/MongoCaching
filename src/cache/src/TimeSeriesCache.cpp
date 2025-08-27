
#include <cache/TimeSeriesCache.hpp>

using namespace hjw::cache;
using namespace hjw::redis;

TimeSeriesCache::~TimeSeriesCache() {

}

// taking the request as an rvalue and moving it to the queue
// the request must be moved as it contains std::promise which cannot be copied
void TimeSeriesCache::enque(TimeSeriesRequest&& r)  {
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

    // Spawn the request handler loop
    net::co_spawn(m_ioc, requestHandler(), net::detached);

    // Run io_context on dedicated thread
    m_ctxThread = std::thread([this]() {
        m_ioc.run();
    });
}

void TimeSeriesCache::stop() {
    m_accepting = false;

    // Wait for all active requests to finish
    std::unique_lock<std::mutex> lk(m_activeMtx);
    m_activeCv.wait(lk, [this] {return m_active.load(std::memory_order_relaxed) == 0;});

    std::cout << "all req processed " << std::endl;

    // tell request loop to stop
    m_running = false;
    // send false request to unwait queue
    TimeSeriesRequest fr{RequestType::SET, "", 0, 0, nullptr};
    m_reqQueue.push_back(std::move(fr));

    // allow io ctx to stop as guard is released
    m_ctxGuard.reset();
    // stop io context
    m_ioc.stop();
    // join dedicated thread
    while (!m_ctxThread.joinable())
        continue;
    m_ctxThread.join();
}

auto TimeSeriesCache::requestHandler() -> net::awaitable<void> {
    TimeSeriesRequest r;

    while (m_running) {
        // handle requests

        // use wait function rather than busy spining
        m_reqQueue.wait();

        if (!m_running)
            break;

        r = m_reqQueue.pop_front();

        if (r.type == RequestType::GET) {
            co_await handleGet(std::move(r));
        }
    }

    co_return;
}

auto TimeSeriesCache::handleGet(TimeSeriesRequest&& r) -> net::awaitable<void> {
    // handle request counter
    auto onExit = gsl::finally([this] {
        if (m_active.fetch_sub(1, std::memory_order_relaxed) == 1) {
            std::lock_guard<std::mutex> lk(m_activeMtx);
            m_activeCv.notify_all();
        }
    });

    auto redisConn = co_await m_redisPool->acquire();
    redis::TimeSeriesService tsService(redisConn);

    utils::series * s = co_await tsService.co_getSeries(r.symbol, r.from, r.to);

    if (s) {
        std::cout << "Cache hit, " << r.symbol << std::endl;
        r.getSeries->set_value(s);
        m_redisPool->release(std::move(redisConn));
        co_return;
    }

    std::cout << "Cache miss, " << r.symbol << std::endl;

    // Must retrieve data from mongoDB and update cache
    std::scoped_lock lock(m_spotServiceMutex);
    s = m_mongoSpotService.get(r.symbol, r.from, r.to);
    r.getSeries->set_value(s);

    co_await tsService.co_addSeries(s);

    m_redisPool->release(std::move(redisConn));

    co_return;
}
