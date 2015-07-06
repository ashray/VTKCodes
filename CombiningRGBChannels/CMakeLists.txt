cmake_minimum_required(VERSION 2.8)

PROJECT(CombiningRGBChannels)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(CombiningRGBChannels MACOSX_BUNDLE CombiningRGBChannels)

if(VTK_LIBRARIES)
  target_link_libraries(CombiningRGBChannels ${VTK_LIBRARIES})
else()
  target_link_libraries(CombiningRGBChannels vtkHybrid vtkWidgets)
endif()
