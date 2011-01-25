#include <QApplication>
 
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageViewer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkJPEGReader.h>
 
#include <QVTKWidget.h>
 
int main(int argc, char** argv)
{
  QApplication app(argc, argv);
 
  QVTKWidget widget;
  widget.resize(512,256);
 
  //setup sphere
  vtkSmartPointer<vtkSphereSource> sphereSource = 
      vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->Update();
  vtkSmartPointer<vtkPolyDataMapper> sphereMapper = 
      vtkSmartPointer<vtkPolyDataMapper>::New();
  sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
  vtkSmartPointer<vtkActor> sphereActor = 
      vtkSmartPointer<vtkActor>::New();
  sphereActor->SetMapper(sphereMapper);
 
  //setup window
  vtkSmartPointer<vtkRenderWindow> renderWindow = 
      vtkSmartPointer<vtkRenderWindow>::New();
 
  //setup renderer
  vtkSmartPointer<vtkRenderer> renderer = 
      vtkSmartPointer<vtkRenderer>::New();
  renderWindow->AddRenderer(renderer);
 
  renderer->AddActor(sphereActor);
  renderer->ResetCamera();
 
  widget.SetRenderWindow(renderWindow);
  widget.show();
 
  app.exec();
 
  return EXIT_SUCCESS;
}
