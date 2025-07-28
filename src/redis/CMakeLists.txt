# External dependencies
find_package(Boost_asio CONFIG REQUIRED)
find_package(Boost_redis CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

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
    PRIVATE
        Boost::asio
        Boost::redis
        mongo::bsoncxx_static
        mongo::mongocxx_static
        RapidJSON
)

target_include_directories(hjw_redis
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
