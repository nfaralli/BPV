#include "infowidgetbnd.h"
#include <QDoubleValidator>
#include <QGridLayout>
#include <QVBoxLayout>

InfoWidgetBnd::InfoWidgetBnd(Particles *part,QWidget *parent) : QGroupBox("Bonds",parent){

  resize(200,190);
  setCheckable(true);
  setChecked(false);
  setEnabled(false);

  QDoubleValidator *vd=new QDoubleValidator(this);

  wireBx=new QCheckBox("Wired bonds");
  radiusLb=new QLabel("Radius:");
  radiusLe=new QLineEdit;
  radiusLe->setValidator(vd);
  radiusLe->setMaximumWidth(60);
  radiusLe->setAlignment(Qt::AlignRight);
  slicesLb=new QLabel("Slices:");
  slicesBx=new QSpinBox;
  slicesBx->setMinimum(3);
  slicesBx->setMaximum(200);
  slicesBx->setMaximumWidth(60);
  slicesBx->setAlignment(Qt::AlignRight);

  QObject::connect(wireBx,SIGNAL(clicked(bool)),this,SLOT(setShowWires(bool)));
  QObject::connect(radiusLe,SIGNAL(editingFinished()),this,SLOT(changeRadius()));
  QObject::connect(slicesBx,SIGNAL(valueChanged(int)),this,SLOT(changeSlices(int)));

  QVBoxLayout *vLayout=new QVBoxLayout;
  vLayout->addWidget(wireBx);
  QGridLayout *gLayout=new QGridLayout;
  gLayout->addWidget(radiusLb,0,0);
  gLayout->addWidget(radiusLe,0,1);
  gLayout->addWidget(slicesLb,1,0);
  gLayout->addWidget(slicesBx,1,1);
  gLayout->setColumnStretch(3,1);
  vLayout->addLayout(gLayout);
  setLayout(vLayout);
  loadParticles(part);
}

void InfoWidgetBnd::loadParticles(Particles *particles){
  bool enable;

  setChecked(false);
  setEnabled(false);
  if(particles!=NULL){
    if(particles->nbBonds>0){
      bSpec=particles->bSpec;
      if(bSpec.showWires){
        wireBx->setCheckState(Qt::Checked);
        enable=false;
      }
      else{
        wireBx->setCheckState(Qt::Unchecked);
        enable=true;
      }
      radiusLb->setEnabled(enable);
      radiusLe->setEnabled(enable);
      slicesLb->setEnabled(enable);
      slicesBx->setEnabled(enable);
      radiusLe->setText(QString("%1").arg(bSpec.radius));
      slicesBx->setValue(bSpec.slices);
      setChecked(true);
      setEnabled(true);
    }
  }
}

void InfoWidgetBnd::changeRadius(){
  QString str=radiusLe->text();
  bool ok;
  float newRadius=str.toFloat(&ok);
  if(ok){
    bSpec.radius=newRadius;
    emit bondSpecChanged(bSpec);
  }
}

void InfoWidgetBnd::changeSlices(int newValue){
  bSpec.slices=newValue;
  emit bondSpecChanged(bSpec);
}

void InfoWidgetBnd::setShowWires(bool enable){
  radiusLb->setEnabled(!enable);
  radiusLe->setEnabled(!enable);
  slicesLb->setEnabled(!enable);
  slicesBx->setEnabled(!enable);
  bSpec.showWires=enable;
  emit bondSpecChanged(bSpec);
}
