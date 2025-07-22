cmake_minimum_required(VERSION 3.14)
project(MongoCaching LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Copy compile_commands.json to root for lsp
add_custom_target(copy_compile_commands ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_BINARY_DIR}/compile_commands.json
          ${PROJECT_SOURCE_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to root directory"
)

find_package(Boost 1.85.0 COMPONENTS program_options REQUIRED)

# Required for Boost::Redis
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

# Find required packages
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# RapidJSON
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h
  PATHS /usr/include /usr/local/include
)

if (NOT RAPIDJSON_INCLUDE_DIR)
  message(FATAL_ERROR "RapidJSON headers not found!")
endif()

### Define library

# All mongo source files
file(GLOB MONGO_SRC CONFIGURE_DEPENDS
  src/mongo/*.cpp
)

add_library(MongoCaching STATIC ${MONGO_SRC})

# Include headers
target_include_directories(MongoCaching
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${MONGOCXX_INCLUDE_DIR}
    ${BSONCXX_INCLUDE_DIR}
    /usr/local/include/mongocxx/v_noabi
    /usr/local/include/bsoncxx/v_noabi
    /usr/local/include/libmongoc-1.0
    /usr/local/include/libbson-1.0
    ${Boost_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

# define name space
target_compile_definitions(MongoCaching INTERFACE MY_LIB_NAMESPACE=hjw)



# link external dependencies
target_link_libraries(MongoCaching
    INTERFACE
      mongocxx
      bsoncxx
)

# add temporary executable
add_executable(temp src/main.cpp)
target_link_libraries(temp PRIVATE MongoCaching)
