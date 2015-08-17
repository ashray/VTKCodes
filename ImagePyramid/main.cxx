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
#include <vtkImageConvolve.h>

#include <vtkImageData.h>
#include <vtkImageDifference.h>

#include <vtkImageExtractComponents.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMathematics.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageResize.h>
#include <vtkImageRGBToYIQ.h>
#include <vtkImageShiftScale.h>
#include <vtkImageSincInterpolator.h>
#include <vtkImageViewer2.h>
#include <vtkImageWeightedSum.h>
#include <vtkImageYIQToRGB.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPNGWriter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

#include <vtksys/SystemTools.hxx>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>

#include "conveniences.h"
#include "constants.h"

int main(int argc, char* argv[])
{

  std::string dirString = "/Users/ashraymalhotra/Desktop/Academic/VTKDev/Data/AllImages/";
  if (argc < 2)
    {
    cerr << "WARNING: Expected arguments" << argv[0] << "/directory/path/to/numbered/image/files/" << endl;
    cerr << "defaulting to " << dirString << endl;
    }
  else
    {
    dirString = argv[1];
    }

  bool show = false;
  if (argc == 3 && !strcmp(argv[2], "show"))
    {
    show = true;
    }

  // Have to define these variables as global since used in mapper, can be optimised better
  vtkSmartPointer<vtkImagePyramid> activePyramid;

  vtkSmartPointer<vtkImagePyramid> lowPass1Y;
  vtkSmartPointer<vtkImagePyramid> lowPass2Y;
  vtkSmartPointer<vtkImagePyramid> lowPass1I;
  vtkSmartPointer<vtkImagePyramid> lowPass2I;
  vtkSmartPointer<vtkImagePyramid> lowPass1Q;
  vtkSmartPointer<vtkImagePyramid> lowPass2Q;
  vtkSmartPointer<vtkImagePyramid> activeLowPass1;
  vtkSmartPointer<vtkImagePyramid> activeLowPass2;

  vtkSmartPointer<vtkImagePyramid> YDifference;
  vtkSmartPointer<vtkImagePyramid> IDifference;
  vtkSmartPointer<vtkImagePyramid> QDifference;
  vtkSmartPointer<vtkImagePyramid> activeDifferencePyramid;

  vtkSmartPointer<vtkImageData> activeDifferenceFrame;
  vtkSmartPointer<vtkImageData> OutputFrameY = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> activeOutputFrame = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> OutputFrameI = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> OutputFrameQ = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageYIQToRGB> rgbConversionFilter =
    vtkSmartPointer<vtkImageYIQToRGB>::New(); // Made a global variable so that can be accessed by mapper
  vtkSmartPointer<vtkImageData> activeImage;


  int frameSize[NumberOfPyramidLevels];

  // Create reader to read all images
  vtksys::SystemTools::ConvertToUnixSlashes(dirString);
  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  glob->SetDirectory(dirString.c_str());
  glob->AddFileNames("*");
  int frameCount = glob->GetNumberOfFileNames();
  cout << "Number of Images read " << frameCount << "\n";

  for (int ImageNumber = 0; ImageNumber < frameCount; ImageNumber++)
    {
    std::string inputFilename = glob->GetNthFileName(ImageNumber);
    cout << inputFilename << "\n";

    // Read the file
    vtkSmartPointer<vtkImageReader2Factory> readerFactory =
      vtkSmartPointer<vtkImageReader2Factory>::New();
    vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
    imageReader->SetFileName(inputFilename.c_str());
    imageReader->Update();

    // ---------------------Get YIQ components-----------------------------
    vtkSmartPointer<vtkImageRGBToYIQ> yiqFilter =
      vtkSmartPointer<vtkImageRGBToYIQ>::New();
    yiqFilter->SetInputConnection(imageReader->GetOutputPort());
    yiqFilter->Update();

    vtkSmartPointer<vtkImageExtractComponents> extractYFilter =
      vtkSmartPointer<vtkImageExtractComponents>::New();
    extractYFilter->SetInputConnection(yiqFilter->GetOutputPort());
    extractYFilter->SetComponents(0);
    extractYFilter->Update();

    vtkSmartPointer<vtkImageExtractComponents> extractIFilter =
      vtkSmartPointer<vtkImageExtractComponents>::New();
    extractIFilter->SetInputConnection(yiqFilter->GetOutputPort());
    extractIFilter->SetComponents(1);
    extractIFilter->Update();

    vtkSmartPointer<vtkImageExtractComponents> extractQFilter =
      vtkSmartPointer<vtkImageExtractComponents>::New();
    extractQFilter->SetInputConnection(yiqFilter->GetOutputPort());
    extractQFilter->SetComponents(2);
    extractQFilter->Update();
    //-------------------------------------------------------------------------

      YDifference = new vtkImagePyramid(NumberOfPyramidLevels);
      IDifference = new vtkImagePyramid(NumberOfPyramidLevels);
      QDifference = new vtkImagePyramid(NumberOfPyramidLevels);
      activeDifferencePyramid = new vtkImagePyramid(NumberOfPyramidLevels);


    for (int color_channel=0; color_channel<3; color_channel++){

//      Select the correct image to use
      switch (color_channel){
        case YChannel:
        {
          activeImage = extractYFilter->GetOutput();
          activeLowPass1 = lowPass1Y;
          activeLowPass2 = lowPass2Y;
          activeDifferencePyramid = YDifference;
          break;
        }
        case IChannel:
        {
          activeImage = extractIFilter->GetOutput();
          activeLowPass1 = lowPass1I;
          activeLowPass2 = lowPass2I;
          activeDifferencePyramid = IDifference;
          break;
        }
        case QChannel:
        {
          activeImage = extractQFilter->GetOutput();
          activeLowPass1 = lowPass1Q;
          activeLowPass2 = lowPass2Q;
          activeDifferencePyramid = QDifference;
          break;
        }
      }

//      Create image pyramid
      activePyramid = new vtkImagePyramid(activeImage, NumberOfPyramidLevels);

//      -------------Initialising lowPass with first frame--------------------------
//      Initialising the Previous frame pyramid for frame 1. We initialise it to the
//      first frame(which means first value of frame difference is going to be zero)
      if (ImageNumber==0)
      {
        activeLowPass1 = new vtkImagePyramid();
        activeLowPass2 = new vtkImagePyramid();
        activeLowPass1->ShallowCopy(activePyramid);
        activeLowPass2->ShallowCopy(activePyramid);

        switch (color_channel){
          case YChannel:
          {
            lowPass1Y = activeLowPass1;
            lowPass2Y = activeLowPass2;
            break;
          }
          case IChannel:
          {
            lowPass1I = activeLowPass1;
            lowPass2I = activeLowPass2;
            break;
          }
          case QChannel:
          {
            lowPass1Q = activeLowPass1;
            lowPass2Q = activeLowPass2;
            break;
          }
        }

//       Inserting the code below since it only needs to be done once(hence only for frame 1)
//        TODO - Make the code below into a difference function (getImageDimensions filter maybe?
        // ----------------Get image dimensions for spatial filtering------------
        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
          int* a = activeLowPass1->vtkImagePyramidData[k]->GetExtent();
          int imageDimension1 = a[1] - a[0] + 1;
          int imageDimension2 = a[3] - a[2] + 1;
          frameSize[k] = int(pow((pow(imageDimension1,2) + pow(imageDimension2,2)), 0.5)/3);
          // 3 is an experimental constant used by the authors of the paper
        }
      }
      else
      {
//        Everything below is done for later frames, processing for first frame ends above

//        Temporal IIR(Infinite impulse response filtering)
        // -------Updating lowpass variable------
        vtkSmartPointer<vtkImageWeightedSum> sumFilter1 = vtkSmartPointer<vtkImageWeightedSum>::New();
        vtkSmartPointer<vtkImageWeightedSum> sumFilter2 = vtkSmartPointer<vtkImageWeightedSum>::New();
        sumFilter1->SetWeight(0,r1);
        sumFilter1->SetWeight(1,(1-r1));
        sumFilter2->SetWeight(0,r2);
        sumFilter2->SetWeight(1,(1-r2));
        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
          sumFilter1->SetInputData(activePyramid->vtkImagePyramidData[k]);
          sumFilter1->AddInputData(activeLowPass1->vtkImagePyramidData[k]);
          sumFilter1->Update();
          activeLowPass1->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
          sumFilter2->SetInputData(activePyramid->vtkImagePyramidData[k]);
          sumFilter2->AddInputData(activeLowPass2->vtkImagePyramidData[k]);
          sumFilter2->Update();
          activeLowPass2->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());
        }

        //----Image Pyramid difference for IIR filtering-----
        vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();
        differenceFilter->AllowShiftOff();
        differenceFilter->AveragingOff();
        differenceFilter->SetAllowShift(0);
        differenceFilter->SetThreshold(0);
        vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
        imageMath->SetOperationToSubtract();

        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
          imageMath->SetInput1Data(activeLowPass1->vtkImagePyramidData[k]);
          imageMath->SetInput2Data(activeLowPass2->vtkImagePyramidData[k]);
          imageMath->Update();
          activeDifferencePyramid->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
        }
      // ------------------End of temporal filtering---------------------------

      // ----------------------------------------------------------------------

        vtkSmartPointer<vtkImageMathematics> differenceBooster = vtkSmartPointer<vtkImageMathematics>::New();
        differenceBooster->SetOperationToMultiplyByK();

//       TODO -----------!!!!!!!!!!! Continue from here----------

//       Spatial filtering
        for (int k=1; k<NumberOfPyramidLevels-1; k++)
        {
          float currAlpha = frameSize[k]/(delta*8) - 1;
          currAlpha = currAlpha * exaggeration_factor;
          float mutiplier = currAlpha;
          if (mutiplier > alpha)
          {
            mutiplier = alpha;
          }
          differenceBooster->SetConstantK(mutiplier);
          differenceBooster->SetInput1Data(activeDifferencePyramid->vtkImagePyramidData[k]);
          differenceBooster->Update();
          activeDifferencePyramid->vtkImagePyramidData[k]->ShallowCopy(differenceBooster->GetOutput());
        }
        
        activeDifferenceFrame = activeDifferencePyramid->Collapse();


        //----------Chromatic Aberration to reduce noise---------------
        vtkSmartPointer<vtkImageMathematics> chromaticCorrection =
          vtkSmartPointer<vtkImageMathematics>::New();
        chromaticCorrection->SetOperationToMultiplyByK();
        chromaticCorrection->SetConstantK(chromatic_abberation);
        if (color_channel == IChannel || QChannel)
        {
          chromaticCorrection->SetInput1Data(activeDifferenceFrame);
          chromaticCorrection->Update();
          activeDifferenceFrame->ShallowCopy(chromaticCorrection->GetOutput());
        }

        //--------Add back frame difference to the original frame that we have read------
        vtkSmartPointer<vtkImageWeightedSum> addDifferenceOrigFrameFilter =
        vtkSmartPointer<vtkImageWeightedSum>::New();
        // Note that we might have to multiply the intensity with a factor of 2 later(Intensity Normalisation)
        addDifferenceOrigFrameFilter->SetWeight(0,.5);
        addDifferenceOrigFrameFilter->SetWeight(1,.5);
        vtkSmartPointer<vtkImageMathematics> IntensityNormalisation = vtkSmartPointer<vtkImageMathematics>::New();
        IntensityNormalisation->SetOperationToMultiplyByK();
        IntensityNormalisation->SetConstantK(2);

        addDifferenceOrigFrameFilter->SetInputData(activeImage);
        addDifferenceOrigFrameFilter->AddInputData(activeDifferenceFrame);
        addDifferenceOrigFrameFilter->Update();
        activeOutputFrame->ShallowCopy(addDifferenceOrigFrameFilter->GetOutput());
        IntensityNormalisation->SetInput1Data(activeOutputFrame);
        IntensityNormalisation->Update();

        switch (color_channel){
          case YChannel:
          {
            OutputFrameY->ShallowCopy(IntensityNormalisation->GetOutput());
            lowPass1Y = activeLowPass1;
            lowPass2Y = activeLowPass2;
            break;
          }
          case IChannel:
          {
            OutputFrameI->ShallowCopy(IntensityNormalisation->GetOutput());
            lowPass1I = activeLowPass1;
            lowPass2I = activeLowPass2;
            break;
          }
          case QChannel:
          {
            OutputFrameQ->ShallowCopy(IntensityNormalisation->GetOutput());
            lowPass1Q = activeLowPass1;
            lowPass2Q = activeLowPass2;
            break;
          }
        }
      }
    }

    // ---------------------Combine color channels in 1 image------------------------
    if(ImageNumber!=0)
    {
      vtkSmartPointer<vtkImageAppendComponents> appendFilter =
        vtkSmartPointer<vtkImageAppendComponents>::New();
      cerr << "Y dims: " << showDims(OutputFrameY) << endl
           << "I dims: " << showDims(OutputFrameI) << endl
           << "Q dims: " << showDims(OutputFrameQ) << endl;
      appendFilter->AddInputData(OutputFrameY);
      appendFilter->AddInputData(OutputFrameI);
      appendFilter->AddInputData(OutputFrameQ);
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
