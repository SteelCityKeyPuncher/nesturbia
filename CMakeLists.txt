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
  src/cartridge.cpp
  src/cpu.cpp
  src/joypad.cpp
  src/mapper.cpp
  src/mappers/mapper0.cpp
  src/mappers/mapper1.cpp
  src/nesturbia.cpp
  src/ppu.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror -pedantic -Ofast>
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


## Third-party code ##

# GLFW (graphics context creation)
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME}-bin "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME}-bin PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME}-bin PRIVATE "GLFW_INCLUDE_NONE")

# glad (GL loader generator)
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME}-bin PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME}-bin "glad" "${CMAKE_DL_LIBS}")

set(PORTAUDIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portaudio")
set(PA_BUILD_SHARED OFF CACHE INTERNAL "Build PortAudio shared library")
add_subdirectory("${PORTAUDIO_DIR}")
target_link_libraries(${PROJECT_NAME}-bin "portaudio_static")
target_include_directories(${PROJECT_NAME}-bin PRIVATE "${PORTAUDIO_DIR}/include")

# Catch2 (testing framework)
set(CATCH2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Catch2")
add_subdirectory("${CATCH2_DIR}")

# Build the test project
# TODO add an option so that this isn't built by default
add_subdirectory(test)
