#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_

#include <boost/asio.hpp>
#include <boost/redis.hpp>
#include <boost/asio/experimental/channel.hpp>

#include <iostream>
#include <deque>
#include <memory>
#include <optional>

namespace hjw {

    namespace redis {

        namespace net = boost::asio;
        using tcp = net::ip::tcp;

        // Manages a pool of asynchronous redis connections
        // Allows multiple consumers to acuqire and release in a async structure
        class connectionPool {

            public :

                // net::any_io_executor : thread context for async tasks
                // tcp::resolver : resolve host and port end points
                connectionPool(net::any_io_executor exec, tcp::resolver& resolver, const std::string& host,
                               const std::string& port, std::size_t pool_size)
                    : m_executor(std::move(exec)), m_poolSize(pool_size)
                {
                    // Start async task to establish all connections
                    for (std::size_t i = 0; i < m_poolSize; i++)
                        createConnection(resolver, host, port);
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
                void createConnection(tcp::resolver& resolver, const std::string& host, const std::string& port) {
                    // Spawn a coroutine on the executor using co_spawn
                    net::co_spawn(m_executor,
                                  [this, &resolver, host, port]() -> net::awaitable<void> {
                                      tcp::socket socket(m_executor);
                                      // Resolve endpoints
                                      auto endpoints = co_await resolver.async_resolve(host, port, net::use_awaitable);

                                      // Asynchronously connect to redis
                                      co_await net::async_connect(socket, endpoints, net::use_awaitable);

                                      // Create connection using the socket
                                      auto conn = std::make_shared<boost::redis::connection>(std::move(socket));

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
                                self.complete(std::move(conn));
                            } else {
                                // Save the handler to be resumed later
                                pool.m_waiters.push_back(
                                    [h = std::move(self)](std::shared_ptr<boost::redis::connection> conn) mutable {
                                        h.complete(std::move(conn));
                                    });
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
