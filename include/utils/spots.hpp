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
            std::vector<double> timestamps;
            std::vector<double> low;
            std::vector<double> high;
            std::vector<double> close;
            std::vector<double> open;

            series(const std::string& s) : symbol(s) {}
        };

        // simple type to use net::awaitable
        typedef std::vector<std::tuple<double, double>> subseries;

        inline void appendToSeries(series& s, rapidjson::Document& doc) {
            s.timestamps.push_back(static_cast<double>(doc["timestamp"]["$date"].GetInt64()));
            s.low.push_back(doc["low"].GetDouble());
            s.high.push_back(doc["high"].GetDouble());
            s.close.push_back(doc["close"].GetDouble());
            s.open.push_back(doc["open"].GetDouble());
        }

    }
}

#endif // SPOTS_H_
