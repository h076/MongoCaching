#ifndef BULK_FLOAT_PATCH_H_
#define BULK_FLOAT_PATCH_H_

#include <boost/redis/resp3/serialization.hpp>
#include <charconv>
#include <limits>
#include <type_traits>
#include <stdexcept>

namespace boost::redis::resp3 {

    template <class T>
    std::enable_if_t<std::is_floating_point_v<T>, void>
    boost_redis_to_bulk(std::string& payload, T n) {
        char buf[64];

        // Convert floating-point number to text (fast and safe).
        auto [ptr, ec] = std::to_chars(
            buf, buf + sizeof(buf),
            n,
            std::chars_format::general,
            std::numeric_limits<T>::max_digits10
        );

        if (ec != std::errc()) {
            throw std::runtime_error("boost_redis_to_bulk: floating-point conversion failed");
        }

        // Delegate back to the existing string_view overload in Boost
        boost_redis_to_bulk(payload, std::string_view(buf, ptr - buf));
    }
};

#endif // BULK_FLOAT_PATCH_H_
