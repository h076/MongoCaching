cmake_minimum_required(VERSION 3.14)
project(MongoCaching LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(src/mongo)
add_subdirectory(src/redis)
add_subdirectory(src/cache)

# Only add test dir if present
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
  add_subdirectory(src/tests)
endif()
