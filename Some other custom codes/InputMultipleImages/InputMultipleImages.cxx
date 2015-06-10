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
#include <stdio.h>
#include "vtkStringArray.h"

#include <vtkImageShiftScale.h>

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
  cout << frameCount;
  cout << "\n";
  //ostream& os;
  //vtkIndent vtk__indent
  //glob->PrintSelf();

  //vtkSmartPointer<vtkStringArray> imageFileNames = vtkSmartPointer<vtkStringArray>::New();
  //imageFileNames = glob->
  cout << (glob->GetNthFileName(1));

  

  return EXIT_SUCCESS;
}
