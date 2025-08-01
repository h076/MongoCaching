#include "utils/spots.hpp"
#include <mongo/SpotService.hpp>

#include <utils/time.hpp>
#include <utils/spots.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

#include <rapidjson/document.h>

using namespace hjw::mongo;
using namespace hjw::utils;

series * SpotService::get(const std::string& symbol, const std::string& from, const std::string& to) {
    return SpotService::get(symbol, parseISO8601(from), parseISO8601(to));
}

series * SpotService::get(const std::string& symbol, const uint64_t from, const uint64_t to) {
    return SpotService::get(symbol, std::chrono::system_clock::time_point{std::chrono::milliseconds{from}}
           , std::chrono::system_clock::time_point{std::chrono::milliseconds{to}});
}

series * SpotService::get(const std::string& symbol, const std::chrono::system_clock::time_point from,
             const std::chrono::system_clock::time_point to) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;

    // Construct document
    auto doc = document{}
        << "symbol" << symbol
        << "timestamp" << open_document
        << "$gte" << bsoncxx::types::b_date{from}
        << "$lt" << bsoncxx::types::b_date{to}
        << close_document
        << bsoncxx::builder::stream::finalize;

    // Execute the query
    auto cursor = collection.find(doc.view());

    rapidjson::Document jd;
    series* s = new series(symbol);
    // Iterate through results and build series
    for(auto&& doc : cursor) {
        // Set doc to parse json
        jd.SetObject();
        jd.Parse(bsoncxx::to_json(doc).c_str());

        // Load data into series
        appendToSeries(*s, jd);
    }

    return s;
}

void SpotService::post() {
    std::cout << "post data ..." << std::endl;
    return;
}
