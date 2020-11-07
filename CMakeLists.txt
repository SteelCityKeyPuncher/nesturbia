cmake_minimum_required(VERSION 3.16)
project(nesturbia)

## General Settings ##

# If not specified, build as debug (at least for now)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

# Generate compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


## Library ##
add_library(${PROJECT_NAME}
  src/nesturbia.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


## Executable / Driver Program ##

# The executable can't have the same name as the library in CMake, so append "-bin" for now
# The "-bin" will be removed below
add_executable(${PROJECT_NAME}-bin main.cpp)

set_target_properties(${PROJECT_NAME}-bin PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME}-bin PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(${PROJECT_NAME}-bin nesturbia)

# Since the executable can't have the same name as the library in CMake, rename the output here
# by removing the "-bin" suffix
set_target_properties(${PROJECT_NAME}-bin PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


## Testing framework (Catch2) ##
set(CATCH2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Catch2")
add_subdirectory("${CATCH2_DIR}")

# Build the test project
add_subdirectory(test)
