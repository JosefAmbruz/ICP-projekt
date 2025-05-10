# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/icp_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/icp_autogen.dir/ParseCache.txt"
  "icp_autogen"
  "nodeeditor_build/CMakeFiles/QtNodes_autogen.dir/AutogenUsed.txt"
  "nodeeditor_build/CMakeFiles/QtNodes_autogen.dir/ParseCache.txt"
  "nodeeditor_build/QtNodes_autogen"
  )
endif()
