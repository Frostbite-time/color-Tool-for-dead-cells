project(colorTool_v2)
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK")
set(CMAKE_BUILD_TYPE Release)
set(OpenCV_DIR "F:\\git\\opencv\\build\\install\\x64\\mingw\\lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${colorTool_v2_SOURCE_DIR}/build/bin)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(colorTool_v2 src/colorTool_v2.cpp)
link_directories(${OpenCV_LIBRARY_DIRS})
target_link_libraries(colorTool_v2 ${OpenCV_LIBS})