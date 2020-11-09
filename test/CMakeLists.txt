add_executable(${PROJECT_NAME}-test
  tests/cpu/instructions/adc.cpp
  tests/cpu/instructions/and.cpp
  tests/cpu/instructions/asl.cpp
  tests/cpu/memory.cpp
  tests/cpu/power.cpp
)

set_target_properties(${PROJECT_NAME}-test PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME}-test PROPERTIES CXX_STANDARD_REQUIRED ON)

target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} Catch2::Catch2WithMain)
