/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImagePyramid.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImagePyramid - tbd
// .SECTION Description
// tbd

#ifndef vtkImagePyramid_h
#define vtkImagePyramid_h

#include "vtkObjectBase.h"
#include "vtkSmartPointer.h"

#include <vector>

class vtkImageData;

class vtkImagePyramid: public vtkObjectBase
{
 public:
  //Description:
  // Empty Constructor
  vtkImagePyramid();

  //Description:
  // Constructor initialises the specified number of levels
  vtkImagePyramid(int levels);

  //Description:
  // Constructor assumes that the input image is only single channel.
  vtkImagePyramid(vtkImageData *img);

  //Description:
  // Constructor assumes that the input image is only single channel and allows user defined pyramid levels. Defaults to 6.
  vtkImagePyramid(vtkImageData *img, int PyramidLevelCount);

  //Description:
  // tbd
  ~vtkImagePyramid();

  //Description:
  // Copies one image pyramid to the other.
  void ShallowCopy(vtkImagePyramid *imp);

  //Description:
  // Collapses the image pyramid into an image
  vtkSmartPointer<vtkImageData> Collapse();

  //Description:
  // tbd
  std::vector<vtkSmartPointer<vtkImageData> > vtkImagePyramidData;
};

#endif //vtkImagePyramid_h
