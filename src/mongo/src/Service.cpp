#include <mongo/Service.hpp>

using namespace hjw::mongo;

void Service::set(const std::string& db, const std::string& coll) {
    database = connection->getDB(db);
    collection = database[coll];
}
