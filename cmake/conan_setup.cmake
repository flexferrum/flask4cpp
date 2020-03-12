if (NOT FLASK4CPP_EXT_CONAN)
    include (cmake/conan.cmake)
    if (NOT MSVC)
        set (FLASK4CPP_CONAN_SETTINGS compiler.libcxx=libstdc++11)
    endif ()

    if (FLASK4CPP_CONAN_PROFILE STREQUAL "default")
        set (FLASK4CPP_CONAN_SETTINGS compiler.cppstd=14)
    else ()
        set (FLASK4CPP_CONAN_PROFILE_PARAM "PROFILE ${FLASK4CPP_CONAN_PROFILE}")
    endif ()

    conan_cmake_run(
            REQUIRES
            jinja2cpp/[>=1.1.0]
            gtest/[>=1.10.0]
            boost/[>=1.72.0]
            nlohmann_json/[>=3.7.0]
            BASIC_SETUP
            SETTINGS ${FLASK4CPP_CONAN_SETTINGS} build_type=${CMAKE_BUILD_TYPE}
            ${FLASK4CPP_CONAN_PROFILE_PARAM}
            BUILD missing
            GENERATORS cmake cmake_find_package
    )
else ()
    include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif ()
