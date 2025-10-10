#ifndef SPOTS_H_
#define SPOTS_H_

#include <iostream>
#include <vector>
#include <string>

#include <rapidjson/document.h>

namespace hjw {

    namespace utils {


        // A series structure stores all spot data in string format
        // Strings are used as values are passed to redis as strings
        // so there is no need to convert after mongo request and before
        // redis post
        struct series {
            std::string symbol;
            std::vector<uint64_t> timestamps;
            std::vector<double> low;
            std::vector<double> high;
            std::vector<double> close;
            std::vector<double> open;

            series(const std::string& s) : symbol(s) {}
        };

        // simple type to use net::awaitable
        typedef std::vector<std::tuple<uint64_t, double>> subseries;

        inline void appendToSeries(series& s, rapidjson::Document& doc) {
            s.timestamps.push_back(doc["timestamp"]["$date"].GetUint64());
            s.low.push_back(doc["low"].GetDouble());
            s.high.push_back(doc["high"].GetDouble());
            s.close.push_back(doc["close"].GetDouble());
            s.open.push_back(doc["open"].GetDouble());
        }

        // Information about a complete time series
        struct seriesInfo {
            std::string symbol;
            uint64_t minStamp;
            uint64_t maxStamp;

            seriesInfo(const std::string& s, uint64_t min, uint64_t max)
                : symbol(s), minStamp(min), maxStamp(max) {}
        };

    }
}

#endif // SPOTS_H_
