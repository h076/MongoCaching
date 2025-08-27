# External dependencies
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_library(hjw_mongo STATIC)

file(GLOB MONGO_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB MONGO_HDR CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/mongo/*.hpp
)

# Set sources for the mongo library
target_sources(hjw_mongo
    PRIVATE
        ${MONGO_SRC}
        ${MONGO_HDR}
)

# Include path for this library
target_link_libraries(hjw_mongo
    PUBLIC
        mongo::bsoncxx_static
        mongo::mongocxx_static
        RapidJSON
)

target_include_directories(hjw_mongo
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
