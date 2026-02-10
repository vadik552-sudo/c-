cmake_minimum_required(VERSION 3.20)

project(KaspiKassaAPIv3 LANGUAGES CXX)

if(NOT MSVC)
    message(FATAL_ERROR "This project supports MSVC only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only x64 build is supported.")
endif()

add_library(KaspiKassaAPIv3 SHARED
    src/AddInNative.cpp
    src/Component.cpp
    src/WinHttpClient.cpp
)

target_include_directories(KaspiKassaAPIv3 PRIVATE
    include_1c
    src
)

target_compile_features(KaspiKassaAPIv3 PRIVATE cxx_std_17)

target_compile_definitions(KaspiKassaAPIv3 PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(KaspiKassaAPIv3 PRIVATE
    /permissive-
    /W4
    /utf-8
)

target_link_libraries(KaspiKassaAPIv3 PRIVATE
    winhttp
)

target_link_options(KaspiKassaAPIv3 PRIVATE
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/exports.def
)

set_target_properties(KaspiKassaAPIv3 PROPERTIES
    OUTPUT_NAME "KaspiKassaAPIv3_x64"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
