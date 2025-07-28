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
            std::vector<std::string> timestamps;
            std::vector<std::string> low;
            std::vector<std::string> high;
            std::vector<std::string> close;
            std::vector<std::string> open;

            series(const std::string& s) : symbol(s) {}
        };

        // simple wrapper to use net::awaitable
        struct subseries {
            std::vector<std::tuple<std::string, std::string>> ss;
        };

        inline void appendToSeries(series& s, rapidjson::Document& doc) {
            s.timestamps.push_back(std::to_string(doc["timestamp"]["$date"].GetInt64()));
            s.low.push_back(std::to_string(doc["low"].GetDouble()));
            s.high.push_back(std::to_string(doc["high"].GetDouble()));
            s.close.push_back(std::to_string(doc["close"].GetDouble()));
            s.open.push_back(std::to_string(doc["open"].GetDouble()));
        }

    }
}

#endif // SPOTS_H_
