# External dependencies
find_package(Boost_asio CONFIG REQUIRED)
find_package(Boost_redis CONFIG REQUIRED)

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
target_link_libraries(hjw_mongo
    PUBLIC
        Boost::asio
        Boost::redis
)

target_include_directories(hjw_cache
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
