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

#define YChannel 0
#define IChannel 1
#define QChannel 2

// #define debug

#include "vtkImagePyramid.h"

#include <vtkDataSetMapper.h>
#include <vtkGlobFileNames.h>
#include <vtkImageActor.h>
#include <vtkImageAppendComponents.h>

#include <vtkImageData.h>
#include <vtkImageDifference.h>

#include <vtkImageMapper3D.h>
#include <vtkImageMathematics.h>
#include <vtkImageRGBToYIQ.h>
#include <vtkImageSincInterpolator.h>
#include <vtkImageViewer2.h>
#include <vtkImageWeightedSum.h>
#include <vtkImageYIQToRGB.h>
#include <vtkPNGWriter.h>
#include <vtkStringArray.h>

#include <vtksys/SystemTools.hxx>
#include <cmath>
#include <sstream>

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

  vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();

  vtkSmartPointer<vtkImageMathematics> chromaticCorrection = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageWeightedSum> addDifferenceOrigFrameFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageMathematics> IntensityNormalisation = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageAppendComponents> appendFilter = vtkSmartPointer<vtkImageAppendComponents>::New();

  vtkImagePyramid *differencePyramid;

  vtkSmartPointer<vtkImageData> differenceFrame;
  vtkSmartPointer<vtkImageData> outputFrame[3];
  vtkSmartPointer<vtkImageData> activeOutputFrame;
  vtkSmartPointer<vtkImageYIQToRGB> rgbConversionFilter = vtkSmartPointer<vtkImageYIQToRGB>::New();

// ------Setup -----
  for(int i = 0; i < 3; i++)
  {
    lowPass1[i] = NULL;
    lowPass2[i] = NULL;
  }

  differenceFilter->AllowShiftOff();
  differenceFilter->AveragingOff();
  differenceFilter->SetAllowShift(0);
  differenceFilter->SetThreshold(0);


  chromaticCorrection->SetOperationToMultiplyByK();
  chromaticCorrection->SetConstantK(chromatic_abberation);
  addDifferenceOrigFrameFilter->SetWeight(0,.5);
  addDifferenceOrigFrameFilter->SetWeight(1,.5);
  IntensityNormalisation->SetOperationToMultiplyByK();
  IntensityNormalisation->SetConstantK(2);
// Setup done ----

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
      // Setup
        // Because ShallowCopy()s don't seem to clear old data
      differencePyramid = new vtkImagePyramid(NumberOfPyramidLevels);
      activeOutputFrame = vtkSmartPointer<vtkImageData>::New();
      // Setup done

      // Extract color channel
      colorChannelImage = extractColorChannel(yiqFilter->GetOutput(), color_channel);

      //Create image pyramid
      Pyramid = new vtkImagePyramid(colorChannelImage, NumberOfPyramidLevels);

//      -------------Initialising lowPass with first frame--------------------------
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
        if (color_channel != YChannel)
        {
          chromaticCorrection->SetInput1Data(differenceFrame);
          chromaticCorrection->Update();
          differenceFrame->ShallowCopy(chromaticCorrection->GetOutput());
        }

        //--------Add back frame difference to the original frame that we have read------
        // Note that we might have to multiply the intensity with a factor of 2 later(Intensity Normalisation)

        addDifferenceOrigFrameFilter->SetInputData(colorChannelImage);
        addDifferenceOrigFrameFilter->AddInputData(differenceFrame);
        addDifferenceOrigFrameFilter->Update();
        activeOutputFrame->ShallowCopy(addDifferenceOrigFrameFilter->GetOutput());
        IntensityNormalisation->SetInput1Data(activeOutputFrame);
        IntensityNormalisation->Update();

        // Because ShallowCopy()s don't seem to clear old data
        outputFrame[color_channel] = vtkSmartPointer<vtkImageData>::New();
        outputFrame[color_channel]->ShallowCopy(IntensityNormalisation->GetOutput());
      }

      delete differencePyramid;
    }

    // ---------------------Combine color channels in 1 image------------------------
    if(ImageNumber!=0)
    {
      appendFilter->SetInputData(outputFrame[YChannel]);
      appendFilter->AddInputData(outputFrame[IChannel]);
      appendFilter->AddInputData(outputFrame[QChannel]);
      appendFilter->Update();
      // -------------------------------------------------------------------------------

      // ---------------------Convert the YIQ frame to the RGB frame--------------------
      rgbConversionFilter->SetInputConnection(appendFilter->GetOutputPort());
      rgbConversionFilter->Update();
      // -------------------------------------------------------------------------------

//      TODO - Make a function to write frames
      std::string iterationNumberString = to_string(ImageNumber);
      std::string outputFileName = "OutputFrame" + iterationNumberString+".png";
      vtkSmartPointer<vtkPNGWriter> writeDifferenceFrames = vtkSmartPointer<vtkPNGWriter>::New();
      writeDifferenceFrames->SetFileName(outputFileName.c_str());
      writeDifferenceFrames->SetInputData(rgbConversionFilter->GetOutput());
      writeDifferenceFrames->Write();
    }
  }
  return EXIT_SUCCESS;
}
