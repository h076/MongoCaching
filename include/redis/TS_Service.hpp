#ifndef SERVICE_H_
#define SERVICE_H_

#include <boost/redis.hpp>

namespace hjw {

    namespace redis {

        // Used for any red timeseries requests and repsonses
        class TS_Service {

            public:

                TS_Service(std::shared_ptr<boost::redis::connection> conn) : m_conn(std::move(conn)) {}

                // Add a whole chunk of spots
                void addSeries();

                // Add single spot
                void add();

                // Get a whole chunk of spots
                void getSeries();

                // Get single spot
                void get();

            private:

                std::shared_ptr<boost::redis::connection> m_conn;
        };
    }
}

#endif // SERVICE_H_
