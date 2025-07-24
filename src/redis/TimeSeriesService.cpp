#include <redis/TimeSeriesService.hpp>

using namespace hjw::utils;
using namespace hjw::redis;
using namespace boost::redis;

auto TimeSeriesService::co_create(const std::string& symbol) -> net::awaitable<void> {
    request req;

    req.push("TS.CREATE", symbol+":low", "DUPLICATE_POLICY", "FIRST", "LABELS", "value_type", "low");
    req.push("TS.CREATE", symbol+":high", "DUPLICATE_POLICY", "FIRST", "LABELS", "value_type", "high");
    req.push("TS.CREATE", symbol+":close", "DUPLICATE_POLICY", "FIRST", "LABELS", "value_type", "close");
    req.push("TS.CREATE", symbol+":open", "DUPLICATE_POLICY", "FIRST", "LABELS", "value_type", "open");

    response<ignore_t> resp;

    co_await m_conn->async_exec(req, resp, net::use_awaitable);
}

auto TimeSeriesService::co_exists(const std::string& symbol) -> net::awaitable<bool> {
    request req;

    // Use close series as a check
    req.push("TYPE", symbol+":close");

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::use_awaitable);

    if (std::get<0>(resp) == "timeseries")
        co_return true;
    else
        co_return false;
}

void TimeSeriesService::addSeries(series * s) {

}

// must add checking correctly for things such as num of values vs timestamps
auto TimeSeriesService::co_add(const std::string& tsName, const std::vector<std::string>& timeStamps,
                               const std::vector<std::string>& values) -> net::awaitable<void> {
    request req;

    std::size_t n = timeStamps.size();
    for(std::size_t i=0; i<n; i++)
        req.push("TS.ADD", tsName, timeStamps[i], values[i]);

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::use_awaitable);
}

series * TimeSeriesService::getSeries(const std::string& symbol) {

}

auto TimeSeriesService::co_get(const std::string& tsName, const std::string& from,
                               const std::string& to) -> net::awaitable<series *> {
    request req;

    req.push("TS.GET", tsName, from, to);

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::use_awaitable);

    // handle series
    co_return nullptr;
}
