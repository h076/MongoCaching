#ifndef SERVICE_H_
#define SERVICE_H_

#include "Connector.hpp"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>

#include <rapidjson/document.h>

#include <utils/time.hpp>
#include <utils/spots.hpp>

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

                virtual utils::series * get(const std::string& symbol, const uint64_t from, const uint64_t to) = 0;

                virtual utils::series * get(const std::string& symbol, const std::chrono::system_clock::time_point from,
                                            const std::chrono::system_clock::time_point to) = 0;

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
