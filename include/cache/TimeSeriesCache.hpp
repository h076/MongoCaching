#ifndef TIMESERIESCACHE_H_
#define TIMESERIESCACHE_H_

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <utils/tsQueue.hpp>
#include <cache/Requests.hpp>

#include <boost/asio/io_context.hpp>

#include <redis/connectionPool.hpp>
#include <redis/TimeSeriesService.hpp>

#include <mongo/Connector.hpp>
#include <mongo/SpotService.hpp>

// Caching interface class to be run on a single thread.
// Using async operations to implement a read-through pattern.
// Operating on a redis time series server acting as a cache
// and a MongoDB cluster as the data source

namespace hjw {

    namespace cache {

        namespace net = boost::asio;
        namespace redis = hjw::redis;

        class TimeSeriesCache {

            public:

                TimeSeriesCache(const std::string& sourceCluster)
                    : m_ioc(), m_ctxGuard(net::make_work_guard(m_ioc)), m_running(false),
                      m_redisPool(nullptr), m_mongoConn(sourceCluster),
                      m_mongoSpotService(std::make_shared<mongo::Connector>(std::move(m_mongoConn)))
                {
                    m_mongoSpotService.set("StockData", "Spots");
                }

                ~TimeSeriesCache();

                // Enqueue a single request
                void enque(TimeSeriesRequest& req);

                // Run start the io context and run cache loop
                void run();

                // Stop cache loop, join io context
                void stop();

            private:

                // personal asio io context for async operations
                net::io_context m_ioc;

                // Dedicated io thread
                std::thread m_ctxThread;

                // work guard to keep cache thread alive
                net::executor_work_guard<net::io_context::executor_type> m_ctxGuard;

                bool m_running;

                // Redis connection pool
                redis::connectionPool * m_redisPool;

            private:

                // Thread safe queue to handle requests
                utils::tsqueue<TimeSeriesRequest> m_reqQueue;

                net::awaitable<void> requestHandler();

                net::awaitable<void> handleGet(TimeSeriesRequest& req);

            private:

                // Handle mongo connection with one connector
                mongo::Connector m_mongoConn;

                // Service to obtain historical data
                // This service should be made thread safe using
                mongo::SpotService m_mongoSpotService;
                std::mutex m_spotServiceMutex;

        };
    }
}

#endif // TIMESERIESCACHE_H_
