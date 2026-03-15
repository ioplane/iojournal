add_executable(basic_console basic_console.c)
target_link_libraries(basic_console PRIVATE iojournal)
target_compile_features(basic_console PRIVATE c_std_23)
target_compile_options(basic_console PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wformat=2
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wunused
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wvla
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(basic_console PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
endif()

add_executable(file_sink file_sink.c)
target_link_libraries(file_sink PRIVATE iojournal)
target_compile_features(file_sink PRIVATE c_std_23)
target_compile_options(file_sink PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wformat=2
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wunused
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wvla
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(file_sink PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
endif()

add_executable(syslog_udp syslog_udp.c)
target_link_libraries(syslog_udp PRIVATE iojournal)
target_compile_features(syslog_udp PRIVATE c_std_23)
target_compile_options(syslog_udp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wformat=2
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wunused
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wvla
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(syslog_udp PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
endif()

add_executable(syslog_tcp syslog_tcp.c)
target_link_libraries(syslog_tcp PRIVATE iojournal)
target_compile_features(syslog_tcp PRIVATE c_std_23)
target_compile_options(syslog_tcp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wformat=2
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wunused
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wvla
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(syslog_tcp PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
endif()
