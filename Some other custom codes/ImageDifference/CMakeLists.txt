
cmake_minimum_required(VERSION 2.8)

PROJECT(ImageDifference)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ImageDifference MACOSX_BUNDLE ImageDifference)

if(VTK_LIBRARIES)
  target_link_libraries(ImageDifference ${VTK_LIBRARIES})
else()
  target_link_libraries(ImageDifference vtkHybrid vtkWidgets)
endif()
