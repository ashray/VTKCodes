
cmake_minimum_required(VERSION 2.8)

PROJECT(InputMultipleImages)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(InputMultipleImages MACOSX_BUNDLE InputMultipleImages
)

if(VTK_LIBRARIES)
  target_link_libraries(InputMultipleImages
  ${VTK_LIBRARIES})
else()
  target_link_libraries(InputMultipleImages
  vtkHybrid vtkWidgets)
endif()
