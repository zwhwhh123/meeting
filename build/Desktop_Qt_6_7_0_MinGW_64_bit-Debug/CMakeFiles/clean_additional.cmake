# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appmeeting_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appmeeting_autogen.dir\\ParseCache.txt"
  "appmeeting_autogen"
  )
endif()
