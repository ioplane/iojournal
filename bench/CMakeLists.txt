macro(ij_add_bench TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE iojournal)
    target_compile_features(${TARGET_NAME} PRIVATE c_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE _GNU_SOURCE)
    target_compile_options(${TARGET_NAME} PRIVATE ${IOJOURNAL_WARNING_FLAGS})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
    endif()
endmacro()

ij_add_bench(bench_hot_path bench_hot_path.c)
ij_add_bench(bench_file_sink bench_file_sink.c)
ij_add_bench(bench_file_sink_uring bench_file_sink_uring.c)
ij_add_bench(bench_simd_scan bench_simd_scan.c)
ij_add_bench(bench_syslog_udp bench_syslog_udp.c)

if(IOJOURNAL_BUILD_TESTS)
    add_test(
        NAME bench_hot_path_medium_message
        COMMAND bench_hot_path 10 --tsv --scenario medium_message
    )
    set_tests_properties(bench_hot_path_medium_message PROPERTIES
        PASS_REGULAR_EXPRESSION "bench_hot_path\tmedium_message\t10\t"
    )

    add_test(
        NAME bench_hot_path_medium_message_with_metadata
        COMMAND bench_hot_path 10 --tsv --scenario medium_message_with_metadata
    )
    set_tests_properties(bench_hot_path_medium_message_with_metadata PROPERTIES
        PASS_REGULAR_EXPRESSION "bench_hot_path\tmedium_message_with_metadata\t10\t"
    )

    add_test(
        NAME bench_hot_path_contention_mpsc
        COMMAND bench_hot_path 16 --tsv --scenario contention_mpsc
    )
    set_tests_properties(bench_hot_path_contention_mpsc PROPERTIES
        PASS_REGULAR_EXPRESSION "bench_hot_path\tcontention_mpsc\t16\t"
    )
endif()
