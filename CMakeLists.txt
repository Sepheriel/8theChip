cmake_minimum_required (VERSION 3.7.2)
project (8theChip)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")
set (CMAKE_CXX_STANDARD 11)

set(SOURCES
  src/main.cpp
  src/chip8.cpp
  src/chip8.hpp
)

find_package(SFML 2.4.1 COMPONENTS system window graphics audio REQUIRED)

if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
endif(SFML_FOUND)

add_executable(8theChip ${SOURCES})
target_link_libraries(8theChip ${SFML_LIBRARIES})
