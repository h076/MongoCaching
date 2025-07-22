#ifndef SERVICE_H_
#define SERVICE_H_

#include <boost/redis.hpp>
#include <boost/asio.hpp>
#include <utils/spots.hpp>

namespace hjw {

    namespace redis {

        namespace net = boost::asio;

        // Used for any red timeseries requests and repsonses
        class TimeSeriesService {

            public:

                TimeSeriesService(std::shared_ptr<boost::redis::connection> conn) : m_conn(std::move(conn)) {}

                // Add a whole chunk of spots
                void addSeries(utils::series * s);

                // Add a range of spot values
                net::awaitable<void> co_add(const std::string& tsName, const std::vector<std::string>& timeStamps,
                                            const std::vector<std::string>& values);

                // Get a whole chunk of spots
                utils::series * getSeries(const std::string& symbol);

                // Get a range of spot values
                void get(const std::string& tsName, const std::string& from, const std::string& to);

            private:

                std::shared_ptr<boost::redis::connection> m_conn;
        };
    }
}

#endif // SERVICE_H_
