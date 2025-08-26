cmake_minimum_required(VERSION 3.12)
project(my_pistache_api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Fetch dependencies
include(FetchContent)

# Fetch CPR
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.10.5
)
FetchContent_MakeAvailable(cpr)

# Fetch nlohmann_json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Add Pistache subdirectory
set(PISTACHE_BUILD_TESTS OFF CACHE BOOL "Disable tests")
set(PISTACHE_INSTALL OFF CACHE BOOL "Disable installation")
add_subdirectory(lib/pistache EXCLUDE_FROM_ALL)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
    include/controllers/*.cpp
    include/services/*.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    pistache
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
)