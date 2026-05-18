include_guard()

find_program(
  CLANG_TIDY
  REQUIRED
  NAMES
    clang-tidy
    clang-tidy-22
    clang-tidy-21
    clang-tidy-20
)

set(
  command
  ${CLANG_TIDY}
  -p
  ${project_root}
  -extra-arg=-std=c++${CMAKE_CXX_STANDARD}
)

if(CMAKE_COMPILE_WARNING_AS_ERROR)
  list(APPEND command -warnings-as-errors=*)
endif()

message(STATUS "Using clang-tidy command line: ${command}")

set(CMAKE_CXX_CLANG_TIDY ${command})
