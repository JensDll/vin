include_guard()

function(configure_warnings target)
  set(
    warnings
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wno-conversion
    -Wno-sign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    -Wno-cast-function-type
    $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-designated-field-initializers>
    $<$<CXX_COMPILER_ID:GNU>:-Wmisleading-indentation>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
    $<$<CXX_COMPILER_ID:GNU>:-Wuseless-cast>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-field-initializers>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-return-type>
    $<$<BOOL:${CMAKE_COMPILE_WARNING_AS_ERROR}>:-Werror>
  )

  get_target_property(target_type ${target} TYPE)

  if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
    message(AUTHOR_WARNING "setting warnings for non-interface target: ${target}")
  endif()

  target_compile_options(
    ${target}
    INTERFACE
      $<$<COMPILE_LANGUAGE:CXX>:${warnings}>
      $<$<COMPILE_LANGUAGE:C>:${warnings}>
  )
endfunction()
