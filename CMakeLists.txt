cmake_minimum_required(VERSION 3.20)
project(search_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get all source files EXCEPT main.cpp and main_web.cpp
file(GLOB ALL_SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*.hpp")

# Create a list of common source files (without any main files)
set(COMMON_SRC_FILES ${ALL_SRC_FILES})
list(REMOVE_ITEM COMMON_SRC_FILES 
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/main_web.cpp"
)

# Original executable (interactive terminal version)
add_executable(main ${COMMON_SRC_FILES} "${CMAKE_SOURCE_DIR}/src/main.cpp" ${HEADER_FILES})
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Server version - long-running process (FAST!)
add_executable(main_server ${COMMON_SRC_FILES} "${CMAKE_SOURCE_DIR}/src/main_server.cpp" ${HEADER_FILES})
target_include_directories(main_server PRIVATE ${CMAKE_SOURCE_DIR}/include)