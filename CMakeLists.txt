# Yet another file lifted from a glorious demo repo, hail https://github.com/gujans/travis-gtest-cmake-example!

cmake_minimum_required(VERSION 3.5.1)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME PDP-1186)
project(${PROJECT_NAME})
include(CTest)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror")

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
#set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Define project sources and includes
#----------------------------------------------------------------------------
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)

set(CMAKE_CXX_STANDARD 14)

################################
# Normal Libraries & Executables
################################
add_library(PDP-1186_lib ${SRC_FILES})
set_target_properties(PDP-1186_lib PROPERTIES LINKER_LANGUAGE CXX)
add_executable(PDP-1186 ${PROJECT_SOURCE_DIR}/src/main.cpp)
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere (e.g linking to the test executable).

target_link_libraries(PDP-1186 PDP-1186_lib)

################################
# Testing
################################


# This adds another subdirectory, which has 'project(gtest)'.
add_subdirectory(ext/gtest)

enable_testing()

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

##############
# Unit Tests
##############
add_executable(runUnitTests ${TEST_SRC_FILES})

# Standard linking to gtest stuff.
target_link_libraries(runUnitTests gtest gtest_main)

# Extra linking for the project.
target_link_libraries(runUnitTests PDP-1186_lib)

# This is so you can do 'make test' to see all your tests run, instead of
# manually running the executable runUnitTests to see those specific tests.
add_test(UnitTests runUnitTests)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.travis/cmake)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)
    setup_target_for_coverage(${PROJECT_NAME}_coverage runUnitTests coverage)

    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif() #CMAKE_BUILD_TYPE STREQUAL "Coverage"
