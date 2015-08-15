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

#include "vtkImagePyramid.h"

#include <vtkImageConvolve.h>
#include <vtkImageData.h>
#include <vtkImageDifference.h>
#include <vtkImageResize.h>
#include <vtkSmartPointer.h>
#include <vtkImageMathematics.h>

#include "conveniences.h"
#include "constants.h"

// If PyramidChoice is 1, then we get a laplacian pyramid, if it is zero we get a gaussian pyramid
#define PyramidChoice 1
//------------------------------------------------------------------------------
vtkImagePyramid::vtkImagePyramid()
{
}

//------------------------------------------------------------------------------
vtkImagePyramid::vtkImagePyramid(int levels)
{
  this->vtkImagePyramidData.resize(levels);
  for(size_t i = 0; i < levels; ++i)
    {
    this->vtkImagePyramidData[i] = vtkSmartPointer<vtkImageData>::New();
    }
}

//------------------------------------------------------------------------------
vtkImagePyramid::vtkImagePyramid(vtkImageData *img)
{
  vtkImagePyramid(img, NumberOfPyramidLevels);
}

//------------------------------------------------------------------------------
vtkImagePyramid::vtkImagePyramid(vtkImageData *img, int PyramidLevelCount)
{

  // vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter;
  vtkSmartPointer<vtkImageResize> resize;
  int *imageDimensionArray = img->GetExtent();
  int imageDimension1 = imageDimensionArray[1] - imageDimensionArray[0] + 1;
  int imageDimension2 = imageDimensionArray[3] - imageDimensionArray[2] + 1;
  vtkImagePyramidData.push_back(img);
  for (int i=1; i<PyramidLevelCount; i++)
    {
    vtkSmartPointer<vtkImageConvolve> convolveFilter =
      vtkSmartPointer<vtkImageConvolve>::New();
    convolveFilter->SetInputData(vtkImagePyramidData[i-1]);
    double kernel[25] = {1,4,6,4,1,4,16,24,16,4,6,24,36,24,6,4,16,24,16,4,1,4,6,4,1};
    for (int internal_loop=0; internal_loop<25;internal_loop++)
      {
      kernel[internal_loop] = kernel[internal_loop]/256;
      }

    convolveFilter->SetKernel5x5(kernel);
    convolveFilter->Update();

    resize = vtkSmartPointer<vtkImageResize>::New();
    resize->SetResizeMethodToOutputDimensions();
    resize->SetInputData(convolveFilter->GetOutput());
    resize->SetOutputDimensions(imageDimension1/(pow(2,i)), imageDimension2/(pow(2,i)), 1);
    resize->Update();

    vtkImagePyramidData.push_back(resize->GetOutput());
    }

// ------------If we remove the loop below we can get gaussian pyramids, but if we use the code below we get laplacian pyramids. Make a functionality for this.
if(PyramidChoice==1)
{
  for (int i=1; i<PyramidLevelCount-1; i++)
    {
    resize = vtkSmartPointer<vtkImageResize>::New();
    resize->SetResizeMethodToOutputDimensions();
    resize->SetInputData(vtkImagePyramidData[i+1]);
    resize->SetOutputDimensions(imageDimension1/(pow(2,i)), imageDimension2/(pow(2,i)), 1);
    resize->Update();
    vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
    imageMath->SetOperationToSubtract();
    imageMath->SetInput1Data(vtkImagePyramidData[i]);
    imageMath->SetInput2Data(resize->GetOutput());
    imageMath->Update();
    vtkImagePyramidData[i] = imageMath->GetOutput();

    // vtkSmartPointer<vtkImageDifference> differenceFilter2 =
    //   vtkSmartPointer<vtkImageDifference>::New();
    // differenceFilter2->AllowShiftOff();
    // differenceFilter2->AveragingOff();
    // differenceFilter2->SetAllowShift(0);
    // differenceFilter2->SetThreshold(0);
    // differenceFilter2->SetInputData(vtkImagePyramidData[i]);
    // differenceFilter2->SetImageData(resize->GetOutput());
    // differenceFilter2->Update();
    // vtkImagePyramidData[i] = differenceFilter2->GetOutput();
    }
  }

}

//------------------------------------------------------------------------------
vtkImagePyramid::~vtkImagePyramid()
{
  // TODO - Complete the destructor
}

//------------------------------------------------------------------------------
void vtkImagePyramid::ShallowCopy(vtkImagePyramid *imp)
{
  this->vtkImagePyramidData.clear();
  this->vtkImagePyramidData.resize(imp->vtkImagePyramidData.size()); // Not strictly necessary
  for(size_t i = 0; i != imp->vtkImagePyramidData.size(); ++i)
    {
    DEBUG
      this->vtkImagePyramidData[i] = vtkSmartPointer<vtkImageData>::New();
    this->vtkImagePyramidData[i]->ShallowCopy(imp->vtkImagePyramidData[i]);
    DEBUG
    }
}
