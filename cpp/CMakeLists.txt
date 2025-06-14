cmake_minimum_required(VERSION 3.10)
project(ComputingFeynman)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GVC REQUIRED libgvc)

find_package(Boost REQUIRED COMPONENTS program_options CONFIG)

include_directories(${GVC_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${GVC_LIBRARY_DIRS})

add_executable(computingFeynman computingFeynman.cpp)

target_link_libraries(computingFeynman ${GVC_LIBRARIES} Boost::program_options)
