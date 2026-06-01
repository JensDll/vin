cmake_minimum_required(VERSION 4.3)

project(Vin VERSION 0.1.0 LANGUAGES CXX)

set(project_root ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH ${project_root}/cmake)

include(options)
include(warnings)
include(ccache)
include(sanitizers)
include(compile_commands)
include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_LINK_LIBRARIES_ONLY_TARGETS ON)

add_library(vin.options INTERFACE)
add_library(vin::options ALIAS vin.options)
target_compile_options(vin.options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-march=native>)
target_compile_definitions(
  vin.options
  INTERFACE
    $<$<BOOL:${BUILD_TESTING}>:VIN_BUILD_TESTING>
    $<$<CONFIG:Release,MinSizeRel>:G_DISABLE_ASSERT>
    JSON_BRACE_INIT_COPY_SEMANTICS
)
target_sources(vin.options PUBLIC FILE_SET HEADERS BASE_DIRS "${project_root}/src")

configure_warnings(vin.options)

configure_sanitizers(
  ADDRESS ${VIN_ENABLE_SANITIZER_ADDRESS}
  LEAK ${VIN_ENABLE_SANITIZER_LEAK}
  UNDEFINED ${VIN_ENABLE_SANITIZER_UNDEFINED}
  THREAD ${VIN_ENABLE_SANITIZER_THREAD}
  MEMORY ${VIN_ENABLE_SANITIZER_MEMORY}
  TARGET vin.options
)

if(VIN_ENABLE_COVERAGE)
  target_compile_options(
    vin.options
    INTERFACE
      -fprofile-instr-generate
      -fcoverage-mapping
  )
  target_link_options(vin.options INTERFACE -fprofile-instr-generate)
endif()

if(VIN_ENABLE_CLANG_TIDY)
  include(clang_tidy)
endif()

if(VIN_ENABLE_CCACHE)
  configure_ccache()
endif()

if(VIN_ENABLE_LTO)
  include(lto)
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  link_compile_commands()
endif()

include(deps)

add_subdirectory(src)
