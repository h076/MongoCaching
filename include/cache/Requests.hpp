#ifndef REQUESTS_H_
#define REQUESTS_H_

#include <future>

#include "utils/hjw_utils.hpp"

namespace hjw {

    namespace cache {

        enum RequestType {GET, SET, INFO, STOP_CACHE};

        template<RequestType T>
        struct TimeSeriesRequest {
            static constexpr RequestType type = T;
        };

        template<>
        struct TimeSeriesRequest<RequestType::GET> {
            static constexpr RequestType type = RequestType::GET;
            std::string symbol;
            uint64_t from;
            uint64_t to;
            std::promise<utils::series*>* series;
        };

        template<>
        struct TimeSeriesRequest<RequestType::SET> {
            static constexpr RequestType type = RequestType::SET;
            std::string symbol;
            uint64_t from;
            uint64_t to;
            utils::series * series;
        };

        template<>
        struct TimeSeriesRequest<RequestType::INFO> {
            static constexpr RequestType type = RequestType::INFO;
            std::string symbol;
            std::promise<bool> * exists; // allows to block on the promise
        };

        template<>
        struct TimeSeriesRequest<RequestType::STOP_CACHE> {
            static constexpr RequestType type = RequestType::STOP_CACHE;
        };
    }
}

#endif // REQUESTS_H_
