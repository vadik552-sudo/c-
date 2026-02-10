cmake_minimum_required(VERSION 3.20)

if(NOT MSVC)
    message(FATAL_ERROR "This project supports MSVC only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only x64 build is supported.")
endif()

add_library(KaspiKassaAPIv3 SHARED
    src/driver.cpp
    src/exports.cpp
)

target_include_directories(KaspiKassaAPIv3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include_1c
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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

target_link_libraries(KaspiKassaAPIv3 PRIVATE winhttp shell32)

target_link_options(KaspiKassaAPIv3 PRIVATE
    /DEF:${CMAKE_SOURCE_DIR}/exports.def
)

set_target_properties(KaspiKassaAPIv3 PROPERTIES
    OUTPUT_NAME "KaspiKassaAPIv3_x64"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_custom_command(TARGET KaspiKassaAPIv3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bpo_driver
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:KaspiKassaAPIv3>
        ${CMAKE_SOURCE_DIR}/bpo_driver/KaspiKassaAPIv3_x64.dll
)
