cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)

project(Sandbox)

function(MakeDay DAY)
add_executable(Day${DAY} ${CMAKE_CURRENT_SOURCE_DIR}/Day${DAY}/Main.cc ${CMAKE_CURRENT_SOURCE_DIR}/utils.h)
target_compile_definitions(Day${DAY} PUBLIC
-D FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/Day${DAY}"
)
target_include_directories(Day${DAY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

MakeDay(1)
MakeDay(2)
MakeDay(3)
MakeDay(4)
MakeDay(5)
MakeDay(6)
MakeDay(7)
MakeDay(8)
MakeDay(9)
MakeDay(10)
MakeDay(11)
MakeDay(12)
MakeDay(13)
MakeDay(14)
MakeDay(15)
MakeDay(16)
MakeDay(17)
MakeDay(18)
MakeDay(19)
MakeDay(20)
