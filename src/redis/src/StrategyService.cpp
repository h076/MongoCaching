#include "redis/StrategyService.hpp"

using namespace hjw::redis;
using namespace boost::redis;

auto StrategyService::co_add(hjw::utils::testReport* report) -> net::awaitable<void> {
    // Get report as json
    std::string rj = reportToJsonString(report);

    // Add to redis
    request req;
    req.push("SET", "test:"+std::to_string(report->tId), rj);

    free(report);

    generic_response resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    co_return;
}

auto StrategyService::co_get(int testId) -> net::awaitable<hjw::utils::testReport*> {
    // Ensure the test exists otherwise return null report
    bool exists = co_await co_exists(testId);
    if (!exists)
        co_return nullptr;

    request req;
    req.push("GET", "test:"+std::to_string(testId));

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    std::string testStr = std::get<0>(resp).value();

    // will return nullptr if any parse errors
    co_return hjw::utils::stringToReport(testStr);
}

auto StrategyService::co_exists(int testId) -> net::awaitable<bool> {
    // check if the test id is present in the cache
    request req;
    req.push("TYPE", "test:"+std::to_string(testId));

    generic_response resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    if (resp.value().at(0).value == "string")
        co_return true;
    else
        co_return false;
}
