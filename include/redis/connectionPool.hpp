#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_

#include <boost/asio.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/detached.hpp>

#include <boost/redis/src.hpp>
#include <boost/redis/logger.hpp>
#include <boost/redis/connection.hpp>
#include <boost/redis/config.hpp>

#include <iostream>
#include <deque>
#include <memory>
#include <optional>

// Note on optimisation
// C++14 and later has return value optimisation
// So all pointers unique / shared should not be returne wrapped in move
// Instead the call to the returning function should be wrappe in a move

namespace hjw {

    namespace redis {

        namespace net = boost::asio;
        using tcp = net::ip::tcp;
        using boost::redis::connection;
        using boost::redis::logger;

        // Manages a pool of asynchronous redis connections
        // Allows multiple consumers to acuqire and release in a async structure
        class connectionPool {

            public :

                // net::any_io_executor : thread context for async tasks
                connectionPool(net::any_io_executor exec, const std::string& host,
                               const std::string& port, std::size_t pool_size)
                    : m_executor(std::move(exec)), m_poolSize(pool_size)
                {
                    // Start async task to establish all connections
                    for (std::size_t i = 0; i < m_poolSize; i++)
                        createConnection(host, port);
                }

                ~connectionPool() {
                    // Users responsibility to release all connections
                    while (!m_connections.empty()) {
                        std::shared_ptr<connection> conn = std::move(m_connections.front());
                        m_connections.pop_front();
                        if(conn)
                            conn->cancel();
                    }
                }

                // Acquire a connection asynchronously
                net::awaitable<std::shared_ptr<boost::redis::connection>> acquire() {
                    // Suspend coroutine until a connection is avaliable
                    co_return co_await waitForConnection();
                }

                // Return the connection to the pool
                void release(std::shared_ptr<boost::redis::connection> conn) {
                    // Push the connection back into the pool and notify waiters
                    net::dispatch(m_executor,
                                  [this, conn = std::move(conn)]() mutable {
                                      if (!m_waiters.empty()) {
                                          auto waiter = std::move(m_waiters.front());
                                          m_waiters.pop_front();
                                          waiter(std::move(conn));
                                      } else {
                                          m_connections.push_back(std::move(conn));
                                      }
                                  });
                }

            private:

                net::any_io_executor m_executor;
                std::size_t m_poolSize;

                std::deque<std::shared_ptr<boost::redis::connection>> m_connections;
                std::deque<std::function<void(std::shared_ptr<boost::redis::connection>)>> m_waiters;

                // Creates and connects a new redis connection and pushes to pool
                void createConnection(const std::string& host, const std::string& port) {
                    // Spawn a coroutine on the executor using co_spawn
                    net::co_spawn(m_executor,
                                  [this, host, port]() -> net::awaitable<void> {
                                      // make connection with executor
                                      auto conn = std::make_shared<connection>(m_executor);

                                      // Run connection within config
                                      boost::redis::config cfg;
                                      logger lgr{logger::level::err};
                                      conn->async_run(cfg, lgr, net::consign(net::detached, conn));

                                      // Store the connection using dispatch for thread-safety
                                      // This ensures we are on the thread of the executor
                                      net::dispatch(m_executor, [this, conn]() {
                                          if (!m_waiters.empty()) {
                                              auto waiter = std::move(m_waiters.front());
                                              m_waiters.pop_front();
                                              waiter(std::move(conn));
                                          } else {
                                              m_connections.push_back(std::move(conn));
                                          }
                                      });

                                      co_return;
                                  },  net::detached);
                }

                template <typename CompletionToken>
                auto awaitable(connectionPool& pool, boost::asio::any_io_executor ex, CompletionToken&& token) {
                    // Custom compose opration to enque lambda in the waiters queue
                    return net::async_compose<CompletionToken, void(std::shared_ptr<boost::redis::connection>)>(
                        [&pool](auto& self) mutable {
                            if (!pool.m_connections.empty()) {
                                auto conn = std::move(pool.m_connections.front());
                                pool.m_connections.pop_front();
                                self.complete(conn);
                            } else {
                                // Save the handler to be resumed later
                                using HandlerType = std::remove_reference_t<decltype(self)>;
                                auto handler = std::make_shared<HandlerType>(std::move(self));
                                pool.m_waiters.push_back([handler](std::shared_ptr<boost::redis::connection> conn) mutable {
                                    (*handler).complete(conn);
                                });
                                //pool.m_waiters.push_back(
                                    //[h = std::move(self)](std::shared_ptr<boost::redis::connection> conn) mutable {
                                        //h.complete(std::move(conn));
                                    //});
                            }
                        },
                        token,
                        ex);
                }



                // Wait until a connection is avaliable, or suspended coroutine
                net::awaitable<std::shared_ptr<boost::redis::connection>> waitForConnection() {
                    // Get current executor to ensure thread safety
                    auto ex = co_await net::this_coro::executor;
                    co_return co_await awaitable(*this, ex, net::use_awaitable);
                }
        };
    }
}

#endif // CONNECTIONPOOL_H_
