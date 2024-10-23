# CMakeList.txt : CMake project for MusicLang, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MusicLang")
include_directories(include)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.1 
)
FetchContent_MakeAvailable(fmt)

# Add source to this project's executable.
add_executable (MusicLang  "main.cpp" 
                           "src/Lexer.cpp" 
                           "src/Parser.cpp" 
                           "src/Enviroment.cpp" 
                           "src/Builtins.cpp" 
                           "src/Ast.cpp" 
                           "src/Evaluator.cpp" 
                           "src/Benchmark.cpp")

target_link_libraries(MusicLang PRIVATE fmt)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MusicLang PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
