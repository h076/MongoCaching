#include <redis/TimeSeriesService.hpp>

using namespace hjw::utils;
using namespace hjw::redis;
using namespace boost::redis;

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

    co_await m_conn->async_exec(req, resp, net::deferred);
}

series * TimeSeriesService::getSeries(const std::string& symbol) {

}

void TimeSeriesService::get(const std::string& tsName, const std::string& from, const std::string& to) {

}
