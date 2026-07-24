cmake_minimum_required(VERSION 3.16)
project(cv_pipeline)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(mat_internals src/01_mat_internals.cpp)
target_link_libraries(mat_internals ${OpenCV_LIBS})
target_include_directories(mat_internals PRIVATE ${OpenCV_INCLUDE_DIRS})