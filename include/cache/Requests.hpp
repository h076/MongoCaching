#ifndef REQUESTS_H_
#define REQUESTS_H_

#include "utils/spots.hpp"
#include <iostream>
#include <future>

namespace hjw {

    namespace cache {

        enum RequestType {GET, SET};

        struct TimeSeriesRequest {
            RequestType type;
            std::string symbol;
            uint64_t from;
            uint64_t to;
            std::promise<utils::series *>* getSeries;
        };
    }
}

#endif // REQUESTS_H_
