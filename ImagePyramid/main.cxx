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
  vtkSmartPointer<vtkImagePyramid> YPyramid;
  vtkSmartPointer<vtkImagePyramid> IPyramid;
  vtkSmartPointer<vtkImagePyramid> QPyramid;

  vtkSmartPointer<vtkImagePyramid> lowPass1Y;
  vtkSmartPointer<vtkImagePyramid> lowPass2Y;
  vtkSmartPointer<vtkImagePyramid> lowPass1I;
  vtkSmartPointer<vtkImagePyramid> lowPass2I;
  vtkSmartPointer<vtkImagePyramid> lowPass1Q;
  vtkSmartPointer<vtkImagePyramid> lowPass2Q;

  vtkSmartPointer<vtkImagePyramid> YDifference;
  vtkSmartPointer<vtkImagePyramid> IDifference;
  vtkSmartPointer<vtkImagePyramid> QDifference;

  vtkSmartPointer<vtkImageData> FrameDifferenceY = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> FrameDifferenceI = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> FrameDifferenceQ = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> OutputFrameY = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> OutputFrameI = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> OutputFrameQ = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageExtractComponents> imageSource; // Moved outside since we need it to add back the differences
  vtkSmartPointer<vtkImageYIQToRGB> rgbConversionFilter =
    vtkSmartPointer<vtkImageYIQToRGB>::New(); // Made a global variable so that can be accessed by mapper



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
    int* imageDimensionArray = imageReader->GetOutput()->GetExtent();
    int imageDimension1 = imageDimensionArray[1] - imageDimensionArray[0] + 1;
    int imageDimension2 = imageDimensionArray[3] - imageDimensionArray[2] + 1;

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

    // ------------------Create image pyramids------------------------------
    for (int color_channel=0; color_channel<3; color_channel++){
      switch (color_channel){
        case YChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              YPyramid = new vtkImagePyramid(extractYFilter->GetOutput(), NumberOfPyramidLevels);
            }
            break;
          }
        case IChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              IPyramid = new vtkImagePyramid(extractIFilter->GetOutput(), NumberOfPyramidLevels);
            }
            break;
          }
        case QChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              QPyramid = new vtkImagePyramid(extractQFilter->GetOutput(), NumberOfPyramidLevels);
            }
            break;
          }
      }
    //-------------------------------------------------------------------------

      // -------------initialising lowPass with first frame--------------------------
      // Initialising the Previous frame pyramid for frame 1. We initialise it to the
      // first frame(which means first value of frame difference is going to be zero)
      if (ImageNumber==0)
      {
        switch (color_channel)
        {
          case YChannel:
          {
            DEBUG;
            lowPass1Y = new vtkImagePyramid();
            lowPass2Y = new vtkImagePyramid();
            lowPass1Y->ShallowCopy(YPyramid);
            lowPass2Y->ShallowCopy(YPyramid);
            break;
          }
          case IChannel:
          {
            DEBUG;
            lowPass1I = new vtkImagePyramid();
            lowPass2I = new vtkImagePyramid();
            lowPass1I->ShallowCopy(IPyramid);
            lowPass2I->ShallowCopy(IPyramid);
            break;
          }
          case QChannel:
          {
            DEBUG;
            lowPass1Q = new vtkImagePyramid();
            lowPass2Q = new vtkImagePyramid();
            lowPass1Q->ShallowCopy(QPyramid);
            lowPass2Q->ShallowCopy(QPyramid);
            break;
          }
        }
      }
      else
      {
        //---------Temporal IIR(Infinite impulse response) filtering-----------
        // -------Updating lowpass variable------
        vtkSmartPointer<vtkImageWeightedSum> sumFilter1 = vtkSmartPointer<vtkImageWeightedSum>::New();
        vtkSmartPointer<vtkImageWeightedSum> sumFilter2 = vtkSmartPointer<vtkImageWeightedSum>::New();
        sumFilter1->SetWeight(0,r1);
        sumFilter1->SetWeight(1,(1-r1));
        sumFilter2->SetWeight(0,r2);
        sumFilter2->SetWeight(1,(1-r2));
        switch (color_channel)
        {
          case YChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(YPyramid->vtkImagePyramidData[k]);
              sumFilter1->AddInputData(lowPass1Y->vtkImagePyramidData[k]);
              sumFilter1->Update();
              lowPass1Y->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(YPyramid->vtkImagePyramidData[k]);
              sumFilter2->AddInputData(lowPass2Y->vtkImagePyramidData[k]);
              sumFilter2->Update();
              lowPass2Y->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());
            }
            break;
          }
          case IChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(IPyramid->vtkImagePyramidData[k]);
              sumFilter1->AddInputData(lowPass1I->vtkImagePyramidData[k]);
              sumFilter1->Update();
              lowPass1I->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(IPyramid->vtkImagePyramidData[k]);
              sumFilter2->AddInputData(lowPass2I->vtkImagePyramidData[k]);
              sumFilter2->Update();
              lowPass2I->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());
            }
            break;
          }
          case QChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(QPyramid->vtkImagePyramidData[k]);
              sumFilter1->AddInputData(lowPass1Q->vtkImagePyramidData[k]);
              sumFilter1->Update();
              lowPass1Q->vtkImagePyramidData[k]->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(QPyramid->vtkImagePyramidData[k]);
              sumFilter2->AddInputData(lowPass2Q->vtkImagePyramidData[k]);
              sumFilter2->Update();
              lowPass2Q->vtkImagePyramidData[k]->ShallowCopy(sumFilter2->GetOutput());
            }
            break;
          }
        }
        //------Updated lowpass variable-------

        //----Image Pyramid difference for IIR filtering-----
        vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();
        differenceFilter->AllowShiftOff();
        differenceFilter->AveragingOff();
        differenceFilter->SetAllowShift(0);
        differenceFilter->SetThreshold(0);
        vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
        YDifference = new vtkImagePyramid(NumberOfPyramidLevels);
        IDifference = new vtkImagePyramid(NumberOfPyramidLevels);
        QDifference = new vtkImagePyramid(NumberOfPyramidLevels);
        imageMath->SetOperationToSubtract();
        switch(color_channel)
        {
          case YChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1Y->vtkImagePyramidData[k]);
              imageMath->SetInput2Data(lowPass2Y->vtkImagePyramidData[k]);
              imageMath->Update();
              YDifference->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
          case IChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1I->vtkImagePyramidData[k]);
              imageMath->SetInput2Data(lowPass2I->vtkImagePyramidData[k]);
              imageMath->Update();
              IDifference->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
          case QChannel:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1Q->vtkImagePyramidData[k]);
              imageMath->SetInput2Data(lowPass2Q->vtkImagePyramidData[k]);
              imageMath->Update();
              QDifference->vtkImagePyramidData[k]->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
        }
        // ------End of pyramid difference------
      // ------------------End of temporal filtering---------------------------

      // ----------------Get image dimensions for spatial filtering------------
        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
          int* a = lowPass1Q->vtkImagePyramidData[k]->GetExtent();
          int imageDimension1 = a[1] - a[0] + 1;
          int imageDimension2 = a[3] - a[2] + 1;
          frameSize[k] = pow((pow(imageDimension1,2) + pow(imageDimension2,2)), 0.5)/3;
          // 3 is an experimental constant used by the authors of the paper
        }
      // ----------------------------------------------------------------------

        vtkSmartPointer<vtkImageMathematics> differenceBooster = vtkSmartPointer<vtkImageMathematics>::New();
        differenceBooster->SetOperationToMultiplyByK();

        switch(color_channel)
        {
          case YChannel:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              int currAlpha = frameSize[k]/(delta*8) - 1;
              currAlpha = currAlpha * exaggeration_factor;
              int mutiplier = currAlpha;
              if (mutiplier > alpha)
              {
                mutiplier = alpha;
              }
              // ----------Verify that multiplier is a float(or acceptable data format for SetConstantK)--------
              differenceBooster->SetConstantK(mutiplier);
              differenceBooster->SetInput1Data(YDifference->vtkImagePyramidData[k]);
              differenceBooster->Update();
              YDifference->vtkImagePyramidData[k]->ShallowCopy(differenceBooster->GetOutput());
            }
            break;
          }
          case IChannel:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              int currAlpha = frameSize[k]/(delta*8) - 1;
              currAlpha = currAlpha * exaggeration_factor;
              int mutiplier = currAlpha;
              if (mutiplier > alpha)
              {
                mutiplier = alpha;
              }
              differenceBooster->SetConstantK(mutiplier);
              differenceBooster->SetInput1Data(IDifference->vtkImagePyramidData[k]);
              differenceBooster->Update();
              IDifference->vtkImagePyramidData[k]->ShallowCopy(differenceBooster->GetOutput());
            }
            break;
          }
          case QChannel:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              int currAlpha = frameSize[k]/(delta*8) - 1;
              currAlpha = currAlpha * exaggeration_factor;
              int mutiplier = currAlpha;
              if (mutiplier > alpha)
              {
                mutiplier = alpha;
              }
              differenceBooster->SetConstantK(mutiplier);
              differenceBooster->SetInput1Data(QDifference->vtkImagePyramidData[k]);
              differenceBooster->Update();
              QDifference->vtkImagePyramidData[k]->ShallowCopy(differenceBooster->GetOutput());
            }
            break;
          }
        }
        // -------------End of spatial filtering----------------------

        //-----------Collapse the image Pyramid------------------------
        vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter;
        vtkSmartPointer<vtkImageResize> resize;
        resize = vtkSmartPointer<vtkImageResize>::New();
        gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
        vtkSmartPointer<vtkImageWeightedSum> sumFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
        sumFilter->SetWeight(0,0.5);
        sumFilter->SetWeight(1,0.5);
        resize->SetResizeMethodToOutputDimensions();
        for (int g = (NumberOfPyramidLevels-1); g>=1; g--)
        {
          if (g == (NumberOfPyramidLevels-1))
          {
            switch (color_channel) {
              case YChannel:
              {
                resize->SetInputData(YDifference->vtkImagePyramidData[g]);
                break;
              }
              case IChannel:
              {
                resize->SetInputData(IDifference->vtkImagePyramidData[g]);
                break;
              }
              case QChannel:
              {
                resize->SetInputData(QDifference->vtkImagePyramidData[g]);
                break;
              }
            }
          }
          else
          {
            resize->SetInputData(sumFilter->GetOutput());
          }

          resize->SetOutputDimensions(imageDimension1/pow(2,(g-1)), imageDimension2/pow(2,(g-1)), -1);
          resize->Update();
          sumFilter = NULL;
          sumFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
          sumFilter->SetWeight(0,0.5);
          sumFilter->SetWeight(1,0.5);
          vtkSmartPointer<vtkImageConvolve> convolveFilter2 =
            vtkSmartPointer<vtkImageConvolve>::New();
          convolveFilter2->SetInputData(resize->GetOutput());
          double kernel[25] = {1,4,6,4,1,4,16,24,16,4,6,24,36,24,6,4,16,24,16,4,1,4,6,4,1};
          for (int internal_loop=0; internal_loop<25;internal_loop++)
          {
            kernel[internal_loop] = kernel[internal_loop]/256;
          }
          convolveFilter2->SetKernel5x5(kernel);
          convolveFilter2->Update();
          sumFilter->SetInputData(convolveFilter2->GetOutput());
          switch (color_channel) {
            case YChannel:
            {
              sumFilter->AddInputData(YDifference->vtkImagePyramidData[g-1]);
              break;
            }
            case IChannel:
            {
              sumFilter->AddInputData(IDifference->vtkImagePyramidData[g-1]);
              break;
            }
            case QChannel:
            {
              sumFilter->AddInputData(QDifference->vtkImagePyramidData[g-1]);
              break;
            }
          }
          sumFilter->Update();

          // --------------Save the final image in corresponding difference variable---------
          if (g==1)
          {
            switch (color_channel)
            {
              case YChannel:
              {
                FrameDifferenceY->ShallowCopy(sumFilter->GetOutput());
              }
              case IChannel:
              {
                FrameDifferenceI->ShallowCopy(sumFilter->GetOutput());
                break;
              }
              case QChannel:
              {
                FrameDifferenceQ->ShallowCopy(sumFilter->GetOutput());
                break;
              }
            }
          }
        }
        //---------------Pyramid Collapsed into image---------------------------

        //----------Chromatic Abberation to reduce noise---------------
        vtkSmartPointer<vtkImageMathematics> chromaticCorrection =
          vtkSmartPointer<vtkImageMathematics>::New();
        chromaticCorrection->SetOperationToMultiplyByK();
        chromaticCorrection->SetConstantK(chromatic_abberation);
        switch(color_channel)
        {
          // Do nothing for Y channel
          case IChannel:
          {
            chromaticCorrection->SetInput1Data(FrameDifferenceI);
            chromaticCorrection->Update();
            FrameDifferenceI->ShallowCopy(chromaticCorrection->GetOutput());
            break;
          }
          case QChannel:
          {
            chromaticCorrection->SetInput1Data(FrameDifferenceQ);
            chromaticCorrection->Update();
            FrameDifferenceQ->ShallowCopy(chromaticCorrection->GetOutput());
          }
        }

        //--------Add back frame difference to the original frame that we have read------
        vtkSmartPointer<vtkImageWeightedSum> addDifferenceOrigFrameFilter =
        vtkSmartPointer<vtkImageWeightedSum>::New();
        // Note that we might have to multiply the intensity with a factor of 2 later...
        addDifferenceOrigFrameFilter->SetWeight(0,.5);
        addDifferenceOrigFrameFilter->SetWeight(1,.5);
        vtkSmartPointer<vtkImageMathematics> IntensityNormalisation = vtkSmartPointer<vtkImageMathematics>::New();
        IntensityNormalisation->SetOperationToMultiplyByK();
        IntensityNormalisation->SetConstantK(2);
        switch (color_channel) {
          case YChannel:
          {
            addDifferenceOrigFrameFilter->SetInputData(extractYFilter->GetOutput());
            addDifferenceOrigFrameFilter->AddInputData(FrameDifferenceY);
            addDifferenceOrigFrameFilter->Update();
            OutputFrameY->ShallowCopy(addDifferenceOrigFrameFilter->GetOutput());
            IntensityNormalisation->SetInput1Data(OutputFrameY);
            IntensityNormalisation->Update();
            OutputFrameY->ShallowCopy(IntensityNormalisation->GetOutput());
            break;
          }
          case IChannel:
          {
            addDifferenceOrigFrameFilter->SetInputData(extractIFilter->GetOutput());
            addDifferenceOrigFrameFilter->AddInputData(FrameDifferenceI);
            addDifferenceOrigFrameFilter->Update();
            OutputFrameI->ShallowCopy(addDifferenceOrigFrameFilter->GetOutput());
            IntensityNormalisation->SetInput1Data(OutputFrameI);
            IntensityNormalisation->Update();
            OutputFrameI->ShallowCopy(IntensityNormalisation->GetOutput());
            break;
          }
          case QChannel:
          {
            addDifferenceOrigFrameFilter->SetInputData(extractQFilter->GetOutput());
            addDifferenceOrigFrameFilter->AddInputData(FrameDifferenceQ);
            addDifferenceOrigFrameFilter->Update();
            OutputFrameQ->ShallowCopy(addDifferenceOrigFrameFilter->GetOutput());
            IntensityNormalisation->SetInput1Data(OutputFrameQ);
            IntensityNormalisation->Update();
            OutputFrameQ->ShallowCopy(IntensityNormalisation->GetOutput());
            break;
          }
        }
        //---------------------------------------------------------------------

      }   // End of the else loop(to perform operations only for frame numbers greater than 1)
    } //End of iteration over the 3 color channels

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

      std::string iterationNumberString = to_string(ImageNumber);
      std::string outputFileName = "OutputFrame" + iterationNumberString+".png";
      vtkSmartPointer<vtkPNGWriter> writeDifferenceFrames = vtkSmartPointer<vtkPNGWriter>::New();
      writeDifferenceFrames->SetFileName(outputFileName.c_str());
      writeDifferenceFrames->SetInputData(rgbConversionFilter->GetOutput());
      writeDifferenceFrames->Write();
    }

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

  } //End of iteration over all input frames of the video(input images)
  return EXIT_SUCCESS;
}
