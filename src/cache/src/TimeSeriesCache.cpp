#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <cache/TimeSeriesCache.hpp>

using namespace hjw::cache;
using namespace hjw::redis;

void TimeSeriesCache::run() {
    m_running = true;

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
    m_running = false;

    // allow io ctx to stop as guard is released
    m_ctxGuard.reset();

    m_ioc.stop();

    if (m_ctxThread.joinable())
        m_ctxThread.join();

}
