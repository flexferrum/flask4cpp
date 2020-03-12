if (Clang OR GNU)
    if (NOT UNIX AND GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    endif ()
endif ()

enable_testing()
find_package(GTest)

include(collect_sources)

set(SUBDIRS unit)

macro(AddTestTarget SubDir)
    set(SrcVar ${SubDir}_Sources)
    set(HdrVar ${SubDir}_Headers)
    set(TargetName flask4cpp_${SubDir}_test)
    CollectSources(${SrcVar} ${HdrVar} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${SubDir})

    add_executable(${TargetName} ${${SrcVar}} ${${HdrVar}} test_main.cpp)

    target_link_libraries(${TargetName} ${LIB_TARGET_NAME} GTest::GTest)

    add_test(NAME ${TargetName} COMMAND ${TargetName})
endmacro()

add_executable(flask4cpp_basic basic/main.cpp)

AddTestTarget(unit)
