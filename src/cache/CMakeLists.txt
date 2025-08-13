# External dependencies
find_package(Boost_asio CONFIG REQUIRED)
find_package(Boost_redis CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_library(hjw_cache STATIC)

file(GLOB CACHE_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB CACHE_HDR CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/cache/*.hpp
)

# Set sources for the mongo library
target_sources(hjw_cache
    PRIVATE
        ${CACHE_SRC}
        ${CACHE_HDR}
)

# Include path for this library
target_link_libraries(hjw_cache
    PUBLIC
        Boost::asio
        Boost::redis
        mongo::bsoncxx_static
        mongo::mongocxx_static
        RapidJSON
)

target_include_directories(hjw_cache
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_library(cache ALIAS hjw_cache)
