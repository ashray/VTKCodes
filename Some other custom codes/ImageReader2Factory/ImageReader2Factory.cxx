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

int main(int argc, char *argv[])
{
  // Verify command line arguments
  // if(argc < 2)
  //   {
  //   std::cout << "Usage: " << argv[0]
  //             << " InputFilename" << std::endl;
  //   return EXIT_FAILURE;
  //   }

  // Parse command line arguments
  // std::string inputFilename = argv[1];
  std::string inputFilename = "image.png";

  // Read file
  vtkSmartPointer<vtkImageReader2Factory> readerFactory =
    vtkSmartPointer<vtkImageReader2Factory>::New();
  vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
  imageReader->SetFileName(inputFilename.c_str());
  imageReader->Update();

  // Create an actor
  vtkSmartPointer<vtkImageActor> actor =
    vtkSmartPointer<vtkImageActor>::New();
  actor->GetMapper()->SetInputConnection(
    imageReader->GetOutputPort());

  // Setup renderer
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(actor);
  renderer->ResetCamera();

  // Setup render window
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  // Setup render window interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleImage> style =
    vtkSmartPointer<vtkInteractorStyleImage>::New();

  renderWindowInteractor->SetInteractorStyle(style);

  // Render and start interaction
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderWindowInteractor->Start();

  imageReader->Delete();

  return EXIT_SUCCESS;
}
