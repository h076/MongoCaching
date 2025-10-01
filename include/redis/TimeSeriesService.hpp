#ifndef TIMESERIESSERVICE_H_
#define TIMESERIESSERVICE_H_

#include <boost/redis/connection.hpp>

#include <boost/asio.hpp>
#include <utils/spots.hpp>

#include <ranges>
#include <algorithm>

namespace hjw {

    namespace redis {

        namespace net = boost::asio;

        // Used for any red timeseries requests and repsonses
        class TimeSeriesService {

            public:

                TimeSeriesService(std::shared_ptr<boost::redis::connection> conn) : m_conn(std::move(conn)) {}

                ~TimeSeriesService() {}

                net::awaitable<void> co_create(const std::string& symbol);

                net::awaitable<bool> co_exists(const std::string& symbol);

                // Add a whole chunk of spots
                net::awaitable<void> co_addSeries(utils::series * s);

                // Add a range of spot values
                net::awaitable<void> co_add(const std::string& tsName, const std::vector<double>& timeStamps,
                                            const std::vector<double>& values);

                // Get a whole chunk of spots
                net::awaitable<utils::series *> co_getSeries(const std::string& symbol, const uint64_t from,
                                                             const uint64_t to);

                // Get a range of spot values
                net::awaitable<utils::subseries *> co_get(const std::string& tsName, const uint64_t from,
                                                          const uint64_t to);

                // Get oldest timestamp
                net::awaitable<uint64_t> co_first_ts(const std::string& symbol);

                // Get most recent timestamp
                net::awaitable<uint64_t> co_latest_ts(const std::string& symbol);

                // move used her to explicilty state that this service no longer has ownership
                // as connection is stored as a member variable
                std::shared_ptr<boost::redis::connection> release() {return std::move(m_conn);}

            private:
                inline void fill_val(std::vector<double> * bucket,
                                     utils::subseries& tap) {
                    bucket->clear();
                    bucket->reserve(tap.size());

                    auto values = tap | std::views::transform([](const auto& tup) {
                        return std::get<1>(tup);
                    });

                    bucket->assign(values.begin(), values.end());
                }

                inline void fill_key(std::vector<double> * bucket,
                                     utils::subseries& tap) {
                    bucket->clear();
                    bucket->reserve(tap.size());

                    auto keys = tap | std::views::transform([](const auto& tup) {
                        return std::get<0>(tup);
                    });

                    bucket->assign(keys.begin(), keys.end());
                }

            private:

                std::shared_ptr<boost::redis::connection> m_conn;
        };
    }
}

#endif // TIMESERIESSERVICE_H_
