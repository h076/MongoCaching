#include <mongo/SpotService.hpp>

#include <utils/time.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

using namespace hjw::mongo;
using namespace hjw::utils;

void SpotService::get(const std::string& symbol, const std::string& from, const std::string& to) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;

    // Construct document
    auto doc = document{}
        << "symbol" << symbol
        << "timestamp" << open_document
        << "$gte" << bsoncxx::types::b_date{parseISO8601(from)}
        << "$lt" << bsoncxx::types::b_date{parseISO8601(to)}
        << close_document
        << bsoncxx::builder::stream::finalize;

    // Execute the query
    auto cursor = collection.find(doc.view());

    // Iterate and print results
    for(auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }
}

void SpotService::post() {
    std::cout << "post data ..." << std::endl;
    return;
}
