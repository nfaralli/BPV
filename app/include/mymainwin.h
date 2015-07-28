#ifndef __MYMAINWIN_H__
#define __MYMAINWIN_H__

#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QIcon>

#include "mywidget.h"
#include "particles.h"
#include "geometry.h"
#include "infowidgetbgd.h"
#include "infowidgetpar.h"
#include "infowidgetbnd.h"
#include "infowidgetmsh.h"
#include "infowidgetgeo.h"
#include "mycolor.h"

class MyMainWin : public QMainWindow{

  Q_OBJECT

public :
  MyMainWin();
  void open(QString);

private slots:
  void open();
  void about();

private:
  void createDockWindows();
  void setDockWidget();

  MyWidget      *myWidget;
  QMenu         *viewMenu;
  QMenu         *fileMenu;
  QMenu         *helpMenu;
  QAction       *openAct;
  QAction       *exportAct;
  QAction       *exitAct;
  QAction       *zoomFitAct;
  QAction       *centerAct;
  QAction       *rotateAct;
  QAction       *blankingAct;
  QAction       *trajectoryAct;
  QAction       *textAct;
  QAction       *visuAct;
  QAction       *axisAct;
  QAction       *viewAct;
  QAction       *aboutAct;
  QAction       *aboutQtAct;
  Particles     *particles;
  MyGeometry    *geometry;
  InfoWidgetBgd *infoBgd;
  InfoWidgetPar *infoPar;
  InfoWidgetBnd *infoBnd;
  InfoWidgetMsh *infoMsh;
  InfoWidgetGeo *infoGeo;
  QDockWidget   *dockWidget;
};

#endif
