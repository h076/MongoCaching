#include "mongo/SpotService.hpp"
#include "utils/time.hpp"

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

    if (cursor.begin() == cursor.end()) {
        std::cout << "Invalid time range for symbol : " << symbol << " - No spots returned." << std::endl;
        return nullptr;
    }

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

seriesInfo * SpotService::info(const std::string& symbol) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    // construct document
    // match stage
    mongocxx::pipeline pipeline;
    pipeline.match(document{}
                   << "symbol" << symbol
                   << finalize);

    // group stage
    pipeline.group(document{}
                   << "_id" << bsoncxx::types::b_null{}
                   << "maxTimestamp" << open_document << "$max" << "$timestamp" << close_document
                   << "minTimestamp" << open_document << "$min" << "$timestamp" << close_document
                   << finalize);

    // Run aggregation
    auto cursor = collection.aggregate(pipeline);

    // Does the symbol exists ?
    auto it = cursor.begin();
    if (it == cursor.end()) {
        return nullptr;
    }

    // Only one document should be returned
    auto&& doc = *(it);
    rapidjson::Document jd;
    jd.Parse(bsoncxx::to_json(doc).c_str());

    uint64_t minTs = jd["minTimestamp"]["$date"].GetUint64();
    uint64_t maxTs = jd["maxTimestamp"]["$date"].GetUint64();

    seriesInfo * si = new seriesInfo(symbol, minTs, maxTs);

    return si;
}

void SpotService::post() {
    std::cout << "post data ..." << std::endl;
    return;
}
