/*=========================================================================

  Program:   Visualization Toolkit
  Module:    main.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

//Program to demonstrate Eulerian Motion Magnification
//by Ashray Malhotra
//for more information see http://www.kitware.com/blog/home/post/952

#include "vtkImagePyramid.h"
#include <vtkDataSetMapper.h>
#include <vtkGlobFileNames.h>
#include <vtkImageActor.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImageRGBToYIQ.h>
#include <vtkImageSincInterpolator.h>
#include <vtkImageViewer2.h>
#include <vtkPNGWriter.h>
#include <vtkStringArray.h>
#include "conveniences.h"
#include "constants.h"


int main(int argc, char* argv[])
{

// --------Variable definitions ---------
  vtkSmartPointer<vtkGlobFileNames> glob;
  vtkSmartPointer<vtkImageRGBToYIQ> yiqFilter = vtkSmartPointer<vtkImageRGBToYIQ>::New();
  vtkSmartPointer<vtkImageData> colorChannelImage;
  vtkImagePyramid *Pyramid;
  vtkImagePyramid *lowPass1[3];
  vtkImagePyramid *lowPass2[3];
  int frameSize[NumberOfPyramidLevels];
  vtkImagePyramid *differencePyramid;
  vtkSmartPointer<vtkImageData> differenceFrame;
  vtkSmartPointer<vtkImageData> outputFrame[3];
  vtkSmartPointer<vtkImageData> YIQOutputImage = vtkSmartPointer<vtkImageData>::New();

//  Setup
  for(int i = 0; i < 3; i++)
  {
    lowPass1[i] = NULL;
    lowPass2[i] = NULL;
  }

//  Creating reader to read images
  glob = fileReaderObjectCreation(argc, argv);
  for (int ImageNumber = 0; ImageNumber < glob->GetNumberOfFileNames(); ImageNumber++)
    {
//      Get input image
    vtkSmartPointer<vtkImageData> inputImage;
    inputImage = readInputImage(glob, ImageNumber);

//    Convert the input frame into YIQ
    yiqFilter->SetInputData(inputImage);
    yiqFilter->Update();

    for (int color_channel=0; color_channel<3; color_channel++){
//      Because ShallowCopy()s don't seem to clear old data
      differencePyramid = new vtkImagePyramid(NumberOfPyramidLevels);

//      Extract color channel
      colorChannelImage = extractColorChannel(yiqFilter->GetOutput(), color_channel);

//      Create image pyramid
      Pyramid = new vtkImagePyramid(colorChannelImage, NumberOfPyramidLevels);

//      Initialising lowPass with first frame
      if (ImageNumber==0)
      {
        lowPass1[color_channel] = new vtkImagePyramid();
        lowPass2[color_channel] = new vtkImagePyramid();
        copyLowPassVariables(lowPass1[color_channel], lowPass2[color_channel], Pyramid);

//        Inserting the code below since it only needs to be done once(hence only for frame 1)
        if (color_channel == 0)
        {
          getImagePyramidDimensions(Pyramid, NumberOfPyramidLevels, frameSize);
        }
      }

      else
      {
//        Temporal IIR(Infinite impulse response filtering)
        updateLowPassVariables(lowPass1[color_channel], lowPass2[color_channel], Pyramid);
        pyramidDifference(lowPass1[color_channel], lowPass2[color_channel], differencePyramid, NumberOfPyramidLevels);

//        Spatial filtering
        spatialFiltering(differencePyramid, NumberOfPyramidLevels, frameSize);

//        Collapse difference Pyramid
        differenceFrame = differencePyramid->Collapse();

//        Chromatic aberration to reduce noise
        chromaticAberration(differenceFrame, color_channel);

//        Add back frame difference to the original frame
        outputFrame[color_channel] = vtkSmartPointer<vtkImageData>::New();
        getOutputFrame(colorChannelImage, differenceFrame, outputFrame[color_channel]);
      }
      delete differencePyramid;
    }

    if(ImageNumber!=0)
    {
      combinedOutputFrames(outputFrame, YIQOutputImage);

//      Write output frames to disk
      frameWriter(YIQOutputImage, ImageNumber);
    }
  }
  return EXIT_SUCCESS;
}