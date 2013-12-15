cmake_minimum_required(VERSION 2.8)

# Find ClanLib version 3.0
set(ClanLib_MAJOR_VERSION 3)
set(ClanLib_MINOR_VERSION 0)
set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/bin")

project(LostWave CXX C)

add_subdirectory(src/kiss_fft130)
add_subdirectory(src/libawe)
add_subdirectory(src)
