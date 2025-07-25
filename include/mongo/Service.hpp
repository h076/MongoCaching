#ifndef SERVICE_H_
#define SERVICE_H_

#include "Connector.hpp"
#include <utils/spots.hpp>

#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>

#include <bsoncxx/document/value.hpp>
#include <bsoncxx/json.hpp>

using namespace bsoncxx;

namespace hjw {

    namespace mongo {

        class Service {

            public:
                // Many services could own a connection
                // So it is best to make connection a shared pointer
                Service(std::shared_ptr<Connector> conn)
                    : connection(std::move(conn)) {};

                void set(const std::string& db, const std::string& coll);

                virtual utils::series * get(const std::string& symbol, const std::string& from, const std::string& to) = 0;

                virtual void post() = 0;

            private:

                std::shared_ptr<Connector> connection;
                mongocxx::database database;

            protected:

                mongocxx::collection collection;
        };
    }
}

#endif // SERVICE_H_
