/*=========================================================================

  Program:   Visualization Toolkit
  Module:    conveniences.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "conveniences.h"
#include <sstream>
#include "vtkImageData.h"

std::string showDims (vtkImageData *img)
{
  std::ostringstream strm;
  int *exts;
  exts = img->GetExtent();
  strm << "(" << exts[1] - exts[0] << ", " << exts[3] - exts[2] << ", " << exts[5] - exts[4] << ")";
  return strm.str();
}
