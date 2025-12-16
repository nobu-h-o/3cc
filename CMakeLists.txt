cmake_minimum_required(VERSION 3.20)
project(3cc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set LLVM path for Homebrew installation
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/llvm")

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate lexer and parser
BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
             COMPILE_FLAGS "-d")

FLEX_TARGET(Lexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Source files
set(SOURCES
    main.cpp
    ast.cpp
    symtab.cpp
    codegen.cpp
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
        main.cpp
        codegen.cpp
        codegen.cpp
        codegen.cpp
        codegen.cpp
)

# Create executable
add_executable(3cc ${SOURCES})

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    support
    core
    irreader
    codegen
    mc
    mcparser
    option
    bitwriter
    target
    x86codegen
    aarch64codegen
    aarch64asmparser
    x86asmparser
)

target_link_libraries(3cc ${llvm_libs})

# Set compiler flags
target_compile_options(3cc PRIVATE -Wall)