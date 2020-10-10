cmake_minimum_required(VERSION 3.18)
project(CE_LIB CXX)

add_library(ce_lib INTERFACE)
target_include_directories(ce_lib INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_compile_features(ce_lib INTERFACE cxx_std_20)

add_library(ce::ce ALIAS ce_lib)

include (CTest)
add_subdirectory(tests)
