cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)

project(Sandbox)

function(MakeDay DAY)
add_executable(${DAY} ${CMAKE_CURRENT_SOURCE_DIR}/${DAY}/Main.cc ${CMAKE_CURRENT_SOURCE_DIR}/utils.h)
target_compile_definitions(${DAY} PUBLIC
-D FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/${DAY}"
)
target_include_directories(${DAY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

file(GLOB DAYS ${CMAKE_CURRENT_SOURCE_DIR}/Day*)
foreach(DAY IN LISTS DAYS)
    get_filename_component(DAY_FOLDER ${DAY} NAME_WE)
    MakeDay(${DAY_FOLDER})
endforeach()
