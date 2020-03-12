cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (flask4cpp LANGUAGES CXX VERSION 0.1.0)


if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(JINJA2CPP_IS_MAIN_PROJECT TRUE)
else()
    set(JINJA2CPP_IS_MAIN_PROJECT FALSE)
endif()

option(FLASK4CPP_EXT_CONAN "External conan recipe is used for build Flask4C++" OFF)
option(FLASK4CPP_BUILD_TESTS "Build tests for Flask4C++" ${FLASK4CPP_IS_MAIN_PROJECT})
set(FLASK4CPP_CONAN_PROFILE "default" CACHE STRING "Path to conan profile to use to build Flask4C++. '<default>' to use default (autodetected) profile")

include (cmake/conan_setup.cmake)
set (LIBRARY_NAME ${PROJECT_NAME})

if (Clang OR GNU)
    if (NOT UNIX AND GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    endif ()
endif ()

add_executable (flask4cpp_basic_test test/basic/main.cpp)

target_link_libraries(flask4cpp_basic_test ${CONAN_LIBS})

enable_testing()

add_test(NAME flask4cpp_basic_test COMMAND flask4cpp_basic_test)