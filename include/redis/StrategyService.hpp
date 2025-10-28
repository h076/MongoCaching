#ifndef STRATEGYSERVICE_H_
#define STRATEGYSERVICE_H_

#include <boost/redis/connection.hpp>
#include <boost/asio.hpp>

#include "utils/hjw_utils.hpp"

namespace hjw {

    namespace redis {

        namespace net = boost::asio;

        // Will be used to write strategy results
        // And retrieve strategy results

        class StrategyService {

            public:

                StrategyService(std::shared_ptr<boost::redis::connection> conn) : m_conn(std::move(conn)) {}

                ~StrategyService() {}

                // Add test report to the redis db
                // Should tae ptr to task and free as strat thread is fired and forgotten
                net::awaitable<void> co_add(utils::testReport* report);

                // Get test report from redis db
                net::awaitable<utils::testReport*> co_get(int testId);

                // Check if test exists
                net::awaitable<bool> co_exists(int testId);

            private:

                std::shared_ptr<boost::redis::connection> m_conn;

        };
    }
}

#endif // STRATEGYSERVICE_H_
