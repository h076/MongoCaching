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

        inline uint64_t ISO8601ToUint64_t(const std::string& s) {
            return static_cast<uint64_t>(
                duration_cast<std::chrono::milliseconds>(
                    parseISO8601(s).time_since_epoch()).count());
        }
    }
}

#endif // TIME_H_
