cmake_minimum_required(VERSION 2.8)
 
PROJECT(RGBToHSV)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(RGBToHSV MACOSX_BUNDLE RGBToHSV)
 
if(VTK_LIBRARIES)
  target_link_libraries(RGBToHSV ${VTK_LIBRARIES})
else()
  target_link_libraries(RGBToHSV vtkHybrid vtkWidgets)
endif()