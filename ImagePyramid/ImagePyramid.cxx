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
#include <stdio.h>
#include "vtkStringArray.h"

#include <vtkImageShiftScale.h>



// typedef int* imageType;

struct imageStruct {
vtkImageData * imagedata;
};
 
int main(int argc, char* argv[])
{
  //Have to define these variables as global since used in mapper
  int NumberOfPyramidLevels = 6;
  imageStruct YPyramid[NumberOfPyramidLevels];
  imageStruct IPyramid[NumberOfPyramidLevels];
  imageStruct QPyramid[NumberOfPyramidLevels];


  // Verify input arguments
  // if(argc != 2)
  //   {
  //   std::cout << "Usage: " << argv[0]
  //             << " Filename.vti" << std::endl;
  //   return EXIT_FAILURE;
  //   }
  // if(argc < 2)
  //   {
  //   std::cout << "Usage: " << argv[0]
  //             << " directoryName" << std::endl;
  //   return EXIT_FAILURE;
  //   }

  // Parse command line arguments
  std::string directoryName = "/Users/ashraymalhotra/Desktop/Academic/VTKDev/Data/ImageSequence";
  
  // Create reader to read all images
  std::string dirString = directoryName;
  vtksys::SystemTools::ConvertToUnixSlashes(dirString);
  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  glob->SetDirectory(dirString.c_str());
  glob->AddFileNames("*");
  int frameCount = glob->GetNumberOfFileNames();
  cout << "Number of Images read " << frameCount << "\n";

  // cout << (glob->GetNthFileName(1));
 

  for (int ImageNumber = 0; ImageNumber < frameCount; ImageNumber++){
    std::string inputFilename = glob->GetNthFileName(ImageNumber);
    inputFilename = directoryName + inputFilename;
    cout << inputFilename << "\n";
    
    // std::string inputFilename = "image.png";
   
    // Read the file
    vtkSmartPointer<vtkImageReader2Factory> readerFactory =
      vtkSmartPointer<vtkImageReader2Factory>::New();
    vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
    imageReader->SetFileName(inputFilename.c_str());
    imageReader->Update();

    cout << "reached here";


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
    imageStruct YPyramidPreviousFrame[NumberOfPyramidLevels];
    imageStruct IPyramidPreviousFrame[NumberOfPyramidLevels];
    imageStruct QPyramidPreviousFrame[NumberOfPyramidLevels];
    imageStruct YDifference[NumberOfPyramidLevels];
    imageStruct IDifference[NumberOfPyramidLevels];
    imageStruct QDifference[NumberOfPyramidLevels];
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
      //---------------------Parts below make the image pyramid---------------------
      // http://www.vtk.org/doc/nightly/html/classvtkImageMagnify.html
      // Somehow get the image pixel height and width from the image reader. Search for functions
      int inputImageHeight = 544;
      int inputImageWidth = 960;

      imageStruct imagePyramid[NumberOfPyramidLevels];
      vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter;
      vtkSmartPointer<vtkImageResize> resize;
      // imageType images[6];
      // Used sips -g pixelWidth image.png to get the above values

      // int a = 0;
      // a = imageReader->GetFileDimensionality();
      // cout << a << "\n";
      // cout << imageReader->GetNumberOfScalarComponents() << "\n";
      imagePyramid[0].imagedata = imageSource->GetOutput();

      // if (!imagePyramid[0].imagedata) printf("0 is NULL!\n");
      for (int i=1; i<NumberOfPyramidLevels; i++){
      // Gaussian smoothing
        // vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = 
        //   vtkSmartPointer<vtkImageGaussianSmooth>::New();
        // gaussianSmoothFilter->SetInputConnection(imageReader->GetOutputPort());
        // gaussianSmoothFilter->Update();

        gaussianSmoothFilter = 
          vtkSmartPointer<vtkImageGaussianSmooth>::New();
        // gaussianSmoothFilter->SetInputData(imagePyramid[i-1].imagedata);
        gaussianSmoothFilter->SetInputData(imagePyramid[i-1].imagedata);
        gaussianSmoothFilter->Update();

        // Can set standard deviation of the gaussian filter using "SetStandardDeviation (double std)"

        resize = vtkSmartPointer<vtkImageResize>::New();
      #if VTK_MAJOR_VERSION <= 5
        resize->SetInput(gaussianSmoothFilter->GetOutput());
      #else
        resize->SetInputData(gaussianSmoothFilter->GetOutput());
      #endif
        resize->SetOutputDimensions(inputImageHeight/(2*i), inputImageWidth/(2*i), 1);
        resize->Update();
        imagePyramid[i].imagedata = resize->GetOutput();
        //imagePyramid[i].imagedata = imagePyramid[0].imagedata;
        // if (!imagePyramid[i].imagedata) printf("%d is NULL!\n", i);
      }

      // Initialising the Previous frame pyramid for frame 1. We initialise it to the first frame(which means first value of frame difference is going to be zero)
      if (ImageNumber==1)
      {
        switch (j){
        case 0:
        {
          //Make the code below in a seperate function to copy image pyramids
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            YPyramidPreviousFrame[k].imagedata = YPyramid[k].imagedata;
          }
          break;
        }
        case 1:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            IPyramidPreviousFrame[k].imagedata = IPyramid[k].imagedata;
          }
          break;
        }
        case 2:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            QPyramidPreviousFrame[k].imagedata = QPyramid[k].imagedata;
          }
          break;
        }
      }
      }
      
      //---------------------------------------------------------------------------------
      switch (j){
        case 0:
        {
          //Make the code below in a seperate function to copy image pyramids
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            YPyramid[k].imagedata = imagePyramid[k].imagedata;
          }
          break;
        }
        case 1:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            IPyramid[k].imagedata = imagePyramid[k].imagedata;
          }
          break;
        }
        case 2:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            QPyramid[k].imagedata = imagePyramid[k].imagedata;
          }
          break;
        }
      }

      //Writing this again so that it is easier later to seperate into different functions
      switch (j){
        case 0:
        {
          //Make the code below in a seperate function to copy image pyramids
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            YDifference[k].imagedata = Difference [ YPyramid[k].imagedata - YPyramidPreviousFrame[k].imagedata ];
            YPyramidPreviousFrame[k].imagedata = YPyramid[k].imagedata;
          }
          break;

        }
        case 1:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            IPyramid[k].imagedata = imagePyramid[k].imagedata;
          }
          break;
        }
        case 2:
        {
          for (int k=1; k<NumberOfPyramidLevels; k++)
          {
            QPyramid[k].imagedata = imagePyramid[k].imagedata;
          }
          break;
        }
      }


    }
  }
  // Now imagePyramid[i].imagedata stores vtkImageData*. Level 0 has the image and as we go higher up we have the smoothed and downsampled image.
  // For example, 
 
  // Visualize
  vtkSmartPointer<vtkDataSetMapper> mapper =
    vtkSmartPointer<vtkDataSetMapper>::New();
  // mapper->SetInputConnection(resize->GetOutputPort());

  //-------------------Suspected code snippet causing segmentation error----------------------------  
  // Why does mapper->SetInputData(imagePyramid[0].imagedata);  throw up an error?
  mapper->SetInputData(YPyramid[0].imagedata);
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

