cmake_minimum_required(VERSION 3.28)

project(iojournal
    VERSION 0.1.0
    DESCRIPTION "Bounded logging library for C23"
    HOMEPAGE_URL "https://github.com/ioplane/iojournal"
    LANGUAGES C
)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(IOJOURNAL_BUILD_TESTS "Build unit tests" ON)
option(IOJOURNAL_BUILD_EXAMPLES "Build examples" OFF)
option(IOJOURNAL_INSTALL "Generate install targets" ON)

set(IOJOURNAL_WARNING_FLAGS
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
    -fstack-protector-strong
    -fPIC
)

set(IOJOURNAL_SOURCES
    src/core/ij_event.c
    src/core/ij_logger.c
    src/core/ij_queue.c
    src/core/ij_version.c
    src/encoders/ij_json_console.c
    src/encoders/ij_ndjson.c
    src/encoders/ij_rfc5424.c
    src/filters/ij_level_filter.c
    src/filters/ij_redact.c
    src/sinks/ij_console_sink.c
    src/sinks/ij_file_sink.c
    src/sinks/ij_syslog_tcp.c
    src/sinks/ij_syslog_udp.c
)

add_library(iojournal STATIC ${IOJOURNAL_SOURCES})
add_library(iojournal::iojournal ALIAS iojournal)

target_compile_features(iojournal PUBLIC c_std_23)
target_include_directories(iojournal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(iojournal PRIVATE ${IOJOURNAL_WARNING_FLAGS})
target_compile_definitions(iojournal PRIVATE _GNU_SOURCE)
set_target_properties(iojournal PROPERTIES EXPORT_NAME iojournal)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(iojournal PRIVATE -Wformat-overflow=2 -Wformat-truncation=2)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(iojournal PRIVATE _FORTIFY_SOURCE=3)
endif()

if(IOJOURNAL_BUILD_TESTS)
    enable_testing()

    find_path(UNITY_INCLUDE_DIR unity/unity.h
        PATHS /usr/local/include /usr/include
    )
    find_path(UNITY_SRC_DIR unity.c
        PATHS /usr/local/src/unity /usr/src/unity
    )

    if(UNITY_INCLUDE_DIR AND UNITY_SRC_DIR)
        message(STATUS "Found Unity: ${UNITY_SRC_DIR}/unity.c")
        add_library(unity STATIC ${UNITY_SRC_DIR}/unity.c)
        target_include_directories(unity PUBLIC ${UNITY_INCLUDE_DIR} ${UNITY_INCLUDE_DIR}/unity)

        macro(ij_add_test TEST_NAME TEST_SRC)
            add_executable(${TEST_NAME} ${TEST_SRC})
            target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
            target_link_libraries(${TEST_NAME} PRIVATE unity iojournal ${ARGN})
            target_compile_definitions(${TEST_NAME} PRIVATE
                IOJOURNAL_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
            )
            target_compile_options(${TEST_NAME} PRIVATE
                ${IOJOURNAL_WARNING_FLAGS}
                -Wno-missing-prototypes
                -Wno-missing-declarations
            )
            if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
                target_compile_options(${TEST_NAME} PRIVATE
                    -Wformat-overflow=2
                    -Wformat-truncation=2
                )
            endif()
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endmacro()

        add_subdirectory(tests/unit)
    else()
        message(WARNING "Unity test framework not found - tests disabled")
    endif()
endif()

if(IOJOURNAL_BUILD_EXAMPLES AND EXISTS "${CMAKE_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

find_program(CLANG_FORMAT_EXE clang-format)
file(GLOB_RECURSE IOJOURNAL_FORMAT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/tests/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.c
    ${CMAKE_SOURCE_DIR}/examples/*.h
    ${CMAKE_SOURCE_DIR}/examples/*.c
)

if(CLANG_FORMAT_EXE AND IOJOURNAL_FORMAT_SOURCES)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${IOJOURNAL_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting C sources with clang-format"
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${IOJOURNAL_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking C source formatting with clang-format"
    )
else()
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found or no C sources discovered"
    )
    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found or no C sources discovered"
    )
endif()

if(IOJOURNAL_INSTALL)
    install(TARGETS iojournal
        EXPORT iojournalTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/iojournal
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(EXPORT iojournalTargets
        FILE iojournalTargets.cmake
        NAMESPACE iojournal::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iojournal
    )
endif()
