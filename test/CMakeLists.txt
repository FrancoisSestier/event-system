cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(event-system_test
  LANGUAGES CXX
)

option(ENABLE_TEST_COVERAGE "Enable test coverage" ON)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

include(../cmake/CPM.cmake)
find_package(Threads REQUIRED)

CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

if (TEST_INSTALLED_VERSION)
  find_package(event-system REQUIRED)
else()
  CPMAddPackage(
    NAME event-system
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

enable_testing()
add_executable(event-system_test ${sources})
target_link_libraries(event-system_test debug gtest_main event-system)
set_target_properties(event-system_test PROPERTIES CXX_STANDARD 20)

include(GoogleTest)
gtest_discover_tests(event-system_test)



if (ENABLE_TEST_COVERAGE)
  target_compile_options(event-system INTERFACE /DEBUG -gdwarf-3 -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(event-system INTERFACE -fprofile-arcs -ftest-coverage)
endif()
