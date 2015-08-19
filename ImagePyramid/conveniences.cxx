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
#include <vtksys/SystemTools.hxx>
#include <vtkGlobFileNames.h>
#include <vtkSmartPointer.h>
#include "vtkImageData.h"
#include <vtksys/SystemTools.hxx>

std::string showDims (vtkImageData *img)
{
  std::ostringstream strm;
  int *exts;
  exts = img->GetExtent();
  strm << "(" << exts[1] - exts[0] << ", " << exts[3] - exts[2] << ", " << exts[5] - exts[4] << ")";
  return strm.str();
}

int getImageDimensions (vtkImageData *img)
{
  int *a = img->GetExtent();
  return int(pow((pow(a[1] - a[0] + 1,2) + pow(a[3] - a[2] + 1,2)), 0.5)/3);
  // 3 is an experimental constant used by the authors of the paper
}

vtkSmartPointer<vtkGlobFileNames> fileReaderObjectCreation(int argc, char* argv[])
{
  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  if (argc < 2)
  {
    cerr << "WARNING: Expected arguments" << argv[0] << "/directory/path/to/numbered/image/files/" << endl;
    cerr << "defaulting to current working directory " << endl;
  }
  else
  {
    std::string dirString;
    dirString = argv[1];
    vtksys::SystemTools::ConvertToUnixSlashes(dirString);
    glob->SetDirectory(dirString.c_str());
  }

  glob->AddFileNames("*");
  cout << "Number of Images read " << glob->GetNumberOfFileNames() << "\n";
  return glob;
}