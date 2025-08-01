#ifndef REQUESTS_H_
#define REQUESTS_H_

#include <iostream>

namespace hjw {

    namespace cache {

        enum RequestType {GET, SET};

        struct TimeSeriesRequest {
            RequestType type;
            std::string symbol;
            uint64_t from;
            uint64_t to;
        };
    }
}

#endif // REQUESTS_H_
