cmake_minimum_required(VERSION 3.10)
project(MusicLang VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.1 
)
FetchContent_MakeAvailable(fmt)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra")

add_executable(mlang src/main.cpp src/Lexer.cpp src/Parser.cpp src/Enviroment.cpp src/Builtins.cpp src/Ast.cpp src/Evaluator.cpp)

target_link_libraries(mlang PRIVATE fmt)
