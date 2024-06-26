cmake_minimum_required(VERSION 2.5)
project(mymuduo)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

#aux_source_directory(. SRC_LIST)

#add_library(mymuduo SHARED ${SRC_LIST})
link_directories(${PROJECT_SOURCE_DIR}/lib)


add_subdirectory(src)

add_subdirectory(example)