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

    co_await m_conn->async_exec(req, resp, net::deferred);

    co_return;
}

auto TimeSeriesService::co_exists(const std::string& symbol) -> net::awaitable<bool> {
    request req;

    // Use close series as a check
    req.push("TYPE", symbol+":close");

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    if (std::get<0>(resp) == "timeseries")
        co_return true;
    else
        co_return false;
}

auto TimeSeriesService::co_addSeries(series * s) -> net::awaitable<void> {
    const std::string symbol = s->symbol;

    // If series does not exist then create the required series keys
    bool exists = co_await co_exists(symbol);
    if (!exists)
        co_await co_create(symbol);

    co_await co_add(symbol+":low", s->timestamps, s->low);
    co_await co_add(symbol+":high", s->timestamps, s->high);
    co_await co_add(symbol+":open", s->timestamps, s->open);
    co_await co_add(symbol+":close", s->timestamps, s->close);

    co_return;
}

// must add checking correctly for things such as num of values vs timestamps
auto TimeSeriesService::co_add(const std::string& tsName, const std::vector<std::string>& timeStamps,
                               const std::vector<std::string>& values) -> net::awaitable<void> {
    request req;

    std::size_t n = timeStamps.size();
    for(std::size_t i=0; i<n; i++)
        req.push("TS.ADD", tsName, timeStamps[i], values[i]);

    response<std::string> resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    co_return;
}

auto TimeSeriesService::co_getSeries(const std::string& symbol, const std::string& from,
                                     const std::string& to) -> net::awaitable<series *> {
    bool exists = co_await co_exists(symbol);
    if (!exists) {
        std::cout << "Cache miss" << std::endl;
        co_return nullptr;
    }

    std::cout << "Cache hit." << std::endl;

    series * s = new series(symbol);

    std::vector<std::tuple<std::string, std::string>> key_value_pairs;
    key_value_pairs = co_await co_get(symbol+":low", from, to);
    fill_val(&s->low, key_value_pairs);

    key_value_pairs = co_await co_get(symbol+":high", from, to);
    fill_val(&s->high, key_value_pairs);

    key_value_pairs = co_await co_get(symbol+":open", from, to);
    fill_val(&s->open, key_value_pairs);

    key_value_pairs = co_await co_get(symbol+":close", from, to);
    fill_val(&s->close, key_value_pairs);

    fill_key(&s->timestamps, key_value_pairs);

    co_return s;
}

auto TimeSeriesService::co_get(const std::string& tsName, const std::string& from,
                               const std::string& to)-> net::awaitable<std::vector<std::tuple<std::string,
                                                                                              std::string>>> {
    request req;

    req.push("TS.RANGE", tsName, from, to);

    response<std::vector<std::tuple<std::string, std::string>>> resp;

    co_await m_conn->async_exec(req, resp, net::deferred);

    // return series
    co_return std::get<0>(resp);
}
