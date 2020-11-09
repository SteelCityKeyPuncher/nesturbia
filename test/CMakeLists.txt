add_executable(${PROJECT_NAME}-test
  tests/cpu/instructions/adc.cpp
  tests/cpu/instructions/and.cpp
  tests/cpu/instructions/asl.cpp
  tests/cpu/instructions/bcc.cpp
  tests/cpu/instructions/bcs.cpp
  tests/cpu/instructions/beq.cpp
  tests/cpu/instructions/bit.cpp
  tests/cpu/instructions/bmi.cpp
  tests/cpu/instructions/bne.cpp
  tests/cpu/instructions/bpl.cpp
  tests/cpu/instructions/brk.cpp
  tests/cpu/instructions/bvc.cpp
  tests/cpu/instructions/bvs.cpp
  tests/cpu/instructions/clc.cpp
  tests/cpu/instructions/cld.cpp
  tests/cpu/instructions/cli.cpp
  tests/cpu/instructions/clv.cpp
  tests/cpu/instructions/cmp.cpp
  tests/cpu/instructions/cpx.cpp
  tests/cpu/instructions/cpy.cpp
  tests/cpu/memory.cpp
  tests/cpu/power.cpp
)

set_target_properties(${PROJECT_NAME}-test PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME}-test PROPERTIES CXX_STANDARD_REQUIRED ON)

target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} Catch2::Catch2WithMain)
