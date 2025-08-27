#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <iostream>
#include <fstream>

#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/json.hpp>

namespace hjw {

    namespace mongo {

        class Connector {

            public:

                Connector(const std::string& cluster) : inst{} {
                    setConnectionString(cluster);
                    connect();
                };

                mongocxx::database getDB(const std::string& db) const;

            private:

                void setConnectionString(const std::string& cluster);
                void connect();

                std::string connection_string;
                mongocxx::instance inst;
                mongocxx::uri uri;
                std::unique_ptr<mongocxx::client> client;
        };
    }
}

#endif // CONNECTOR_H_
