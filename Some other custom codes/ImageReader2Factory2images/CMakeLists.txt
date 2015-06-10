
cmake_minimum_required(VERSION 2.8)

PROJECT(2ImageReader)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(2ImageReader MACOSX_BUNDLE 2ImageReader)

if(VTK_LIBRARIES)
  target_link_libraries(2ImageReader ${VTK_LIBRARIES})
else()
  target_link_libraries(2ImageReader vtkHybrid vtkWidgets)
endif()
