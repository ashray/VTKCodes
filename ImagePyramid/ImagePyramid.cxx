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

#include <vtkImageResize.h>
#include "vtkImageSincInterpolator.h"

// typedef int* imageType;

struct imageStruct {
vtkImageData * imagedata;
};
 
int main(int argc, char* argv[])
{

  // Verify input arguments
  // if(argc != 2)
  //   {
  //   std::cout << "Usage: " << argv[0]
  //             << " Filename.vti" << std::endl;
  //   return EXIT_FAILURE;
  //   }
 
  std::string inputFilename = "image.png";
 
  // Read the file
  vtkSmartPointer<vtkImageReader2Factory> readerFactory =
    vtkSmartPointer<vtkImageReader2Factory>::New();
  vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
  imageReader->SetFileName(inputFilename.c_str());
  imageReader->Update();

  // Somehow get the image pixel height and width from the above image reader. Search for functions

  // http://www.vtk.org/doc/nightly/html/classvtkImageMagnify.html
  int inputImageHeight = 544;
  int inputImageWidth = 960;

  int NumberOfPyramidLevels = 6;
  imageStruct imagePyramid[NumberOfPyramidLevels];
  // imageType images[6];
  // Used sips -g pixelWidth image.png to get the above values

  // int a = 0;
  // a = imageReader->GetFileDimensionality();
  // cout << a << "\n";
  // cout << imageReader->GetNumberOfScalarComponents() << "\n";
  for (int i=0; i<NumberOfPyramidLevels; i++){
  // Gaussian smoothing
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = 
      vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputConnection(imageReader->GetOutputPort());
    gaussianSmoothFilter->Update();

    // Can set standard deviation of the gaussian filter using "SetStandardDeviation (double std)"

    vtkSmartPointer<vtkImageResize> resize =
      vtkSmartPointer<vtkImageResize>::New();
  #if VTK_MAJOR_VERSION <= 5
    resize->SetInput(gaussianSmoothFilter->GetOutput());
  #else
    resize->SetInputData(gaussianSmoothFilter->GetOutput());
  #endif
    resize->SetOutputDimensions(inputImageHeight/8, inputImageWidth/8, 1);
    resize->Update();
    imagePyramid[i].imagedata = resize->GetOutput();
  }
 
  // Visualize
  vtkSmartPointer<vtkDataSetMapper> mapper =
    vtkSmartPointer<vtkDataSetMapper>::New();
  // mapper->SetInputConnection(resize->GetOutputPort());
  mapper->SetInputData((vtkDataSet *)imagePyramid[0].imagedata);
 
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

