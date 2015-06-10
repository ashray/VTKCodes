#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkImageData.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageDifference.h>
#include <vtkImageMapper3D.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>

#include <vtkImageShiftScale.h>

int main(int argc, char *argv[])
{
  // Verify command line arguments
  if(argc < 2)
    {
    std::cout << "Usage: " << argv[0]
              << " InputFilename" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse command line arguments
  std::string inputFilename = argv[1];
  std::string inputFilename2 = argv[2];

  // Read file 1
  vtkSmartPointer<vtkImageReader2Factory> readerFactory =
    vtkSmartPointer<vtkImageReader2Factory>::New();
  vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(inputFilename.c_str());
  imageReader->SetFileName(inputFilename.c_str());
  imageReader->Update();

  // Read file 2
  vtkSmartPointer<vtkImageReader2Factory> readerFactory2 =
    vtkSmartPointer<vtkImageReader2Factory>::New();
  vtkImageReader2 * imageReader2 = readerFactory2->CreateImageReader2(inputFilename2.c_str());
  imageReader2->SetFileName(inputFilename2.c_str());
  imageReader2->Update();

  // Create difference Filter
  vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();

  differenceFilter->SetInputConnection(imageReader->GetOutputPort());
  #if VTK_MAJOR_VERSION <= 5
    differenceFilter->SetImage(imageReader2->GetOutput());
  #else
    differenceFilter->SetImageConnection(imageReader2->GetOutputPort());
  #endif
    differenceFilter->Update();

  // Create difference scale filter
  vtkSmartPointer<vtkImageShiftScale> scaleDifference = vtkSmartPointer<vtkImageShiftScale>::New();
  scaleDifference->SetScale(5000);
  scaleDifference->SetInputConnection(differenceFilter->GetOutputPort());


  // Create an actor
  vtkSmartPointer<vtkImageActor> actor =
    vtkSmartPointer<vtkImageActor>::New();
  //actor->GetMapper()->SetInputConnection(imageReader->GetOutputPort());
  actor->GetMapper()->SetInputConnection(scaleDifference->GetOutputPort());

  // Create actor 2
  vtkSmartPointer<vtkImageActor> actor2 =
    vtkSmartPointer<vtkImageActor>::New();
  actor2->GetMapper()->SetInputConnection(
    imageReader2->GetOutputPort());

  // Setup renderer
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(actor);
  renderer->ResetCamera();

  // Setup renderer 2
  //----Try adding two actors to a single renderer----
  vtkSmartPointer<vtkRenderer> renderer2 =
    vtkSmartPointer<vtkRenderer>::New();
  renderer2->AddActor(actor2);
  renderer2->ResetCamera();

  // Setup render window
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  // Setup render window 2
  vtkSmartPointer<vtkRenderWindow> renderWindow2 =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow2->AddRenderer(renderer2);

  // Setup render window interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleImage> style =
    vtkSmartPointer<vtkInteractorStyleImage>::New();

  renderWindowInteractor->SetInteractorStyle(style);

  // Setup render window interactor 2
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor2 =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleImage> style2 =
    vtkSmartPointer<vtkInteractorStyleImage>::New();

  renderWindowInteractor2->SetInteractorStyle(style2);

  // Render and start interaction
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderWindowInteractor2->SetRenderWindow(renderWindow2);
  renderWindowInteractor2->Initialize();  

  renderWindowInteractor->Start();
  renderWindowInteractor2->Start();

  imageReader->Delete();
  imageReader2->Delete();

  return EXIT_SUCCESS;
}
