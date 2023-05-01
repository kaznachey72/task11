cmake_minimum_required(VERSION 3.10)
project(
    ex11 
    VERSION 1.0.0
    LANGUAGES C
)

set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/curl/includes/")
set(CURL_LIBRARIES   "${CMAKE_SOURCE_DIR}/curl/libs/libcurl.so")

add_subdirectory(src)
