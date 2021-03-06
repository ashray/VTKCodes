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
#include <vtksys/SystemTools.hxx>
#include "constants.h"
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkImageRGBToYIQ.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageWeightedSum.h>
#include <vtkImageMathematics.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageYIQToRGB.h>
#include <vtkPNGWriter.h>

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

vtkSmartPointer<vtkImageData> readInputImage(vtkSmartPointer<vtkGlobFileNames> glob, int imageNumber)
{
  vtkSmartPointer<vtkImageReader2> imageReader;
  vtkSmartPointer<vtkImageReader2Factory> readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
  std::string inputFilename = glob->GetNthFileName(imageNumber);
  cout << inputFilename << "\n";

  imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
  imageReader->SetFileName(inputFilename.c_str());
  imageReader->Update();
  return imageReader->GetOutput();
}

vtkSmartPointer<vtkImageData> extractColorChannel(vtkSmartPointer<vtkImageData>activeOutputFrame, int color_channel)
{
  vtkSmartPointer<vtkImageExtractComponents> extractFilter = vtkSmartPointer<vtkImageExtractComponents>::New();
  extractFilter->SetInputData(activeOutputFrame);
  extractFilter->SetComponents(color_channel);
  extractFilter->Update();
  return extractFilter->GetOutput();
}

void copyLowPassVariables(vtkImagePyramid *lowPass1Pointer, vtkImagePyramid *lowPass2Pointer, vtkImagePyramid *inputPyramid)
{
  lowPass1Pointer->ShallowCopy(inputPyramid);
  lowPass2Pointer->ShallowCopy(inputPyramid);
}

void getImagePyramidDimensions(vtkImagePyramid *Pyramid, int levels, int *frameSize)
{
  for (int k = 0; k < levels; k++) {
    frameSize[k] = getImageDimensions(Pyramid->vtkImagePyramidData[k]);
  }
}

void updateLowPassVariables(vtkImagePyramid *lowPass1Pointer, vtkImagePyramid *lowPass2Pointer, vtkImagePyramid *inputPyramid)
{
  vtkSmartPointer<vtkImageWeightedSum> sumFilter1 = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageWeightedSum> sumFilter2 = vtkSmartPointer<vtkImageWeightedSum>::New();
  sumFilter1->SetWeight(0,r1);
  sumFilter1->SetWeight(1,(1-r1));
  sumFilter2->SetWeight(0,r2);
  sumFilter2->SetWeight(1,(1-r2));
  for (int k=0; k<NumberOfPyramidLevels; k++)
  {
    sumFilter1->SetInputData(inputPyramid->vtkImagePyramidData[k]);
    sumFilter1->AddInputData(lowPass1Pointer->vtkImagePyramidData[k]);
    sumFilter1->Update();
    lowPass1Pointer->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
    sumFilter2->SetInputData(inputPyramid->vtkImagePyramidData[k]);
    sumFilter2->AddInputData(lowPass2Pointer->vtkImagePyramidData[k]);
    sumFilter2->Update();
    lowPass2Pointer->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());
  }
}

void pyramidDifference(vtkImagePyramid *lowPass1Pointer, vtkImagePyramid *lowPass2Pointer, vtkImagePyramid *differencePyramid, int levelCount)
{
  vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
  imageMath->SetOperationToSubtract();
  for (int k=0; k<levelCount; k++)
  {
    imageMath->SetInput1Data(lowPass1Pointer->vtkImagePyramidData[k]);
    imageMath->SetInput2Data(lowPass2Pointer->vtkImagePyramidData[k]);
    imageMath->Update();
    differencePyramid->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
  }
}

void spatialFiltering(vtkImagePyramid *differencePyramid, int levelCount, int frameSize[])
{
  vtkSmartPointer<vtkImageMathematics> differenceBooster = vtkSmartPointer<vtkImageMathematics>::New();
  differenceBooster->SetOperationToMultiplyByK();
  for (int k=1; k<levelCount-1; k++)
  {
    int currAlpha = frameSize[k]/(delta*8) - 1;
    currAlpha = currAlpha * exaggeration_factor;
    int mutiplier = currAlpha;
    if (mutiplier > alpha)
    {
      mutiplier = alpha;
    }
    differenceBooster->SetConstantK(mutiplier);
    differenceBooster->SetInput1Data(differencePyramid->vtkImagePyramidData[k]);
    differenceBooster->Update();
    differencePyramid->vtkImagePyramidData[k]->ShallowCopy(differenceBooster->GetOutput());
  }
}

void chromaticAberration(vtkSmartPointer<vtkImageData> differenceFrame, int color_channel)
{
  vtkSmartPointer<vtkImageMathematics> chromaticCorrection = vtkSmartPointer<vtkImageMathematics>::New();
  chromaticCorrection->SetOperationToMultiplyByK();
  chromaticCorrection->SetConstantK(chromatic_abberation);
  if (color_channel != YChannel)
  {
    chromaticCorrection->SetInput1Data(differenceFrame);
    chromaticCorrection->Update();
    differenceFrame->ShallowCopy(chromaticCorrection->GetOutput());
  }
}

void getOutputFrame(vtkSmartPointer<vtkImageData> colorChannelImage, vtkSmartPointer<vtkImageData> differenceFrame,
                    vtkSmartPointer<vtkImageData> outputFrameColorChannel)
{
  vtkSmartPointer<vtkImageWeightedSum> addDifferenceOrigFrameFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageMathematics> IntensityNormalisation = vtkSmartPointer<vtkImageMathematics>::New();
  addDifferenceOrigFrameFilter->SetWeight(1,.5);
  addDifferenceOrigFrameFilter->SetWeight(0,.5);
  IntensityNormalisation->SetOperationToMultiplyByK();
  IntensityNormalisation->SetConstantK(2);
  addDifferenceOrigFrameFilter->SetInputData(colorChannelImage);
  addDifferenceOrigFrameFilter->AddInputData(differenceFrame);
  addDifferenceOrigFrameFilter->Update();
  IntensityNormalisation->SetInput1Data(addDifferenceOrigFrameFilter->GetOutput());
  IntensityNormalisation->Update();
  outputFrameColorChannel->ShallowCopy(IntensityNormalisation->GetOutput());
}

void combinedOutputFrames(vtkSmartPointer<vtkImageData> *outputFrame, vtkSmartPointer<vtkImageData> YIQOutputImage)
{
  vtkSmartPointer<vtkImageAppendComponents> appendFilter = vtkSmartPointer<vtkImageAppendComponents>::New();
  vtkSmartPointer<vtkImageYIQToRGB> rgbConversionFilter = vtkSmartPointer<vtkImageYIQToRGB>::New();
  appendFilter->SetInputData(outputFrame[YChannel]);
  appendFilter->AddInputData(outputFrame[IChannel]);
  appendFilter->AddInputData(outputFrame[QChannel]);
  appendFilter->Update();

  rgbConversionFilter->SetInputConnection(appendFilter->GetOutputPort());
  rgbConversionFilter->Update();
  YIQOutputImage->ShallowCopy(rgbConversionFilter->GetOutput());
}

void frameWriter(vtkSmartPointer<vtkImageData> YIQOutputImage, int ImageNumber)
{
  std::string iterationNumberString = to_string(ImageNumber);
  std::string outputFileName = "OutputFrame" + iterationNumberString+".png";
  vtkSmartPointer<vtkPNGWriter> writeDifferenceFrames = vtkSmartPointer<vtkPNGWriter>::New();
  writeDifferenceFrames->SetFileName(outputFileName.c_str());
  writeDifferenceFrames->SetInputData(YIQOutputImage);
  writeDifferenceFrames->Write();
}