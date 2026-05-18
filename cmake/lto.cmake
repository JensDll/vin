include_guard()

include(CheckIPOSupported)

check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
  message(WARNING "LTO is not supported: ${ipo_error}")
endif()
