#ifndef TIME_H_
#define TIME_H_

#include <ctime>
#include <chrono>
#include <iostream>

namespace hjw {

    namespace utils {

        inline std::chrono::system_clock::time_point parseISO8601(const std::string& s) {
            std::tm tm = {};
            strptime(s.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm); // parse into tm struct
            std::time_t time = timegm(&tm); // convert to UTC
            return std::chrono::system_clock::from_time_t(time);
        }
    }
}

#endif // TIME_H_
