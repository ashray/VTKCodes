#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkImageActor.h>
#include <vtkImageViewer2.h>
#include <vtkImageReader2Factory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageGaussianSmooth.h>

#include <vtkSmartPointer.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageExtractComponents.h>


#include <vtkImageResize.h>
#include "vtkImageSincInterpolator.h"
#include <vtkImageRGBToYIQ.h>

#include <vtkGlobFileNames.h>
#include <vtksys/SystemTools.hxx>
#include <string>
#include <cstdio>
#include <cmath>
#include "vtkStringArray.h"

#include <vtkImageShiftScale.h>
#include <vtkImageDifference.h>
#include <vtkImageMathematics.h>
#include <vtkImageWeightedSum.h>



// Originally defined as struct but with a struct the variables go out of scope and hence get deleted.
class imageStruct
{
  public:
  imageStruct() { this->imagedata = vtkSmartPointer<vtkImageData>::New(); }
  ~imageStruct() { this->imagedata = NULL; }
  vtkSmartPointer<vtkImageData> imagedata;
};

int main(int argc, char* argv[])
{
  //Have to define these variables as global since used in mapper
  int NumberOfPyramidLevels = 6;
  imageStruct *imagePyramid = new imageStruct[NumberOfPyramidLevels];
  imageStruct *YPyramid = new imageStruct[NumberOfPyramidLevels];
  imageStruct *IPyramid = new imageStruct[NumberOfPyramidLevels];
  imageStruct *QPyramid = new imageStruct[NumberOfPyramidLevels];

  imageStruct *YPyramidPreviousFrame = new imageStruct[NumberOfPyramidLevels];
  imageStruct *IPyramidPreviousFrame = new imageStruct[NumberOfPyramidLevels];
  imageStruct *QPyramidPreviousFrame = new imageStruct[NumberOfPyramidLevels];
  imageStruct *YDifference = new imageStruct[NumberOfPyramidLevels];
  imageStruct *IDifference = new imageStruct[NumberOfPyramidLevels];
  imageStruct *QDifference = new imageStruct[NumberOfPyramidLevels];

  imageStruct *lowPass1Y = new imageStruct[NumberOfPyramidLevels];
  imageStruct *lowPass2Y = new imageStruct[NumberOfPyramidLevels];
  imageStruct *lowPass1I = new imageStruct[NumberOfPyramidLevels];
  imageStruct *lowPass2I = new imageStruct[NumberOfPyramidLevels];
  imageStruct *lowPass1Q = new imageStruct[NumberOfPyramidLevels];
  imageStruct *lowPass2Q = new imageStruct[NumberOfPyramidLevels];

  int frameSize[NumberOfPyramidLevels];

  //TODO: Parse command line arguments
  std::string directoryName = "/Users/ashraymalhotra/Desktop/Academic/VTKDev/Data/ImageSequence/";

  // Create reader to read all images
  std::string dirString = directoryName;
  vtksys::SystemTools::ConvertToUnixSlashes(dirString);
  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  glob->SetDirectory(dirString.c_str());
  glob->AddFileNames("*");
  int frameCount = glob->GetNumberOfFileNames();
  cout << "Number of Images read " << frameCount << "\n";

  for (int ImageNumber = 0; ImageNumber < frameCount; ImageNumber++){
    std::string inputFilename = glob->GetNthFileName(ImageNumber);
    cout << inputFilename << "\n";

    // std::string inputFilename = "image.png";

    // Read the file
    vtkSmartPointer<vtkImageReader2Factory> readerFactory =
      vtkSmartPointer<vtkImageReader2Factory>::New();
    vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
    imageReader->SetFileName(inputFilename.c_str());
    imageReader->Update();
    // cout << __LINE__<<" Reached here \n";

    //--------------This part should go into an extract YIQ filter----------------
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
    //-----------------------------------------------------------------------------

    vtkSmartPointer<vtkImageExtractComponents> imageSource;
    int NumberOfPyramidLevels = 6;
    for (int j=0; j<3; j++){
      switch (j){
        case 0:
          imageSource = extractYFilter;
          break;
        case 1:
          imageSource = extractIFilter;
          break;
        case 2:
          imageSource = extractQFilter;
          break;
      }

      // ---------------------Image Pyramid Construction---------------------
      int* a = imageReader->GetOutput()->GetExtent();
      int imageDimension1 = a[1] - a[0] + 1;
      int imageDimension2 = a[3] - a[2] + 1;

      vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter;
      vtkSmartPointer<vtkImageResize> resize;

      imagePyramid[0].imagedata->ShallowCopy(imageSource->GetOutput());

      // if (!imagePyramid[0].imagedata) printf("0 is NULL!\n");
      for (int i=1; i<NumberOfPyramidLevels; i++){
        gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
        // gaussianSmoothFilter->SetInputData(imagePyramid[i-1].imagedata);
        gaussianSmoothFilter->SetInputData(imagePyramid[i-1].imagedata);
        gaussianSmoothFilter->Update();

        // Can set standard deviation of the gaussian filter using "SetStandardDeviation (double std)"

        resize = vtkSmartPointer<vtkImageResize>::New();
        resize->SetResizeMethodToOutputDimensions();
#if VTK_MAJOR_VERSION <= 5
        resize->SetInput(gaussianSmoothFilter->GetOutput());
#else
        resize->SetInputData(gaussianSmoothFilter->GetOutput());
#endif
        resize->SetOutputDimensions(imageDimension1/(pow(2,i)), imageDimension2/(pow(2,i)), 1);
        resize->Update();
        imagePyramid[i].imagedata->ShallowCopy(resize->GetOutput());
      }
      // ------------------Image Pyramid construction complete--------------

      // -------------Copy Image Pyramid into corresponding variable-------
      switch (j){
        case 0:
          {
            //Make the code below in a seperate function to copy image pyramids
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              YPyramid[k].imagedata->ShallowCopy(imagePyramid[k].imagedata);
            }
            break;
          }
        case 1:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              IPyramid[k].imagedata->ShallowCopy(imagePyramid[k].imagedata);
            }
            break;
          }
        case 2:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              QPyramid[k].imagedata->ShallowCopy(imagePyramid[k].imagedata);
            }
            break;
          }
      }
      // --------Copied image pyramid into corresponding variable-------------

      // // -------------Copy Image Pyramid for first frame--------------------------
      // // Initialising the Previous frame pyramid for frame 1. We initialise it to the first frame(which means first value of frame difference is going to be zero)
      // if (!ImageNumber)
      // {
      //   switch (j)
      //   {
      //     case 0:
      //     {
      //       //Make the code below in a seperate function to copy image pyramids
      //       for (int k=0; k<NumberOfPyramidLevels; k++)
      //       {
      //         YPyramidPreviousFrame[k].imagedata->ShallowCopy(YPyramid[k].imagedata);
      //       }
      //       break;
      //     }
      //     case 1:
      //     {
      //       for (int k=0; k<NumberOfPyramidLevels; k++)
      //       {
      //         IPyramidPreviousFrame[k].imagedata->ShallowCopy(IPyramid[k].imagedata);
      //       }
      //       break;
      //     }
      //     case 2:
      //     {
      //       for (int k=0; k<NumberOfPyramidLevels; k++)
      //       {
      //         QPyramidPreviousFrame[k].imagedata->ShallowCopy(QPyramid[k].imagedata);
      //       }
      //       break;
      //     }
      //   }
      // }
      // //--------------------------------------------------------------------

      int r1 = 0.4;
      int r2 = 0.05;

      // -------------initialising lowPass with first frame--------------------------
      // Initialising the Previous frame pyramid for frame 1. We initialise it to the first frame(which means first value of frame difference is going to be zero)
      if (!ImageNumber)
      {
        switch (j)
        {
          case 0:
          {
            //Make the code below in a seperate function to copy image pyramids
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              lowPass1Y[k].imagedata->ShallowCopy(YPyramid[k].imagedata);
              lowPass2Y[k].imagedata->ShallowCopy(YPyramid[k].imagedata);
            }
            break;
          }
          case 1:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              lowPass1I[k].imagedata->ShallowCopy(IPyramid[k].imagedata);
              lowPass2I[k].imagedata->ShallowCopy(IPyramid[k].imagedata);
            }
            break;
          }
          case 2:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              lowPass1Q[k].imagedata->ShallowCopy(QPyramid[k].imagedata);
              lowPass2Q[k].imagedata->ShallowCopy(QPyramid[k].imagedata);
            }
            break;
          }
        }
      } //--------------------------------------------------------------------
      else
      {
        //---------Temporal IIR(Infinite impulse response) filtering-----------
        // ----Updating lowpass variable------
        vtkSmartPointer<vtkImageWeightedSum> sumFilter1 = vtkSmartPointer<vtkImageWeightedSum>::New();
        vtkSmartPointer<vtkImageWeightedSum> sumFilter2 = vtkSmartPointer<vtkImageWeightedSum>::New();
        sumFilter1->SetWeight(0,r1);
        sumFilter1->SetWeight(1,(1-r1));
        sumFilter2->SetWeight(0,r2);
        sumFilter2->SetWeight(1,(1-r2));
        switch (j)
        {
          case 0:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(YPyramid[k].imagedata);
              sumFilter1->AddInputData(lowPass1Y[k].imagedata);
              sumFilter1->Update();
              lowPass1Y[k].imagedata->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(YPyramid[k].imagedata);
              sumFilter2->AddInputData(lowPass2Y[k].imagedata);
              sumFilter2->Update();
              lowPass2Y[k].imagedata->ShallowCopy(sumFilter1->GetOutput());
            }
            break;
          }
          case 1:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(IPyramid[k].imagedata);
              sumFilter1->AddInputData(lowPass1I[k].imagedata);
              sumFilter1->Update();
              lowPass1I[k].imagedata->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(IPyramid[k].imagedata);
              sumFilter2->AddInputData(lowPass2I[k].imagedata);
              sumFilter2->Update();
              lowPass2I[k].imagedata->ShallowCopy(sumFilter1->GetOutput());
            }
            break;
          }
          case 2:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              sumFilter1->SetInputData(QPyramid[k].imagedata);
              sumFilter1->AddInputData(lowPass1Q[k].imagedata);
              sumFilter1->Update();
              lowPass1Q[k].imagedata->ShallowCopy(sumFilter1->GetOutput());
              sumFilter2->SetInputData(QPyramid[k].imagedata);
              sumFilter2->AddInputData(lowPass2Q[k].imagedata);
              sumFilter2->Update();
              lowPass2Q[k].imagedata->ShallowCopy(sumFilter1->GetOutput());            }
            break;
          }
        }
        //-----Updated lowpass variable-------

        //----Image Pyramid difference for IIR filtering------
        vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();
        differenceFilter->AllowShiftOff();
        differenceFilter->AveragingOff();
        differenceFilter->SetAllowShift(0);
        differenceFilter->SetThreshold(0);
        // Verify that image mathematics works the way we would expect it to!!
        vtkSmartPointer<vtkImageMathematics> imageMath = vtkSmartPointer<vtkImageMathematics>::New();
        imageMath->SetOperationToSubtract();
        switch(j)
        {
          case 0:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1Y[k].imagedata);
              imageMath->SetInput2Data(lowPass2Y[k].imagedata);
              imageMath->Update();
              YDifference[k].imagedata->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
          case 1:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1I[k].imagedata);
              imageMath->SetInput2Data(lowPass2I[k].imagedata);
              imageMath->Update();
              IDifference[k].imagedata->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
          case 2:
          {
            for (int k=0; k<NumberOfPyramidLevels; k++)
            {
              imageMath->SetInput1Data(lowPass1Q[k].imagedata);
              imageMath->SetInput2Data(lowPass2Q[k].imagedata);
              imageMath->Update();
              QDifference[k].imagedata->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
        }
        // ------End of pyramid difference------
      // ------------------End of temporal filtering-------------------------------------------

      // ----------------Get image dimensions for spatial filtering----------------------
        for (int k=0; k<NumberOfPyramidLevels; k++)
        {
          // Will the below code work or not?? Is imagedata returned as a pointer? Verify.
          int* a = lowPass1Q[k].imagedata->GetExtent();
          int imageDimension1 = a[1] - a[0] + 1;
          int imageDimension2 = a[3] - a[2] + 1;
          frameSize[k] = pow((pow(imageDimension1,2) + pow(imageDimension2,2)), 0.5)/3;
          // 3 is an experimental constant used by the authors of the paper
        }
      // ---------------------------------------------------------------------------------


      // ----------------Spatial Filtering----------------------
      // Refer to the diagram included in the paper for explanation of the code below

      //!!!!!!!!!!Assign the top and bottom of the image pyramid to zero!!!!!

        alpha = 10;
        lambda_c = 16;
        delta = lambda_c/8/(1+alpha);
        //exaggeration_factor HAS to be a user defined constant, above values could be hardcoded as well
        exaggeration_factor = 20;
        vtkSmartPointer<vtkImageMathematics> differenceBooster = vtkSmartPointer<vtkImageMathematics>::New();
        differenceBooster->SetOperationToMultiplyByK();


        differenceBooster->Update();
        switch(j)
        {
          case 0:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              int currAlpha = frameSize[k]/(delta*8) - 1;
              currAlpha = currAlpha * exaggeration_factor;
              int mutiplier = currAlpha;
              if (mutiplier > alpha)
              {
                mutiplier = alpha
              }
              // ----------Verify that multiplier is a float--------
              differenceBooster->SetConstantK(mutiplier);
              // differenceBooster->SetInputConnection(imageSource->GetOutputPort());
              differenceBooster->SetInput1Data(YDifference[k].imagedata);
              differenceBooster->Update();
              YDifference[k].imagedata->ShallowCopy(differenceBooster->GetOutput())
              // YDifference[k] =
            }
            break;
          }
          case 1:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              imageMath->SetInput1Data(lowPass1I[k].imagedata);
              imageMath->SetInput2Data(lowPass2I[k].imagedata);
              imageMath->Update();
              IDifference[k].imagedata->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
          case 2:
          {
            for (int k=1; k<NumberOfPyramidLevels-1; k++)
            {
              imageMath->SetInput1Data(lowPass1Q[k].imagedata);
              imageMath->SetInput2Data(lowPass2Q[k].imagedata);
              imageMath->Update();
              QDifference[k].imagedata->ShallowCopy(imageMath->GetOutput());
            }
            break;
          }
        }



      }
      // // ----------------Copy image pyramid for all frames except first-------------------
      // // Writing this again so that it is easier later to seperate into different functions
      // switch (j)
      // {
      //   case 0:
      //   {
      //     //Make the code below in a seperate function to copy image pyramids
      //     for (int k=0; k<NumberOfPyramidLevels; k++)
      //     {
      //       // YDifference[k].imagedata->ShallowCopy(Difference ( YPyramid[k].imagedata - YPyramidPreviousFrame[k].imagedata ));
      //       YPyramidPreviousFrame[k].imagedata->ShallowCopy(YPyramid[k].imagedata);
      //     }
      //     break;
      //   }
      //   case 1:
      //   {
      //     for (int k=0; k<NumberOfPyramidLevels; k++)
      //     {
      //       IPyramidPreviousFrame[k].imagedata->ShallowCopy(imagePyramid[k].imagedata);
      //     }
      //     break;
      //   }
      //   case 2:
      //   {
      //     for (int k=0; k<NumberOfPyramidLevels; k++)
      //     {
      //       QPyramidPreviousFrame[k].imagedata->ShallowCopy(imagePyramid[k].imagedata);
      //     }
      //     break;
      //   }
      // }
      // // ---------------------------------------------------------------------------------

      //-----------Collapse the image Pyramid------------------------
      imagePyramid[0].imagedata->ShallowCopy(imageSource->GetOutput());

      for (int i=1; i<NumberOfPyramidLevels; i++){
        gaussianSmoothFilter =
          vtkSmartPointer<vtkImageGaussianSmooth>::New();
        gaussianSmoothFilter->SetInputData(imagePyramid[i-1].imagedata);
        gaussianSmoothFilter->Update();

        resize = vtkSmartPointer<vtkImageResize>::New();
        resize->SetResizeMethodToOutputDimensions();
#if VTK_MAJOR_VERSION <= 5
        resize->SetInput(gaussianSmoothFilter->GetOutput());
#else
        resize->SetInputData(gaussianSmoothFilter->GetOutput());
#endif
        resize->SetOutputDimensions(imageDimension1/(pow(2,i)), imageDimension2/(pow(2,i)), 1);
        resize->Update();
        imagePyramid[i].imagedata->ShallowCopy(resize->GetOutput());
      }

      // Verify that the algorithm to implement the laplacian pyramid is correct
      resize = vtkSmartPointer<vtkImageResize>::New();
      gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
      vtkSmartPointer<vtkImageWeightedSum> sumFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
      sumFilter->SetWeight(0,0.5);
      sumFilter->SetWeight(1,0.5);
      resize->SetResizeMethodToOutputDimensions();
      // switch (j)
      // {
      //   case 0:
      //   {
      for (int g = (NumberOfPyramidLevels-1); g>=1; g--)
      {
        // vtkSmartPointer<vtkImageWeightedSum> sumFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
        // sumFilter->SetWeight(0,0.5);
        // sumFilter->SetWeight(1,0.5);
        if (g == (NumberOfPyramidLevels-1))
        {
#if VTK_MAJOR_VERSION <= 5
          resize->SetInput(YDifference[g].imagedata);
#else
          resize->SetInputData(YDifference[g].imagedata);
#endif
        }

        else
        {
#if VTK_MAJOR_VERSION <= 5
          resize->SetInput(sumFilter->GetOutput());
#else
          resize->SetInputData(sumFilter->GetOutput());
#endif
        }

        sumFilter = NULL;
        sumFilter = vtkSmartPointer<vtkImageWeightedSum>::New();
        sumFilter->SetWeight(0,0.5);
        sumFilter->SetWeight(1,0.5);

        resize->SetOutputDimensions(imageDimension1/pow(2,(g-1)), imageDimension2/pow(2,(g-1)), -1);
        resize->Update();

        gaussianSmoothFilter->SetInputData(resize->GetOutput());
        gaussianSmoothFilter->Update();
        sumFilter->SetInputData(gaussianSmoothFilter->GetOutput());
        sumFilter->AddInputData(YDifference[g-1].imagedata);
        sumFilter->Update();
      }
      //---------------Pyramid Collapsed into image---------------------------


      //----------Chromatic Abberation to reduce noise---------------
      double chromatic_abberation = 0.1; //User given input
      vtkSmartPointer<vtkImageMathematics> chromaticCorrection =
        vtkSmartPointer<vtkImageMathematics>::New();
      chromaticCorrection->SetOperationToMultiplyByK();
      chromaticCorrection->SetConstantK(chromatic_abberation);
      switch(j)
      {
        //Do nothing for Y channel
        case 1:
        {
          for (int k=0; k<NumberOfPyramidLevels; k++)
          {
            chromaticCorrection->SetInput1Data(IDifference[k].imagedata);
            chromaticCorrection->Update();
            IDifference[k].imagedata->ShallowCopy(chromaticCorrection->GetOutput());
          }
          break;
        }
        case 2:
        {
          for (int k=0; k<NumberOfPyramidLevels; k++)
          {
            chromaticCorrection->SetInput1Data(QDifference[k].imagedata);
            chromaticCorrection->Update();
            QDifference[k].imagedata->ShallowCopy(chromaticCorrection->GetOutput());
          }
          break;
        }
      }

    } //End of iteration over image Pyramids
  } //End of iteration over
  // Now imagePyramid[i].imagedata stores vtkImageData*. Level 0 has the image and as we go higher up we have the smoothed and downsampled image.
  // For example,

//   std::string iterationNumberString = std::to_string(i);
//   std::string outputFileName = "OutputFrame" + iterationNumberString+".png";
//   vtkSmartPointer<vtkPNGWriter> writeDifferenceFrames = vtkSmartPointer<vtkPNGWriter>::New();
//   writeDifferenceFrames->SetFileName(outputFileName.c_str());
//   writeDifferenceFrames->SetInputConnection(scaleDifference->GetOutputPort());
//   writeDifferenceFrames->Write();
// // Visualize




  vtkSmartPointer<vtkDataSetMapper> mapper =
    vtkSmartPointer<vtkDataSetMapper>::New();


  // mapper->SetInputConnection(resize->GetOutputPort());

  //-------------------Suspected code snippet causing segmentation error----------------------------
  // Why does mapper->SetInputData(imagePyramid[0].imagedata);  throw up an error?
  mapper->SetInputData(YDifference[0].imagedata);
    // mapper->SetInputData(differenceFilter->GetOutput);
  // mapper->SetInputData(resize->GetOutput());
  //--------------------------------------------------------------------------------------------------

  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetRepresentationToWireframe();

  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(actor);
  renderer->ResetCamera();
  renderer->SetBackground(1,1,1);

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
