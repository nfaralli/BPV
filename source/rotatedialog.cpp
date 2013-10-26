#include "rotatedialog.h"

#include <QLabel>
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

RotateDialog::RotateDialog(QWidget *parent) : QDialog(parent){

  setWindowTitle(tr("Rotate"));

  centerGB=new QGroupBox("Center of Rotation");
  QDoubleValidator *vd=new QDoubleValidator(this);
  xCenterLe=new QLineEdit;
  xCenterLe->setValidator(vd);
  yCenterLe=new QLineEdit;
  yCenterLe->setValidator(vd);
  zCenterLe=new QLineEdit;
  zCenterLe->setValidator(vd);
  resetPB=new QPushButton("Reset");
  QGridLayout *gLayoutCenter=new QGridLayout;
  gLayoutCenter->addWidget(new QLabel("X"),0,0);
  gLayoutCenter->addWidget(xCenterLe,0,1);
  gLayoutCenter->addWidget(new QLabel("Y"),1,0);
  gLayoutCenter->addWidget(yCenterLe,1,1);
  gLayoutCenter->addWidget(new QLabel("Z"),2,0);
  gLayoutCenter->addWidget(zCenterLe,2,1);
  QHBoxLayout *hLayoutCenter=new QHBoxLayout;
  hLayoutCenter->addWidget(resetPB);
  QVBoxLayout *vLayoutCenter=new QVBoxLayout;
  vLayoutCenter->addLayout(gLayoutCenter);
  vLayoutCenter->addLayout(hLayoutCenter);
  centerGB->setLayout(vLayoutCenter);
  connect(xCenterLe,SIGNAL(editingFinished()),this,SLOT(setCenterSc()));
  connect(yCenterLe,SIGNAL(editingFinished()),this,SLOT(setCenterSc()));
  connect(zCenterLe,SIGNAL(editingFinished()),this,SLOT(setCenterSc()));
  connect(resetPB,SIGNAL(pressed()),this,SIGNAL(resetCenter()));

  anglesGB=new QGroupBox("Rotation Angles");
  xRotLe=new QLineEdit;
  xRotLe->setValidator(vd);
  yRotLe=new QLineEdit;
  yRotLe->setValidator(vd);
  zRotLe=new QLineEdit;
  zRotLe->setValidator(vd);
  QGridLayout *gLayoutRot=new QGridLayout;
  gLayoutRot->addWidget(new QLabel("X"),0,0);
  gLayoutRot->addWidget(xRotLe,0,1);
  gLayoutRot->addWidget(new QLabel("Y"),1,0);
  gLayoutRot->addWidget(yRotLe,1,1);
  gLayoutRot->addWidget(new QLabel("Z"),2,0);
  gLayoutRot->addWidget(zRotLe,2,1);
  anglesGB->setLayout(gLayoutRot);
  connect(xRotLe,SIGNAL(editingFinished()),this,SLOT(setRot()));
  connect(yRotLe,SIGNAL(editingFinished()),this,SLOT(setRot()));
  connect(zRotLe,SIGNAL(editingFinished()),this,SLOT(setRot()));

  viewsGB=new QGroupBox("Preset Views");
  xyViewPB=new QPushButton("XY");
  yzViewPB=new QPushButton("YZ");
  xzViewPB=new QPushButton("XZ");
  QHBoxLayout *hLayoutView=new QHBoxLayout;
  hLayoutView->addWidget(xyViewPB);
  hLayoutView->addWidget(yzViewPB);
  hLayoutView->addWidget(xzViewPB);
  viewsGB->setLayout(hLayoutView);
  connect(xyViewPB,SIGNAL(pressed()),this,SLOT(setXYView()));
  connect(yzViewPB,SIGNAL(pressed()),this,SLOT(setYZView()));
  connect(xzViewPB,SIGNAL(pressed()),this,SLOT(setXZView()));

  buttonBox=new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));

  QVBoxLayout *dialogLayout=new QVBoxLayout;
  dialogLayout->addWidget(centerGB);
  dialogLayout->addWidget(anglesGB);
  dialogLayout->addWidget(viewsGB);
  dialogLayout->addWidget(buttonBox);
  setLayout(dialogLayout);
  setFixedSize(sizeHint());
  setFixedWidth(200);
}

void RotateDialog::setCenter(float newX,float newY, float newZ){
  xCenterLe->setText(QString("%1").arg(newX));
  yCenterLe->setText(QString("%1").arg(newY));
  zCenterLe->setText(QString("%1").arg(newZ));
}

void RotateDialog::setRotAngles(float newXRot, float newYRot,float newZRot){
  while(newXRot<=-180) newXRot+=360;
  while(newXRot>180)   newXRot-=360;
  while(newYRot<=-180) newYRot+=360;
  while(newYRot>180)   newYRot-=360;
  while(newZRot<=-180) newZRot+=360;
  while(newZRot>180)   newZRot-=360;
  xRotLe->setText(QString("%1").arg(newXRot));
  yRotLe->setText(QString("%1").arg(newYRot));
  zRotLe->setText(QString("%1").arg(newZRot));
}

void RotateDialog::setXYView(){
  xRotLe->setText(QString("%1").arg(0));
  yRotLe->setText(QString("%1").arg(0));
  zRotLe->setText(QString("%1").arg(0));
  emit rotAnglesChanged(0,0,0);
}

void RotateDialog::setYZView(){
  xRotLe->setText(QString("%1").arg(-90));
  yRotLe->setText(QString("%1").arg(0));
  zRotLe->setText(QString("%1").arg(-90));
  emit rotAnglesChanged(-90,0,-90);
}

void RotateDialog::setXZView(){
  xRotLe->setText(QString("%1").arg(-90));
  yRotLe->setText(QString("%1").arg(0));
  zRotLe->setText(QString("%1").arg(0));
  emit rotAnglesChanged(-90,0,0);
}

void RotateDialog::setRot(){
  float newXRot=xRotLe->text().toFloat();
  float newYRot=yRotLe->text().toFloat();
  float newZRot=zRotLe->text().toFloat();
  while(newXRot<=-180) newXRot+=360;
  while(newXRot>180)   newXRot-=360;
  while(newYRot<=-180) newYRot+=360;
  while(newYRot>180)   newYRot-=360;
  while(newZRot<=-180) newZRot+=360;
  while(newZRot>180)   newZRot-=360;
  xRotLe->setText(QString("%1").arg(newXRot));
  yRotLe->setText(QString("%1").arg(newYRot));
  zRotLe->setText(QString("%1").arg(newZRot));
  emit rotAnglesChanged(newXRot,newYRot,newZRot);
}

void RotateDialog::setCenterSc(){
  emit centerChanged(xCenterLe->text().toFloat(),yCenterLe->text().toFloat(),zCenterLe->text().toFloat());
}
