#ifndef __MYWIDGET_H__
#define __MYWIDGET_H__

#include <QWidget>
#include <QPushButton>
#include <QFont>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QIcon>
#include <QObject>
#include <QFileDialog>
#include <QString>
#include <QTimer>
#include <QKeyEvent>

#include "glwidget.h"
#include "particles.h"
#include "geometry.h"
#include "exportdialog.h"
#include "rotatedialog.h"
#include "blankingdialog.h"
#include "trajectorydialog.h"
#include "textdialog.h"

class MyWidget : public QWidget{

  Q_OBJECT

public:
  MyWidget(QWidget *parent=0);
  void clearParticles();
  void clearGeometry();

signals:
  void stepChanged(int theStep);
  void axisToggled(bool enabled);
  void viewToggled(bool enabled);
  void particlesChanged(bool enabled);
  void geometryChanged(bool enabled);

public slots:
  void setNewParticles(Particles *part);
  void setNewGeometry(MyGeometry *geo);
  void zoomFit();
  void center();
  void setSpeed(GLint fps);
  void exportScene();
  void rotateScene();
  void setBlanking();
  void setTrajectory();
  void setText();

private slots:
  void playPause();
  void play();
  void pause();
  void stop();
  void updateWidget();
  void setDefaultFont();

protected:
  void keyPressEvent(QKeyEvent *event);

public:
  GLWidget         *glWidget;
  RotateDialog     *rotateDialog;
  BlankingDialog   *blankingDialog;
  TrajectoryDialog *trajectoryDialog;
  TextDialog       *textDialog;

private:
  QSlider      *timeSl;
  QPushButton  *playBt;
  QPushButton  *stopBt;
  QString      timeSt;
  QLabel       *timeLb;
  QHBoxLayout  *hLayout;
  QVBoxLayout  *mainLayout;
  bool         isPlaying;
  QIcon        playIcon;
  QIcon        pauseIcon;
  QIcon        stopIcon;
  Particles    *particles;
  MyGeometry   *geometry;
  GLint        framePerSec;
  QTimer       *timer;
  ExportDialog *exportDialog;
};

#endif
