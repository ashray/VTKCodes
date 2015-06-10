
cmake_minimum_required(VERSION 2.8)

PROJECT(ImageReader2Factory)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ImageReader2Factory MACOSX_BUNDLE ImageReader2Factory)

if(VTK_LIBRARIES)
  target_link_libraries(ImageReader2Factory ${VTK_LIBRARIES})
else()
  target_link_libraries(ImageReader2Factory vtkHybrid vtkWidgets)
endif()
