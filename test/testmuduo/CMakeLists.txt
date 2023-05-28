cmake_minimum_required(VERSION 3.0)
project(main)

# Compiler flags
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# Include directories
# include_directories()

# Link directories
# link_directories()

# Set source files list
set(SRC_LIST muduoserver.cpp)

# Set executable output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Generate executable server, compiled with source files defined in SRC_LIST
add_executable(server ${SRC_LIST})

# Link libraries to executable server, need muduo_net, muduo_base, pthread
target_link_libraries(server muduo_net muduo_base pthread)
