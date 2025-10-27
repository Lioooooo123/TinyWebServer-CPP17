cmake_minimum_required(VERSION 3.15)

project(TinyWebServer
    VERSION 1.0.0
    DESCRIPTION "A lightweight C++17 web server"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler options
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wextra -Wpedantic")

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
)

find_library(MYSQL_LIBRARY
    NAMES mysqlclient
    HINTS
    /usr/lib
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu
)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL client library not found. Please install libmysqlclient-dev")
endif()

message(STATUS "MySQL include directory: ${MYSQL_INCLUDE_DIR}")
message(STATUS "MySQL library: ${MYSQL_LIBRARY}")

# Source files
set(SERVER_SOURCES
    main.cpp
    config.cpp
    webserver.cpp
    timer/lst_timer.cpp
    http/http_conn.cpp
    log/log.cpp
    CGImysql/sql_connection_pool.cpp
)

# Header files (for IDEs)
set(SERVER_HEADERS
    config.h
    webserver.h
    timer/lst_timer.h
    http/http_conn.h
    log/log.h
    log/block_queue.h
    lock/locker.h
    threadpool/threadpool.h
    CGImysql/sql_connection_pool.h
)

# Create executable
add_executable(server ${SERVER_SOURCES} ${SERVER_HEADERS})

# Include directories
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/timer
    ${CMAKE_CURRENT_SOURCE_DIR}/http
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${CMAKE_CURRENT_SOURCE_DIR}/lock
    ${CMAKE_CURRENT_SOURCE_DIR}/threadpool
    ${CMAKE_CURRENT_SOURCE_DIR}/CGImysql
    ${MYSQL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(server PRIVATE
    Threads::Threads
    ${MYSQL_LIBRARY}
)

# Installation
install(TARGETS server
    RUNTIME DESTINATION bin
)

# Install resource files
install(DIRECTORY root/
    DESTINATION share/TinyWebServer/root
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.gif"
    PATTERN "*.jpg"
    PATTERN "*.ico"
    PATTERN "*.mp4"
)

# Install configuration
install(FILES config/server.conf
    DESTINATION etc/TinyWebServer
)

# Print build information
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
