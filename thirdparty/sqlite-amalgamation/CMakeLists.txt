cmake_minimum_required(VERSION 3.16)

# Create SQLite static library
add_library(sqlite3 STATIC
    sqlite3.c
    sqlite3.h
    sqlite3ext.h
)

# Set target properties
target_include_directories(sqlite3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Disable warnings for SQLite code since it's third-party
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /w)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

# Define SQLite compile options for better performance and features
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_ENABLE_FTS5=1
    SQLITE_ENABLE_JSON1=1
    SQLITE_ENABLE_RTREE=1
    SQLITE_ENABLE_COLUMN_METADATA=1
) 