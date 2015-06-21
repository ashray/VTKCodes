
cmake_minimum_required(VERSION 2.8)

PROJECT(ImagePyramid)

set(CMAKE_BUILD_TYPE Debug)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ImagePyramid MACOSX_BUNDLE d ImagePyramid)

if(VTK_LIBRARIES)
  target_link_libraries(ImagePyramid ${VTK_LIBRARIES})
else()
  target_link_libraries(ImagePyramid vtkHybrid vtkWidgets)
endif()
