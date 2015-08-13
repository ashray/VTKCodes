cmake_minimum_required (VERSION 2.8)

project (ImagePyramid)

set (CMAKE_BUILD_TYPE Debug)

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

set (files
  conveniences.h
  conveniences.cxx
  main.cxx
  vtkImagePyramid.h
  vtkImagePyramid.cxx
)

add_executable (ImagePyramid MACOSX_BUNDLE ${files})

target_link_libraries (ImagePyramid ${VTK_LIBRARIES})
