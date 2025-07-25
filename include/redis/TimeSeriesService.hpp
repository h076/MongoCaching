#ifndef SERVICE_H_
#define SERVICE_H_

#include <boost/redis.hpp>
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

                net::awaitable<void> co_create(const std::string& symbol);

                net::awaitable<bool> co_exists(const std::string& symbol);

                // Add a whole chunk of spots
                net::awaitable<void> co_addSeries(utils::series * s);

                // Add a range of spot values
                net::awaitable<void> co_add(const std::string& tsName, const std::vector<std::string>& timeStamps,
                                            const std::vector<std::string>& values);

                // Get a whole chunk of spots
                net::awaitable<utils::series *> co_getSeries(const std::string& symbol, const std::string& from,
                                                             const std::string& to);

                // Get a range of spot values
                net::awaitable<std::vector<std::tuple<std::string, std::string>>>
                co_get(const std::string& tsName, const std::string& from, const std::string& to);

            private:
                inline void fill_val(std::vector<std::string> * bucket,
                                     std::vector<std::tuple<std::string, std::string>>& tap) {
                    bucket->clear();
                    bucket->reserve(tap.size());

                    auto values = tap | std::views::transform([](const auto& tup) {
                        return std::get<1>(tup);
                    });

                    bucket->assign(values.begin(), values.end());
                }

                inline void fill_key(std::vector<std::string> * bucket,
                                     std::vector<std::tuple<std::string, std::string>>& tap) {
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

#endif // SERVICE_H_
