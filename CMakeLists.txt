cmake_minimum_required(VERSION 3.12)
project(my_pistache_api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED) 

# Configure Pistache subproject to skip tests
set(PISTACHE_BUILD_TESTS OFF CACHE BOOL "Disable tests")
set(PISTACHE_INSTALL OFF CACHE BOOL "Disable installation")

# Include Pistache
add_subdirectory(lib/pistache EXCLUDE_FROM_ALL)

# Include JWT-CPP
include_directories(jwt-cpp/include)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/controllers/auth_controller.cpp
    src/services/auth_service.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    pistache
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    nlohmann_json::nlohmann_json
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/pistache/include
    ${PROJECT_SOURCE_DIR}/jwt-cpp/include
)