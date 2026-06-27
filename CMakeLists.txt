cmake_minimum_required(VERSION 3.16)

project(riscv_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(riscv-sim
    src/main.cpp
    src/cpu.cpp
    src/memory.cpp
    src/instruction.cpp
    src/simulator.cpp
)

target_include_directories(riscv-sim PRIVATE include)

if(MSVC)
    target_compile_options(riscv-sim PRIVATE /W4 /utf-8)
else()
    target_compile_options(
        riscv-sim
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

enable_testing()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME simulator_programs
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_tests.py
    )
endif()
