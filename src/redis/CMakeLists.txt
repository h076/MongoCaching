# External dependencies
find_package(Boost_asio CONFIG REQUIRED)
find_package(Boost_redis CONFIG REQUIRED)

add_library(hjw_redis STATIC)

file(GLOB REDIS_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB REDIS_HDR CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/redis/*.hpp
)

# Set sources for the redis library
target_sources(hjw_redis
    PRIVATE
        ${REDIS_SRC}
        ${REDIS_HDR}
)

# Include path for this library
target_link_libraries(hjw_redis
    PUBLIC
        Boost::asio
        Boost::redis
        hjw_mongo
)

target_include_directories(hjw_redis
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
