#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include <math.h>
#include <QGLWidget>
#include <QButtonGroup>
#include <QTimer>
#include "particles.h"
#include "geometry.h"
#include "mycolor.h"
#include "blankingdialog.h"
#include "trajectorydialog.h"
#include "glgeometry.h"

#define ANGLE_DIV 32.0

class GLWidget : public QGLWidget
{

  Q_OBJECT

public:
  GLWidget(QWidget *parent = 0);
  void zoomFit();
  void loadParticles(Particles *part);
  void clearParticles();
  void loadGeometry(MyGeometry *geo);
  void clearGeometry();
  void enableGlList(bool);
  int  getStep(){return step;}
  void resizegl(int width, int height){resizeGL(width,height);}
  void getRotAngles(float &rx, float &ry, float &rz){rx=xRot/16.;ry=yRot/16.;rz=zRot/16.;}
  void getCenterSc(float &cx, float &cy, float &cz){cx=-xCenterSc;cy=-yCenterSc;cz=-zCenterSc;}
  void updateFps(int fps);

signals:
  void initializationDone();
  void stepChanged(int newStep);
  void rotAnglesChanged(float, float, float);
  void centerChanged(float,float,float);
  
public slots:
  void goToStep(int nextStep);
  void setEnabledAxis(bool enabled);
  void switchView(bool persp);  
  void setEnabledParticles(bool);
  void setEnabledBonds(bool);
  void setEnabledGeometry(bool);
  void updateBackground(MyColor);
  void setAnimTrajectory(bool);
  void setShowText(bool);
  void updateTextColor(QColor);
  void center();
  void setRotAngles(float,float,float);
  void setCenter(float,float,float);
  void setBlank(int,Blank);
  void setTrajectory(int,Trajectory);
  void setPartSpec(int,PartSpec);
  void setAllPartSpec(PartSpec*);
  void setBondSpec(BondSpec);
  void resetGeometry();
  void loadFont(char *font);

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);
  void keyPressEvent(QKeyEvent *event); //reimplementation of virtual function of QWidget
  void keyReleaseEvent(QKeyEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void drawScene(bool opaque);
  void drawTrajectory(Trajectory &traj);
  void drawAxis();
  float* getMinMax();
  float getMaxDim();
  void createParticlesList();
  void createGeometryList();
  void resetBlanks();
  bool blankParticle(int index, int theStep);
  void resetTrajectory();
  void printScr(int x, int y, const char *format, ...);
  void updateTimeStr(int theStep);
  void printMat(GLenum mode);

public:
  Particles  *particles;
  MyGeometry *geometry;
  
private:
  bool     shiftKeyPressed;
  int      xRot,yRot,zRot;
  float    xCen,yCen;
  float    xCenterSc,yCenterSc,zCenterSc;
  float    depth,depth0;
  QPoint   lastPos;
  float    fovy; //field of view in degree in the y direction
  float    zNear,zFar;
  int      step;
  GLuint   partsList;
  GLuint   geomsList;
  GLuint   fontList;
  bool     enabledAxis;
  bool     perspective;
  float    left,right,bottom,top;
  float    totScale;
  float    oldMaxDim;
  GLfloat  axisMatrix[16];
  GLdouble invProjectionMatrix[16];
  int      viewWidth, viewHeight;
  bool     enabledParticles;
  bool     enabledBonds;
  bool     enabledGeometry;
  bool     useGlList;
  int      nbParticles;
  int      nbBonds;
  int      nbTypes;
  MyColor  bgColor;
  bool     animTra;
  Blank    blanks[NB_BLANK];
  int      blankIndex[NB_BLANK];
  int      nbBlanksUsed;
  Trajectory trajectory[NB_TRAJECTORIES];
  int      trajIndex[NB_TRAJECTORIES];
  int      nbTrajActive;
  int      charWidth[95];
  int      charHeight;
  int      (**fontPixelsPos)[2];
  int      *fontPixelsNum;
  float    fontColor[3];
  int      timeXPos,timeYPos;
  char     timeStr[100];
  bool     showText;
  QTimer   *fpsTimer;
  int      fps;
  bool     isInitialized;
};

#endif
