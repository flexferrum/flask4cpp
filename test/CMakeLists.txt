if (Clang OR GNU)
    if (NOT UNIX AND GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    endif ()
endif ()

add_executable (flask4cpp_basic_test basic/main.cpp)

target_link_libraries(flask4cpp_basic_test ${LIB_TARGET_NAME})

enable_testing()

add_test(NAME flask4cpp_basic_test COMMAND flask4cpp_basic_test)