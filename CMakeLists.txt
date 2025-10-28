cmake_minimum_required(VERSION 3.15)

project(TinyWebServer
    VERSION 1.0.0
    DESCRIPTION "A lightweight C++17 web server with modern design"
    LANGUAGES CXX
)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler options based on compiler ID
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wuseless-cast
        -Wdouble-promotion
        -Wformat=2
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(Threads REQUIRED)

# Find MySQL client library
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    HINTS
    /usr/include
    /usr/local/include
    /opt/homebrew/include  # For macOS with Homebrew
)

find_library(MYSQL_LIBRARY
    NAMES mysqlclient
    HINTS
    /usr/lib
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu
    /opt/homebrew/lib  # For macOS with Homebrew
)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL client library not found. Please install libmysqlclient-dev")
endif()

message(STATUS "MySQL include directory: ${MYSQL_INCLUDE_DIR}")
message(STATUS "MySQL library: ${MYSQL_LIBRARY}")

# Check for required filesystem library (needed for some compilers)
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(std::filesystem::path "filesystem" HAVE_STD_FILESYSTEM)
if(NOT HAVE_STD_FILESYSTEM)
    find_library(STD_FS_LIBRARY stdc++fs)
    if(STD_FS_LIBRARY)
        message(STATUS "Found std::filesystem library: ${STD_FS_LIBRARY}")
    endif()
endif()

# Source files grouped by module
set(LOG_SOURCES
    log/log.cpp
)

set(TIMER_SOURCES
    timer/lst_timer.cpp
)

set(HTTP_SOURCES
    http/http_conn.cpp
)

set(SQL_SOURCES
    CGImysql/sql_connection_pool.cpp
)

set(CORE_SOURCES
    config.cpp
    webserver.cpp
    main.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${LOG_SOURCES}
    ${TIMER_SOURCES}
    ${HTTP_SOURCES}
    ${SQL_SOURCES}
)

# Header files (for IDE support)
set(ALL_HEADERS
    config.h
    webserver.h
    log/log.h
    log/block_queue.h
    timer/lst_timer.h
    http/http_conn.h
    threadpool/threadpool.h
    CGImysql/sql_connection_pool.h
)

# Create executable target
add_executable(server ${ALL_SOURCES} ${ALL_HEADERS})

# Set target properties
set_target_properties(server PROPERTIES
    OUTPUT_NAME "tinywebserver"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MYSQL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(server PRIVATE
    Threads::Threads
    ${MYSQL_LIBRARY}
)

# Link filesystem library if needed
if(STD_FS_LIBRARY)
    target_link_libraries(server PRIVATE ${STD_FS_LIBRARY})
endif()

# Compiler definitions
target_compile_definitions(server PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
    $<$<CONFIG:Release>:NDEBUG>
)

# Installation
install(TARGETS server
    RUNTIME DESTINATION bin
    COMPONENT runtime
)

# Install resource files
install(DIRECTORY root/
    DESTINATION share/tinywebserver/root
    COMPONENT resources
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.gif"
    PATTERN "*.jpg"
    PATTERN "*.ico"
    PATTERN "*.mp4"
    PATTERN "README.md" EXCLUDE
)

# Install configuration
install(FILES config/server.conf
    DESTINATION etc/tinywebserver
    COMPONENT config
)

# Install documentation
install(FILES README.md LICENSE
    DESTINATION share/doc/tinywebserver
    COMPONENT documentation
    OPTIONAL
)

# Uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()

# Print build information
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  MySQL include: ${MYSQL_INCLUDE_DIR}")
message(STATUS "  MySQL library: ${MYSQL_LIBRARY}")
message(STATUS "")
message(STATUS "=== Build Targets ===")
message(STATUS "  make         - Build the server")
message(STATUS "  make install - Install the server")
if(TARGET uninstall)
    message(STATUS "  make uninstall - Uninstall the server")
endif()
message(STATUS "=======================")
message(STATUS "")
