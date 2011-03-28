#include <QApplication>
#include "glwidget.h"

int main( int argc, char **argv )
{
  QApplication a( argc, argv );
  GLWidget *w = new GLWidget();
  w->show();
  return a.exec();
}
