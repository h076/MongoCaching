#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <iostream>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>

namespace hjw {

    namespace mongo {

        class Connector {

            public:

                Connector(const std::string& cluster) {
                    setConnectionString(cluster);
                };

                void setConnectionString(const std::string& cluster);

                void connect();

            private:

                std::string connection_string;
                std::unique_ptr<mongocxx::uri> uri;
                std::unique_ptr<mongocxx::client> client;
            
        };
    }
}

#endif // CONNECTOR_H_
