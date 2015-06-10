
cmake_minimum_required(VERSION 2.8)

PROJECT(AllFramesDifferenceCalculation)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(AllFramesDifferenceCalculation MACOSX_BUNDLE AllFramesDifferenceCalculation
)

if(VTK_LIBRARIES)
  target_link_libraries(AllFramesDifferenceCalculation
  ${VTK_LIBRARIES})
else()
  target_link_libraries(AllFramesDifferenceCalculation
  vtkHybrid vtkWidgets)
endif()
