#ifndef SimpleViewUI_H
#define SimpleViewUI_H
 
#include "vtkSmartPointer.h"
#include <QMainWindow>
 
// Forward Qt class declarations
class Ui_SimpleView;
 
class SimpleView : public QMainWindow
{
    Q_OBJECT
    public:
          SimpleView(); 
          ~SimpleView() {};
     
    public slots:
        virtual void slotExit();
    protected:
    protected slots:
    private:
        Ui_SimpleView *ui;
};
 
#endif // SimpleViewUI_H
