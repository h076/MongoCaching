cmake_minimum_required(VERSION 3.14)
project(MongoCaching LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(copy_compile_commands ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_BINARY_DIR}/compile_commands.json
          ${PROJECT_SOURCE_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to root directory"
)

# Add subdirectories
add_subdirectory(src/mongo)
add_subdirectory(src/redis)

add_executable(MRtest src/mongoAndRedis.cpp)

# Link local directories and external libraries
target_link_libraries(MRtest PRIVATE hjw_redis hjw_mongo)
