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

#include <vtkImageExtractComponents.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMathematics.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
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

  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  vtkSmartPointer<vtkImageReader2Factory> readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
  vtkSmartPointer<vtkImageReader2> imageReader;

  vtkSmartPointer<vtkImageRGBToYIQ> yiqFilter = vtkSmartPointer<vtkImageRGBToYIQ>::New();
  vtkSmartPointer<vtkImageExtractComponents> extractFilter = vtkSmartPointer<vtkImageExtractComponents>::New();
  vtkImagePyramid *Pyramid;

  vtkImagePyramid *lowPass1[3];
  vtkImagePyramid *lowPass2[3];

  int frameSize[NumberOfPyramidLevels];

  vtkSmartPointer<vtkImageWeightedSum> sumFilter1 = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageWeightedSum> sumFilter2 = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();
  vtkSmartPointer<vtkImageMathematics> differenceBooster = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageMathematics> chromaticCorrection = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageWeightedSum> addDifferenceOrigFrameFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
  vtkSmartPointer<vtkImageMathematics> IntensityNormalisation = vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageAppendComponents> appendFilter = vtkSmartPointer<vtkImageAppendComponents>::New();

  vtkImagePyramid *differencePyramid;

  vtkSmartPointer<vtkImageData> differenceFrame;
  vtkSmartPointer<vtkImageData> outputFrame[3];
  vtkSmartPointer<vtkImageData> activeOutputFrame;
  vtkSmartPointer<vtkImageYIQToRGB> rgbConversionFilter = vtkSmartPointer<vtkImageYIQToRGB>::New();
  vtkSmartPointer<vtkImageData> activeImage;

// Setup -----
  for(int i = 0; i < 3; i++)
  {
    lowPass1[i] = NULL;
    lowPass2[i] = NULL;
  }

  sumFilter1->SetWeight(0,r1);
  sumFilter1->SetWeight(1,(1-r1));
  sumFilter2->SetWeight(0,r2);
  sumFilter2->SetWeight(1,(1-r2));

  //----Image Pyramid difference for IIR filtering-----
  differenceFilter->AllowShiftOff();
  differenceFilter->AveragingOff();
  differenceFilter->SetAllowShift(0);
  differenceFilter->SetThreshold(0);
  imageMath->SetOperationToSubtract();

  differenceBooster->SetOperationToMultiplyByK();
  chromaticCorrection->SetOperationToMultiplyByK();
  chromaticCorrection->SetConstantK(chromatic_abberation);
  addDifferenceOrigFrameFilter->SetWeight(0,.5);
  addDifferenceOrigFrameFilter->SetWeight(1,.5);
  IntensityNormalisation->SetOperationToMultiplyByK();
  IntensityNormalisation->SetConstantK(2);

// Setup done ----

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

  bool show = false;
  if (argc == 3 && !strcmp(argv[2], "show"))
  {
    show = true;
  }


  glob->AddFileNames("*");
  int frameCount = glob->GetNumberOfFileNames();
  cout << "Number of Images read " << frameCount << "\n";

  for (int ImageNumber = 0; ImageNumber < frameCount; ImageNumber++)
    {
    std::string inputFilename = glob->GetNthFileName(ImageNumber);
    cout << inputFilename << "\n";

      // Read the file
    imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
    imageReader->SetFileName(inputFilename.c_str());
    imageReader->Update();

    yiqFilter->SetInputConnection(imageReader->GetOutputPort());
    yiqFilter->Update();

    for (int color_channel=0; color_channel<3; color_channel++){
      // Setup
        // Because ShallowCopy()s don't seem to clear old data
      differencePyramid = new vtkImagePyramid(NumberOfPyramidLevels);
      activeOutputFrame = vtkSmartPointer<vtkImageData>::New();
      // Setup done

      // Extract color channel
      extractFilter->SetInputConnection(yiqFilter->GetOutputPort());
      extractFilter->SetComponents(color_channel);
      extractFilter->Update();
      activeImage = extractFilter->GetOutput();

      //Create image pyramid
      Pyramid = new vtkImagePyramid(activeImage, NumberOfPyramidLevels);

//      -------------Initialising lowPass with first frame--------------------------
//      Initialising the Previous frame pyramid for frame 1. We initialise it to the
//      first frame(which means first value of frame difference is going to be zero)
      if (ImageNumber==0)
      {
        lowPass1[color_channel] = new vtkImagePyramid();
        lowPass2[color_channel] = new vtkImagePyramid();
        lowPass1[color_channel]->ShallowCopy(Pyramid);
        lowPass2[color_channel]->ShallowCopy(Pyramid);

        // Inserting the code below since it only needs to be done once(hence only for frame 1)
        if (color_channel == 0)
        {
          // ----------------Get image dimensions for spatial filtering------------
          for (int k = 0; k < NumberOfPyramidLevels; k++) {
            frameSize[k] = getImageDimensions(Pyramid->vtkImagePyramidData[k]);
          }
        }
      }
      else
      {
//        Everything below is done for later frames, processing for first frame ends above

        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
//        Temporal IIR(Infinite impulse response filtering)
          // -------Updating lowpass variable------
          sumFilter1->SetInputData(Pyramid->vtkImagePyramidData[k]);
          sumFilter1->AddInputData(lowPass1[color_channel]->vtkImagePyramidData[k]);
          sumFilter1->Update();
          lowPass1[color_channel]->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
          sumFilter2->SetInputData(Pyramid->vtkImagePyramidData[k]);
          sumFilter2->AddInputData(lowPass2[color_channel]->vtkImagePyramidData[k]);
          sumFilter2->Update();
          lowPass2[color_channel]->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());

          imageMath->SetInput1Data(lowPass1[color_channel]->vtkImagePyramidData[k]);
          imageMath->SetInput2Data(lowPass2[color_channel]->vtkImagePyramidData[k]);
          imageMath->Update();
          differencePyramid->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
        }
// ------------------End of temporal filtering---------------------------

//        Spatial filtering
        for (int k=1; k<NumberOfPyramidLevels-1; k++) // Is separate loop than temporal filtering, because limits
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

        differenceFrame = differencePyramid->Collapse();


        //----------Chromatic Aberration to reduce noise---------------
        if (color_channel != YChannel)
        {
          chromaticCorrection->SetInput1Data(differenceFrame);
          chromaticCorrection->Update();
          differenceFrame->ShallowCopy(chromaticCorrection->GetOutput());
        }

        //--------Add back frame difference to the original frame that we have read------
        // Note that we might have to multiply the intensity with a factor of 2 later(Intensity Normalisation)

        addDifferenceOrigFrameFilter->SetInputData(activeImage);
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

//      TODO - Make a function to show(uncomment the code basically and put it in a function)
    if (show)
      {
      cerr << "TODO show" << endl;

      //or maybe use vtkFFMPEG to encode into a new video.
      //I think it is fine to dump into files and let user decide

      // --------Use the code below to visualise the frames instead of writing the frames to disk---------
      // vtkSmartPointer<vtkDataSetMapper> mapper =
      //   vtkSmartPointer<vtkDataSetMapper>::New();
      //
      // // mapper->SetInputConnection(resize->GetOutputPort());
      //
      // //-------------------Suspected code snippet causing segmentation error----------------------------
      // // Why does mapper->SetInputData(ImagePyramidGeneral[0].imagedata);  throw up an error?
      // // mapper->SetInputData(YDifference[0].imagedata);
      // mapper->SetInputData(rgbConversionFilter->GetOutput());
      //   // mapper->SetInputData(differenceFilter->GetOutput);
      // // mapper->SetInputData(resize->GetOutput());
      // //--------------------------------------------------------------------------------------------------
      //
      // vtkSmartPointer<vtkActor> actor =
      //   vtkSmartPointer<vtkActor>::New();
      // actor->SetMapper(mapper);
      // actor->GetProperty()->SetRepresentationToWireframe();
      //
      // vtkSmartPointer<vtkRenderer> renderer =
      //   vtkSmartPointer<vtkRenderer>::New();
      // renderer->AddActor(actor);
      // renderer->ResetCamera();
      // renderer->SetBackground(1,1,1);
      //
      // vtkSmartPointer<vtkRenderWindow> renderWindow =
      //   vtkSmartPointer<vtkRenderWindow>::New();
      // renderWindow->AddRenderer(renderer);
      //
      // vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
      //   vtkSmartPointer<vtkRenderWindowInteractor>::New();
      // renderWindowInteractor->SetRenderWindow(renderWindow);
      // renderWindowInteractor->Initialize();
      //
      // renderWindowInteractor->Start();
      }
  }
  return EXIT_SUCCESS;
}
