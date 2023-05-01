find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2 glib-2.0>2.6)
pkg_check_modules (GIO2 gio-2.0)

add_executable(
    ex11 
    main.c
    fs_utils.c
    hash_table.c
    thread_process.c
    log_parser.c
)
target_compile_options(
    ex11
    PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
    -std=c11
    -D_GNU_SOURCE
)
target_include_directories(
    ex11 
    PRIVATE 
    ${GLIB2_INCLUDE_DIRS} 
    ${GIO2_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIR} 
)
target_link_libraries(
    ex11 
    PRIVATE  
    ${GIO2_LIBRARIES}
    ${CURL_LIBRARIES}
)
