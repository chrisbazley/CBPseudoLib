cmake_minimum_required(VERSION 3.14)
project(CBPseudoLib C)

option(USE_OPTIONAL "Enable the _Optional qualifier" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

include(FetchContent)

FetchContent_Declare(OptionalAcornC
    GIT_REPOSITORY https://github.com/chrisbazley/OptionalAcornC.git
    GIT_TAG main)
FetchContent_Declare(CBUtil
    GIT_REPOSITORY https://github.com/chrisbazley/CBUtilLib.git
    GIT_TAG main)
FetchContent_Declare(CBDebug
    GIT_REPOSITORY https://github.com/chrisbazley/CBDebugLib.git
    GIT_TAG main)
FetchContent_Declare(Fortify
    GIT_REPOSITORY https://github.com/chrisbazley/Fortify.git
    GIT_TAG main)

FetchContent_MakeAvailable(OptionalAcornC CBUtil CBDebug Fortify)

if(USE_OPTIONAL)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR
            "USE_OPTIONAL requires Christopher Bazley's Clang fork")
    endif()

    add_compile_definitions(USE_OPTIONAL)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS NO)

if(MSVC)
    add_compile_options(/W4 /wd4200 /wd4996 /wd4130)
else()
    add_compile_options(-Wall -Wextra -Wsign-compare -pedantic)
endif()

if(ENABLE_CLANG_TIDY)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR "ENABLE_CLANG_TIDY requires Clang")
    endif()

    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        HINTS "/opt/homebrew/opt/llvm/bin")

    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR
            "ENABLE_CLANG_TIDY is ON, but clang-tidy was not found")
    endif()

    set(CMAKE_C_CLANG_TIDY
        "${CLANG_TIDY_EXE}"
        "--checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*"
        "--warnings-as-errors=clang-analyzer-optionality.*")
endif()

set(SOURCES
    PseudoEvnt.c
    PseudoExit.c
    PseudoFlex.c
    PseudoKern.c
    PseudoTbox.c
    PseudoWimp.c)

set(PUBLIC_HEADERS
    PseudoEvnt.h
    PseudoExit.h
    PseudoFlex.h
    PseudoIO.h
    PseudoKern.h
    PseudoTbox.h
    PseudoWimp.h)

add_library(CBPseudo STATIC
    ${SOURCES}
    ${PUBLIC_HEADERS}
    Internal/CBPsdoMisc.h)

set_target_properties(CBPseudo PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
    DEBUG_POSTFIX dbg)

target_include_directories(CBPseudo PUBLIC .)

target_compile_definitions(CBPseudo PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>)

target_link_libraries(CBPseudo PUBLIC
    OptionalAcornC
    CBUtil
    CBDebug
    Fortify)

install(TARGETS CBPseudo
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include)
