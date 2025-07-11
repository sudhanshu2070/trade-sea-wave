
if(NOT "/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitinfo.txt" IS_NEWER_THAN "/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/root/my_pistache_api/build/_deps/rapidjson-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/root/my_pistache_api/build/_deps/rapidjson-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --no-checkout --depth 1 --no-single-branch --config "advice.detachedHead=false" "https://github.com/Tencent/rapidjson.git" "rapidjson-src"
    WORKING_DIRECTORY "/root/my_pistache_api/build/_deps"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/Tencent/rapidjson.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout v1.1.0 --
  WORKING_DIRECTORY "/root/my_pistache_api/build/_deps/rapidjson-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v1.1.0'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git"  submodule update --recursive --init 
    WORKING_DIRECTORY "/root/my_pistache_api/build/_deps/rapidjson-src"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/root/my_pistache_api/build/_deps/rapidjson-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitinfo.txt"
    "/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/root/my_pistache_api/build/_deps/rapidjson-subbuild/rapidjson-populate-prefix/src/rapidjson-populate-stamp/rapidjson-populate-gitclone-lastrun.txt'")
endif()

