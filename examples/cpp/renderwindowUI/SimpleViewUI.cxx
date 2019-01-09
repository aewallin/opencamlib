#include "ui_SimpleViewUI.h"
#include "SimpleViewUI.h"
 
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkConeSource.h>
 
#include "vtkSmartPointer.h"
 
// Constructor
SimpleView::SimpleView() {
    this->ui = new Ui_SimpleView;
    this->ui->setupUi(this);
 
    // sphere
    vtkSmartPointer<vtkConeSource> sphereSource = vtkSmartPointer<vtkConeSource>::New();
    sphereSource->Update();
    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
    vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
     
    // VTK Renderer
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(sphereActor);
 
    // VTK/Qt wedded
    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
 
    // Set up action signals and slots
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
 
};
 
void SimpleView::slotExit() {
  qApp->exit();
}
