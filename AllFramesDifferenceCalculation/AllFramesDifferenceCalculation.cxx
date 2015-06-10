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

#include <vtkGlobFileNames.h>
#include <vtksys/SystemTools.hxx>
#include <string>
#include <iostream>
#include <stdio.h>
#include "vtkStringArray.h"
#include <vtkPNGWriter.h>
#include <vtkImageWriter.h>

#include <vtkImageShiftScale.h>

#define DEBUG = 1
int main(int argc, char *argv[])
{
  // Verify command line arguments
  
  if(argc < 2)
    {
    std::cout << "Usage: " << argv[0]
              << " directoryName" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse command line arguments
  std::string directoryName = argv[1];
  
  // Create reader to read all images
  std::string dirString = directoryName;
  vtksys::SystemTools::ConvertToUnixSlashes(dirString);
  vtkSmartPointer<vtkGlobFileNames> glob = vtkSmartPointer<vtkGlobFileNames>::New();
  glob->SetDirectory(dirString.c_str());
  glob->AddFileNames("*");
  int frameCount = glob->GetNumberOfFileNames();
  cout << "Frame Count is "<< frameCount << "\n";
  //ostream& os;
  //vtkIndent vtk__indent
  //glob->PrintSelf();

  //vtkSmartPointer<vtkStringArray> imageFileNames = vtkSmartPointer<vtkStringArray>::New();
  //imageFileNames = glob->
  //cout << (glob->GetNthFileName(1));
  const char* oldFrameName;
  const char* newFrameName;
  oldFrameName = glob->GetNthFileName(0);
  for (int i=1; i<frameCount; i++)
  {
    newFrameName = glob->GetNthFileName(i);

//--------Need to convert the below part into a seperate function/method------
    vtkSmartPointer<vtkImageReader2Factory> readerFactory =
    vtkSmartPointer<vtkImageReader2Factory>::New();
    vtkImageReader2 * imageReader = readerFactory->CreateImageReader2(oldFrameName);
    imageReader->SetFileName(oldFrameName);
    imageReader->Update();

    // Read file 2
    vtkSmartPointer<vtkImageReader2Factory> readerFactory2 =
      vtkSmartPointer<vtkImageReader2Factory>::New();
    vtkImageReader2 * imageReader2 = readerFactory2->CreateImageReader2(newFrameName);
    imageReader2->SetFileName(newFrameName);
    imageReader2->Update();

    // Create difference Filter
    vtkSmartPointer<vtkImageDifference> differenceFilter = vtkSmartPointer<vtkImageDifference>::New();
    differenceFilter->AllowShiftOff();
    differenceFilter->AveragingOff();  //Do we really need this and what does it exactly do???
    differenceFilter->SetAllowShift(0);
    cout << "Is shift Allowed? " << differenceFilter->GetAllowShift() << "\n";
    cout << "Is averaging Allowed? " << differenceFilter->GetAveraging() << "\n";
    differenceFilter->SetThreshold(0);
    
    differenceFilter->SetInputConnection(imageReader->GetOutputPort());
    #if VTK_MAJOR_VERSION <= 5
      differenceFilter->SetImage(imageReader2->GetOutput());
    #else
      differenceFilter->SetImageConnection(imageReader2->GetOutputPort());
    #endif
      differenceFilter->Update();

    cout << "Total Error " << differenceFilter->GetError() << "\n";
    cout << "Error threshold " << differenceFilter->GetThreshold()<<"\n";
    cout << "Thresholded error "<< differenceFilter->GetThresholdedError()<<"\n";

    vtkSmartPointer<vtkImageShiftScale> scaleDifference = vtkSmartPointer<vtkImageShiftScale>::New();
    //scaleDifference->SetScale(5000);
    scaleDifference->SetScale(100);
    scaleDifference->SetInputConnection(differenceFilter->GetOutputPort());
    scaleDifference->Update();

    std::string iterationNumberString = std::to_string(i);
    std::string outputFileName = "OutputFrame" + iterationNumberString+".png";
    
//-------------Standard image writer not working, cant specify output format anywhere-----
    //Write output Image
/*    vtkSmartPointer<vtkImageWriter> writeDifferenceFrames = vtkSmartPointer<vtkImageWriter>::New();
    writeDifferenceFrames->SetFileName(outputFileName.c_str());
    writeDifferenceFrames->SetInputConnection(scaleDifference->GetOutputPort());
    writeDifferenceFrames->Write(); 
*/
//-----------------------------------------------------------------------------------------


    vtkSmartPointer<vtkPNGWriter> writeDifferenceFrames = vtkSmartPointer<vtkPNGWriter>::New();
    writeDifferenceFrames->SetFileName(outputFileName.c_str());
    writeDifferenceFrames->SetInputConnection(scaleDifference->GetOutputPort());
    writeDifferenceFrames->Write(); 


    // Create an actor
    vtkSmartPointer<vtkImageActor> actor =
      vtkSmartPointer<vtkImageActor>::New();
      actor->GetMapper()->SetInputConnection(scaleDifference->GetOutputPort());

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
  //----------------------------------------------------------------------------
    oldFrameName = newFrameName;

  }
  

  return EXIT_SUCCESS;
}
