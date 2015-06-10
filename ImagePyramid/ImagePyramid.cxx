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



  // Gaussian smoothing
  vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = 
    vtkSmartPointer<vtkImageGaussianSmooth>::New();
  gaussianSmoothFilter->SetInputConnection(imageReader->GetOutputPort());
  gaussianSmoothFilter->Update();

  // Can set standard deviation of the gaussian filter using "SetStandardDeviation (double std)"
 
  // Visualize
  vtkSmartPointer<vtkDataSetMapper> mapper =
    vtkSmartPointer<vtkDataSetMapper>::New();
  mapper->SetInputConnection(gaussianSmoothFilter->GetOutputPort());
 
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