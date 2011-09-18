#include <QApplication>
#include "nehewidget.h"

int main( int argc, char **argv )
{
  QApplication a( argc, argv );
  NeHeWidget *w = new NeHeWidget();
  w->show();
  return a.exec();
}
