cmake_minimum_required(VERSION 2.8)
 
PROJECT(RGBToYIQ)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(RGBToYIQ MACOSX_BUNDLE RGBToYIQ)
 
if(VTK_LIBRARIES)
  target_link_libraries(RGBToYIQ ${VTK_LIBRARIES})
else()
  target_link_libraries(RGBToYIQ vtkHybrid vtkWidgets)
endif()