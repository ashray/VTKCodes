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

#ifndef conveniences_h
#define conveniences_h

#include <sstream>

class vtkImageData;

//in case std::to_string not available
template <typename T>
std::string to_string(T value)
{
  //create an output string stream
  std::ostringstream os;
  //throw the value into the string stream
  os << value;
  //convert the string stream into a string and return
  return os.str();
}

std::string showDims (vtkImageData *img);

#ifdef debug
#define DEBUG printf("line number %d\n", __LINE__);
#else
#define DEBUG
#endif

#endif //conveniences_h
