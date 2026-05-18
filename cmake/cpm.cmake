include_guard()

set(version 0.42.3)

if(CPM_SOURCE_CACHE)
  set(location "${CPM_SOURCE_CACHE}/cpm/CPM_${version}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(location "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${version}.cmake")
else()
  set(location "${CMAKE_BINARY_DIR}/cmake/CPM_${version}.cmake")
endif()

cmake_path(ABSOLUTE_PATH location NORMALIZE OUTPUT_VARIABLE location)

if(NOT EXISTS ${location})
  message(STATUS "Downloading CPM.cmake to: ${location}")
  file(
    DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${version}/CPM.cmake
    ${location}
    SHOW_PROGRESS
    EXPECTED_HASH
      SHA512=2b6c94a21b4ada97c782c752718a1145c506b4de0d73a189f82daf66c06535794e44777923398a06ae157cd78c7cab28b12ce17b9e793a0e8661caaac03255b9
  )
endif()

include(${location})
