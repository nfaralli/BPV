#include <QApplication>
#include "mymainwin.h"

int main(int argc, char *argv[]){
  QApplication app(argc, argv);
  MyMainWin win;
  win.show();
  if(argc==2)
    win.open(QString(argv[1]));
  return app.exec();
}
