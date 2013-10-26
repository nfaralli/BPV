#include "mywidget.h"
#include <QFile>
#include <QTextStream>
#include <QList>
#include <QVBoxLayout>
#include <QApplication>

/*returns the path of the executable.
 * if folder!=NULL, append folder to the path,
 * including separator.*/
QString getPath(const char *folder){
  QString path=qApp->arguments()[0];
  char separator;
  int index;
  if((index=path.lastIndexOf("\\"))>-1){
    path.truncate(index+1);
    separator='\\';
  }
  else if((index=path.lastIndexOf("/"))>-1){
    path.truncate(index+1);
    separator='/';
  }
  else{
    path=QString("./");
    separator='/';
    #ifdef ROOTDIR
    path=QString(ROOTDIR);
    if((index=path.lastIndexOf("\\"))>-1){
      if(index<path.size()-1)
        path+=QString("\\");
      separator='\\';
    }
    else if((index=path.lastIndexOf("/"))>-1){
      if(index<path.size()-1)
        path+=QString("/");
      separator='/';
    }
    else
    #endif
  }
  if(folder){
    path+=folder;
    path+=separator;
  }
  return path;
}

MyWidget::MyWidget(QWidget *parent):QWidget(parent){

  QString path;
  path=getPath("images");
  playIcon=QIcon(path+"play.png");
  pauseIcon=QIcon(path+"pause.png");
  stopIcon=QIcon(path+"stop.png");
  particles=NULL;
  geometry=NULL;
  isPlaying=false;

  float xRot,yRot,zRot;
  float xCen,yCen,zCen;
  glWidget = new GLWidget(this);
  QObject::connect(this,SIGNAL(axisToggled(bool)),glWidget,SLOT(setEnabledAxis(bool)));
  QObject::connect(this,SIGNAL(viewToggled(bool)),glWidget,SLOT(switchView(bool)));
  exportDialog = new ExportDialog(glWidget,this);
  rotateDialog = new RotateDialog(this);
  blankingDialog = new BlankingDialog(this);
  trajectoryDialog = new TrajectoryDialog(this);
  textDialog = new TextDialog(getPath("fonts"),this);
  glWidget->getRotAngles(xRot,yRot,zRot);
  glWidget->getCenterSc(xCen,yCen,zCen);
  glWidget->setFontPath(getPath("fonts").toAscii().data());
  rotateDialog->setRotAngles(xRot,yRot,zRot);
  rotateDialog->setCenter(xCen,yCen,zCen);
  QObject::connect(glWidget,SIGNAL(rotAnglesChanged(float,float,float)),rotateDialog,SLOT(setRotAngles(float,float,float)));
  QObject::connect(rotateDialog,SIGNAL(rotAnglesChanged(float,float,float)),glWidget,SLOT(setRotAngles(float,float,float)));
  QObject::connect(glWidget,SIGNAL(centerChanged(float,float,float)),rotateDialog,SLOT(setCenter(float,float,float)));
  QObject::connect(rotateDialog,SIGNAL(centerChanged(float,float,float)),glWidget,SLOT(setCenter(float,float,float)));
  QObject::connect(rotateDialog,SIGNAL(resetCenter()),glWidget,SLOT(center()));
  QObject::connect(blankingDialog,SIGNAL(blankChanged(int,Blank)),glWidget,SLOT(setBlank(int,Blank)));
  QObject::connect(trajectoryDialog,SIGNAL(animTraChanged(bool)),glWidget,SLOT(setAnimTrajectory(bool)));
  QObject::connect(trajectoryDialog,SIGNAL(trajectoryChanged(int,Trajectory)),glWidget,SLOT(setTrajectory(int,Trajectory)));
  QObject::connect(glWidget,SIGNAL(initializationDone()),this,SLOT(setDefaultFont()));
  QObject::connect(textDialog,SIGNAL(showTextChanged(bool)),glWidget,SLOT(setShowText(bool)));
  QObject::connect(textDialog,SIGNAL(colorChanged(QColor)),glWidget,SLOT(updateTextColor(QColor)));
  QObject::connect(textDialog,SIGNAL(fontChanged(char*)),glWidget,SLOT(loadFont(char*)));
  playBt = new QPushButton();
  playBt->setIcon(playIcon);
  playBt->setEnabled(false);
  QObject::connect(playBt,SIGNAL(clicked()),this,SLOT(playPause()));
  stopBt = new QPushButton();
  stopBt->setIcon(stopIcon);
  stopBt->setEnabled(false);
  QObject::connect(stopBt,SIGNAL(clicked()),this,SLOT(stop()));
  timeSl = new QSlider(Qt::Horizontal);
  timeSl->setEnabled(false);
  QObject::connect(timeSl,SIGNAL(valueChanged(int)),glWidget,SLOT(goToStep(int)));
  QObject::connect(glWidget,SIGNAL(stepChanged(int)),timeSl,SLOT(setValue(int)));
  QObject::connect(timeSl,SIGNAL(sliderPressed()),this,SLOT(pause()));
  hLayout = new QHBoxLayout;
  hLayout->addWidget(playBt);
  hLayout->addWidget(stopBt);
  hLayout->addWidget(timeSl);

  mainLayout = new QVBoxLayout;
  mainLayout->addWidget(glWidget);
  mainLayout->addLayout(hLayout);
  mainLayout->setStretchFactor(glWidget,1);
  setLayout(mainLayout);

  framePerSec=25;
  timer = new QTimer(this);
  timer->setInterval((int)(1000.0/framePerSec));
  QObject::connect(timer,SIGNAL(timeout()),this,SLOT(updateWidget()));
}

void MyWidget::setNewParticles(Particles *part){
  if(part!=NULL){
    stop();
    clearParticles();
    particles=part;
    glWidget->loadParticles(part);
    timeSl->setRange(0,particles->nbSteps-1);
    timeSl->setValue(0);
    playBt->setEnabled(true);
    stopBt->setEnabled(true);
    timeSl->setEnabled(true);
    emit particlesChanged(true);
    exportDialog->updateDialog();
    blankingDialog->setDialog(part);
    trajectoryDialog->setDialog(part);
  }
}

void MyWidget::clearParticles(){
  if(particles!=NULL){
    stop();
    glWidget->clearParticles();
    particles=NULL;
    playBt->setEnabled(false);
    stopBt->setEnabled(false);
    timeSl->setEnabled(false);
    emit particlesChanged(false);
  }
}

void MyWidget::setNewGeometry(MyGeometry *geo){
  if(geo!=NULL){
    stop();
    clearGeometry();
    geometry=geo;
    glWidget->loadGeometry(geo);
    emit geometryChanged(true);
    exportDialog->updateDialog();
  }
}

void MyWidget::clearGeometry(){
  if(geometry!=NULL){
    stop();
    glWidget->clearGeometry();
    geometry=NULL;
    emit geometryChanged(false);
  }
}

void MyWidget::exportScene(){
  pause();
  exportDialog->updateDialog();
  exportDialog->exec();
}

void MyWidget::rotateScene(){
  pause();
  rotateDialog->show();
}

void MyWidget::setBlanking(){
  pause();
  blankingDialog->show();
}

void MyWidget::setTrajectory(){
  pause();
  trajectoryDialog->show();
}

void MyWidget::setText(){
  pause();
  textDialog->show();
}

void MyWidget::center(){
  glWidget->center();
}

void MyWidget::zoomFit(){
  glWidget->zoomFit();
}

void MyWidget::playPause(){
  if(isPlaying)
    pause();
  else
    play();
}

void MyWidget::pause(){
  if(isPlaying){
    playBt->setIcon(playIcon);
    isPlaying=false;
    timer->stop();
  }
}

void MyWidget::play(){
  if(!isPlaying){
    if(timeSl->sliderPosition()==timeSl->maximum())
      timeSl->setValue(0);
    playBt->setIcon(pauseIcon);
    isPlaying=true;
    timer->start();
  }
}

void MyWidget::stop(){
  pause();
  timeSl->setValue(0);
}

void MyWidget::setSpeed(GLint fps){
  if(fps>0 &&fps<1000 && fps!=framePerSec){
    framePerSec=fps;
    timer->setInterval((int)(1000.0/framePerSec));
    glWidget->updateFps(framePerSec);
  }
}

void MyWidget::updateWidget(){
  if(timeSl->sliderPosition()<timeSl->maximum())
    timeSl->setValue(timeSl->sliderPosition()+1);
  else
    pause();
}

void MyWidget::keyPressEvent(QKeyEvent *event){
  event->accept();
  if(event->key()==Qt::Key_Plus)
    setSpeed(framePerSec+1);
  else if(event->key()==Qt::Key_Minus)
    setSpeed(framePerSec-1);
  else if(event->key()==Qt::Key_Space)
    playPause();
  else if(event->key()==Qt::Key_S)
    stop();
  else if(event->key()==Qt::Key_P)
    play();
  else
    event->ignore();
}

 void MyWidget::setDefaultFont(){
   glWidget->loadFont(textDialog->getFont().toAscii().data());
 }
