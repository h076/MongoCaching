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

        inline void appendToSeries(series& s, rapidjson::Document& doc) {
            s.timestamps.push_back(doc["timestamp"]["$date"].GetString());
            s.low.push_back(doc["low"].GetString());
            s.high.push_back(doc["high"].GetString());
            s.close.push_back(doc["close"].GetString());
            s.open.push_back(doc["open"].GetString());
        }

    }
}

#endif // SPOTS_H_
