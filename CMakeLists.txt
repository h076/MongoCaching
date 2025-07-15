cmake_minimum_required(VERSION 3.14)
project(MongoCaching LANGUAGE CXX)

set_property(TARGET MongoCaching CMAKE_CXX_STANDARD 20)
set_property(TARGET MongoCaching CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy compile_commands.json to root for lsp
add_custom_target(copy_compile_commands ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_BINARY_DIR}/compile_commands.json
          ${PROJECT_SOURCE_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to root directory"
)

# Find required packages
find_package(libmongocxx REQUIRED)
find_package(libbsoncxx REQUIRED)

# Create header-only library
add_library(MongoCaching INTERFACE)

# Include all headers
target_include_directories(MongoCaching
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# define name space
target_compile_definitions(MongoCaching INTERFACE MY_LIB_NAMESPACE=hjw)

# link external dependencies
target_link_libraries(MongoCaching
    PRIVATE
    libmongocxx::mongocxx
    libbsoncxx::bsoncxx
)

# add temporary executable
add_executable(temp src/main.cpp)
target_link_libraries(temp PRIVATE lib)
