cmake_minimum_required(VERSION 3.16)
project(nesturbia)

## General Settings ##

# Use the release build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Generate compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


## Library ##
add_library(${PROJECT_NAME}
  src/apu.cpp
  src/cpu.cpp
  src/mapper.cpp
  src/mappers/mapper0.cpp
  src/nesturbia.cpp
  src/ppu.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror -pedantic>
)


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
