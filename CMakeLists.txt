cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project(flask4cpp LANGUAGES CXX VERSION 0.1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_CURRENT_BINARY_DIR})

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(FLASK4CPP_IS_MAIN_PROJECT TRUE)
else ()
    set(FLASK4CPP_IS_MAIN_PROJECT FALSE)
endif ()

set(FLASK4CPP_CONAN_PROFILE "auto" CACHE STRING "Path to conan profile to use to build Flask4C++. 'auto' to use autodetected profile settings")
set(FLASK4CPP_CODEGENPATH "" CACHE STRING "Path to fl-codegen codegeneration utility")

if (FLASK4CPP_CODEGENPATH)
    set(FLASK4CPP_ENABLED_CODEGEN TRUE)
endif ()

option(FLASK4CPP_EXT_CONAN "External conan recipe is used for build Flask4C++" OFF)
option(FLASK4CPP_BUILD_TESTS "Build tests for Flask4C++" ${FLASK4CPP_IS_MAIN_PROJECT})
option(FLASK4CPP_USE_CODEGEN_METACLASSES "Enable support for fl-codegen metaclasses" ${FLASK4CPP_ENABLED_CODEGEN})
option(FLASK4CPP_USE_STD_METACLASSES "Enable support for standard C++2y metaclasses" OFF)
option(FLASK4CPP_ENABLE_COROUTINES "Enable support for C++20 coroutines" OFF)
option(FLASK4CPP_ENABLE_SSL "Enable support for SSL connections" OFF)

include(cmake/conan_setup.cmake)
include(collect_sources)

CollectSources(Sources Headers ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
CollectSources(PublicSources PublicHeaders ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(jinja2cpp)
find_package(Boost)
find_package(nlohmann_json)

set (LIB_TARGET_NAME ${PROJECT_NAME})

add_library(${LIB_TARGET_NAME} STATIC # ${LIB_LINK_TYPE}
        ${Sources}
        ${Headers}
        )

target_include_directories(${LIB_TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${Boost_INCLUDE_DIRS}
        )

set_target_properties(${LIB_TARGET_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        )

set_property(TARGET ${LIB_TARGET_NAME} PROPERTY PUBLIC_HEADER ${PublicHeaders})

target_link_libraries(
        ${LIB_TARGET_NAME}
        PUBLIC jinja2cpp::jinja2cpp nlohmann_json::nlohmann_json
)

if (FLASK4CPP_ENABLE_SSL)
    find_package(OpenSSL)

    target_link_libraries(
            ${LIB_TARGET_NAME}
            PRIVATE OpenSSL::OpenSSL
    )

    target_compile_definitions(
            ${LIB_TARGET_NAME}
            PRIVATE -DFLASK4CPP_SSL_ENABLED
    )
endif ()

if (FLASK4CPP_BUILD_TESTS)
    add_subdirectory(test)
endif ()