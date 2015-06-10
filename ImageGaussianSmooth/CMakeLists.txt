
cmake_minimum_required(VERSION 2.8)

PROJECT(ImageGaussianSmooth)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ImageGaussianSmooth MACOSX_BUNDLE ImageGaussianSmooth)

if(VTK_LIBRARIES)
  target_link_libraries(ImageGaussianSmooth ${VTK_LIBRARIES})
else()
  target_link_libraries(ImageGaussianSmooth vtkHybrid vtkWidgets)
endif()
