cmake_minimum_required(VERSION 3.10)
project(EcoLang CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ecolang
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/ir.cpp
    src/optimizer.cpp
    src/energy.cpp
    src/codegen.cpp
    src/report.cpp
)

target_include_directories(ecolang PRIVATE include)
target_compile_options(ecolang PRIVATE -Wall -Wextra)
