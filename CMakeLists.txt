#
# Program: STEPToPoints
#
# Description:
#
# The program STEPToPoints converts solids contained in STEP files into point clouds by regular sampling.
#
# Copyright(C) 2022 Alexander Leutgeb
#
# This library is free software; you can redistribute it and / or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA
#

cmake_minimum_required (VERSION 3.18)

project (STEPToPoints LANGUAGES CXX VERSION 2025.0.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# compile options for all targets depending on the compiler and the system
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(MY_COMPILE_OPTIONS "-Wall;-Wextra;-pedantic;-Wno-comment;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;-Wconversion;-Wmisleading-indentation;-Wsign-conversion;-Wfloat-equal")
    if(ENABLE_WARNINGS_AS_ERRORS)
        list(APPEND MY_COMPILE_OPTIONS "-Werror")
    endif()
elseif(MSVC)
    set(MY_COMPILE_OPTIONS "/W4;/permissive-;/Zc:__cplusplus")
    if(ENABLE_WARNINGS_AS_ERRORS)
        list(APPEND MY_COMPILE_OPTIONS "/WX")
    endif()
endif()
if(APPLE)
    list(APPEND MY_COMPILE_OPTIONS "-Wno-deprecated-declarations")
endif()

# compile definitions for all targets depending on the compiler and the system
set(MY_COMPILE_DEFINITIONS "")
if(MSVC)
    list(APPEND MY_COMPILE_DEFINITIONS "-DNOMINMAX;-D_USE_MATH_DEFINES")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MY_CXX_FEATURE cxx_std_${CMAKE_CXX_STANDARD})

find_package(OpenCASCADE QUIET)
if(NOT OpenCASCADE_FOUND)
  message(STATUS "OpenCASCADE not found - installing via CMake fetchcontent")
  include(FetchContent)
  FetchContent_Declare(
    OpenCASCADE
    GIT_REPOSITORY https://github.com/Open-Cascade-SAS/OCCT.git
    )

  FetchContent_MakeAvailable(OpenCASCADE)
else()
    message(STATUS "OpenCASCADE found: ${OpenCASCADE_VERSION}")
endif()

find_package(indicators QUIET)
if(NOT indicators_FOUND)
  message(STATUS "indicators not found - installing via CMake...")

  include(FetchContent)
  FetchContent_Declare(
    indicators
    GIT_REPOSITORY https://github.com/p-ranav/indicators.git
    )
    set(INDICATORS_INSTALL ON CACHE INTERNAL "")
    set(INDICATORS_DEMO OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(indicators)

else()
    message(STATUS "indicators found: ${indicators_VERSION}")
endif()

find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    list(APPEND MY_COMPILE_DEFINITIONS "-DS2P_USE_OPENMP")
else()
    message(STATUS "OpenMP not found")
endif()

add_executable(STEPToPoints src/STEPToPoints.cpp src/solid_index_parser.cpp)

# @TODO: check which OpenCASCADE libraries are really needed
target_link_libraries(STEPToPoints ${OpenCASCADE_FoundationClasses_LIBRARIES} ${OpenCASCADE_ModelingData_LIBRARIES} ${OpenCASCADE_ModelingAlgorithms_LIBRARIES} ${OpenCASCADE_Visualization_LIBRARIES} ${OpenCASCADE_ApplicationFramework_LIBRARIES} ${OpenCASCADE_DataExchange_LIBRARIES} ${OpenCASCADE_Draw_LIBRARIES} indicators::indicators) #

if(OpenMP_CXX_FOUND)
    target_link_libraries(STEPToPoints OpenMP::OpenMP_CXX)
endif()

target_compile_options(STEPToPoints PRIVATE ${MY_COMPILE_OPTIONS})
target_compile_definitions(STEPToPoints PUBLIC ${MY_COMPILE_DEFINITIONS})
target_compile_features(STEPToPoints PRIVATE ${MY_CXX_FEATURE})