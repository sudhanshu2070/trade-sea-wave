cmake_minimum_required(VERSION 3.15)
project(my_pistache_api LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Dependencies
# -------------------------
include(FetchContent)

# CPR (HTTP Client)
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.10.5
)
FetchContent_MakeAvailable(cpr)

# nlohmann_json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Pistache (local submodule under lib/pistache)
set(PISTACHE_BUILD_TESTS OFF CACHE BOOL "Disable Pistache tests")
set(PISTACHE_INSTALL OFF CACHE BOOL "Disable Pistache install")
# add_subdirectory(lib/pistache EXCLUDE_FROM_ALL)

# -------------------------
# Source files
# -------------------------
file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
    include/controllers/*.cpp
    include/services/*.cpp
    include/models/*.cpp
)

# -------------------------
# Executable
# -------------------------
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include paths (for headers in include/)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# -------------------------
# Disable sanitizers for memory-intensive backtest files
# -------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # List of files that cause memory issues with sanitizers
    set(BACKTEST_FILES
        "services/BacktestService.cpp"
        "models/RenkoBuilder.cpp"
        "models/IchimokuCalculator.cpp"
    )
    
    # Disable all sanitizers for backtest files
    foreach(backtest_file ${BACKTEST_FILES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/include/${backtest_file}")
            set_source_files_properties(
                "include/${backtest_file}"
                PROPERTIES
                COMPILE_FLAGS "-fno-sanitize=address -fno-sanitize=leak -fno-sanitize=undefined -fno-sanitize=thread"
            )
            message(STATUS "Disabled sanitizers for: include/${backtest_file}")
        endif()
        
        if(EXISTS "${PROJECT_SOURCE_DIR}/src/${backtest_file}")
            set_source_files_properties(
                "src/${backtest_file}"
                PROPERTIES
                COMPILE_FLAGS "-fno-sanitize=address -fno-sanitize=leak -fno-sanitize=undefined -fno-sanitize=thread"
            )
            message(STATUS "Disabled sanitizers for: src/${backtest_file}")
        endif()
    endforeach()
endif()

# -------------------------
# Alternatively, disable sanitizers globally if still having issues
# -------------------------
option(USE_SANITIZERS "Enable address, leak, and undefined behavior sanitizers" OFF)
if(NOT USE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fno-sanitize=address -fno-sanitize=leak -fno-sanitize=undefined -fno-sanitize=thread)
        add_link_options(-fno-sanitize=address -fno-sanitize=leak -fno-sanitize=undefined -fno-sanitize=thread)
    endif()
    message(STATUS "Sanitizers disabled globally")
endif()

# -------------------------
# Linking
# -------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    pistache
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# -------------------------
# Install target (optional)
# -------------------------
install(TARGETS ${PROJECT_NAME} DESTINATION bin)