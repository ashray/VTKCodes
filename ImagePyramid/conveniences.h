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
#include <vtkGlobFileNames.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include "vtkImagePyramid.h"

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

std::string showDims (vtkImageData *);
int getImageDimensions (vtkImageData *);

//Helper functions

//Description:
//Creates an image reader object setting path from command line input
vtkSmartPointer<vtkGlobFileNames> fileReaderObjectCreation(int argc, char* argv[]);

//Description:
//Reads an input image
vtkSmartPointer<vtkImageData> readInputImage(vtkSmartPointer<vtkGlobFileNames> glob, int imageNumber);

//Description:
//Extracts a specific color channel from an image
vtkSmartPointer<vtkImageData> extractColorChannel(vtkSmartPointer<vtkImageData>activeOutputFrame, int color_channel);

//Description:
//Copy image pyramid pointed by input pyramid into lowpass1 and lowpass2
void copyLowPassVariables(vtkImagePyramid *lowPass1Pointer, vtkImagePyramid *lowPass2Pointer, vtkImagePyramid *inputPyramid);

//Description:
//Returns dimensions of the image Pyramid levels
void getImagePyramidDimensions(vtkImagePyramid *Pyramid, int NumberOfPyramidLevels, int *frameSize);

//Description:
//Adds input Pyramid to lowpass 1 and lowpass 2 pyramid accumulators for first step of IIR filtering
void updateLowPassVariables(vtkImagePyramid *, vtkImagePyramid *, vtkImagePyramid *);

//Description:
//Computes difference between image Pyramids
void pyramidDifference(vtkImagePyramid *, vtkImagePyramid *, vtkImagePyramid *, int);

//Description:
//Performs spatial filtering for Eulerian Motion Magnification
void spatialFiltering(vtkImagePyramid *differencePyramid, int levelCount, int frameSize[]);

//Description:
//Performs chromatic aberration to reduce noise in output frames
void chromaticAberration(vtkSmartPointer<vtkImageData> differenceFrame, int color_channel);

//Description:
//Adds back difference and gets individual color channel of output frame
void getOutputFrame(vtkSmartPointer<vtkImageData> colorChannelImage, vtkSmartPointer<vtkImageData> differenceFrame,
                    vtkSmartPointer<vtkImageData> outputFrameColorChannel);

//Description:
//Combines output frame channels, converts YIQ color domain back to RGB
void combinedOutputFrames(vtkSmartPointer<vtkImageData> *outputFrame, vtkSmartPointer<vtkImageData> YIQOutputImage);

//Description:
//Writes output frames to disk
void frameWriter(vtkSmartPointer<vtkImageData> YIQOutputImage, int ImageNumber);


#ifdef debug
#define DEBUG printf("line number %d\n", __LINE__);
#else
#define DEBUG
#endif

#endif //conveniences_h
