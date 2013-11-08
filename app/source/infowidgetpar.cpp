#include "infowidgetpar.h"
#include <QString>
#include <QColorDialog>
#include <QString>
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "colordialog.h"

InfoWidgetPar::InfoWidgetPar(Particles *part,QWidget *parent) : QGroupBox("Particles",parent){

  resize(200,190);
  setCheckable(true);
  setChecked(false);
  setEnabled(false);

  QDoubleValidator *vd=new QDoubleValidator(this);

  nbTypes=0;
  minMaxParticles=NULL;
  pSpec=NULL;
  activeTypeBx=new QCheckBox();
  typeBx=new QComboBox;
  colorLb=new QLabel("Color:");
  selColorBt=new QPushButton();
  colorBt=new QPushButton("Apply to all");
  transpLb=new QLabel("Transparence:");
  transpSl=new QSlider(Qt::Horizontal);
  transpSl->setRange(0,255);
  transpBt=new QPushButton("Apply to all");
  pointBx=new QCheckBox("Points");
  pointBt=new QPushButton("Apply to all");
  ptSizeLb=new QLabel("Point size:");
  ptSizeSb=new QSpinBox;
  ptSizeSb->setMinimum(1);
  ptSizeSb->setMaximum(20);
  ptSizeSb->setAlignment(Qt::AlignRight);
  ptSizeBt=new QPushButton("Apply to all");
  radiusLb=new QLabel("Radius:");
  radiusLe=new QLineEdit;
  radiusLe->setValidator(vd);
  radiusLe->setAlignment(Qt::AlignRight);
  radiusBt=new QPushButton("Apply to all");
  segsLb=new QLabel("Segments:");
  segsBx=new QSpinBox;
  segsBx->setMinimum(1);
  segsBx->setMaximum(32);
  segsBx->setAlignment(Qt::AlignRight);
  segsBt=new QPushButton("Apply to all");

  QObject::connect(activeTypeBx,SIGNAL(clicked(bool)),this,SLOT(changeActiveType(bool)));
  QObject::connect(typeBx,SIGNAL(currentIndexChanged (int)),this,SLOT(setType(int)));
  QObject::connect(selColorBt,SIGNAL(pressed()),this,SLOT(changeColor()));
  QObject::connect(colorBt,SIGNAL(pressed()),this,SLOT(changeAllColor()));
  QObject::connect(transpSl,SIGNAL(valueChanged(int)),this,SLOT(changeTransparence()));
  QObject::connect(transpBt,SIGNAL(pressed()),this,SLOT(changeAllTransparence()));
  QObject::connect(pointBx,SIGNAL(clicked(bool)),this,SLOT(changeShowPoints(bool)));
  QObject::connect(pointBt,SIGNAL(pressed()),this,SLOT(changeAllShowPoints()));
  QObject::connect(ptSizeSb,SIGNAL(valueChanged(int)),this,SLOT(changePtSize(int)));
  QObject::connect(ptSizeBt,SIGNAL(pressed()),this,SLOT(changeAllPtSize()));
  QObject::connect(radiusLe,SIGNAL(editingFinished()),this,SLOT(changeRadius()));
  QObject::connect(radiusBt,SIGNAL(pressed()),this,SLOT(changeAllRadius()));
  QObject::connect(segsBx,SIGNAL(valueChanged(int)),this,SLOT(changeSegs(int)));
  QObject::connect(segsBt,SIGNAL(pressed()),this,SLOT(changeAllSegs()));

  QVBoxLayout *vLayout = new QVBoxLayout;
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->addWidget(activeTypeBx);
  hLayout->addWidget(typeBx);
  hLayout->setStretchFactor(typeBx,1);
  vLayout->addLayout(hLayout);
  QGridLayout *gLayout = new QGridLayout;
  gLayout->addWidget(colorLb,0,0);
  gLayout->addWidget(selColorBt,0,1);
  gLayout->addWidget(colorBt,0,2);
  gLayout->addWidget(transpLb,1,0);
  gLayout->addWidget(transpSl,1,1);
  gLayout->addWidget(transpBt,1,2);
  gLayout->addWidget(pointBx,2,0);
  gLayout->addWidget(pointBt,2,2);
  gLayout->addWidget(ptSizeLb,3,0);
  gLayout->addWidget(ptSizeSb,3,1);
  gLayout->addWidget(ptSizeBt,3,2);
  gLayout->addWidget(radiusLb,4,0);
  gLayout->addWidget(radiusLe,4,1);
  gLayout->addWidget(radiusBt,4,2);
  gLayout->addWidget(segsLb,5,0);
  gLayout->addWidget(segsBx,5,1);
  gLayout->addWidget(segsBt,5,2);
  vLayout->addLayout(gLayout);
  setLayout(vLayout);
  loadParticles(part);
}

void InfoWidgetPar::loadParticles(Particles *particles){
  int k;

  freeParticles(minMaxParticles);
  minMaxParticles=NULL;
  if(pSpec!=NULL)
    free(pSpec);
  pSpec=NULL;
  nbTypes=0;
  if(particles!=NULL){
    minMaxParticles=minMaxParticleCopy(particles);
    nbTypes=particles->nbTypes;
    pSpec=(PartSpec*)calloc(nbTypes,sizeof(PartSpec));
    memcpy(pSpec,particles->pSpec,nbTypes*sizeof(PartSpec));
    while(typeBx->count())
      typeBx->removeItem(typeBx->count()-1);
    for(k=0;k<nbTypes;k++)
      typeBx->addItem(pSpec[k].name);
    typeBx->setCurrentIndex(0);
    setType(0);
    setChecked(true);
    setEnabled(true);
  }
  else{
    while(typeBx->count())
      typeBx->removeItem(typeBx->count()-1);
    setChecked(false);
    setEnabled(false);
  }
}

void InfoWidgetPar::enableWidgets(){
  bool enableType,enablePoint;
  enableType=activeTypeBx->checkState()==Qt::Checked;
  enablePoint=pointBx->checkState()==Qt::Checked;
  colorLb->setEnabled(enableType);
  selColorBt->setEnabled(enableType);
  colorBt->setEnabled(enableType);
  transpLb->setEnabled(enableType);
  transpSl->setEnabled(enableType);
  transpBt->setEnabled(enableType);
  pointBx->setEnabled(enableType);
  pointBt->setEnabled(enableType);
  ptSizeLb->setEnabled(enableType && enablePoint);
  ptSizeSb->setEnabled(enableType && enablePoint);
  ptSizeBt->setEnabled(enableType && enablePoint);
  radiusLb->setEnabled(enableType && !enablePoint);
  radiusLe->setEnabled(enableType && !enablePoint);
  radiusBt->setEnabled(enableType && !enablePoint);
  segsLb->setEnabled(enableType && !enablePoint);
  segsBx->setEnabled(enableType && !enablePoint);
  segsBt->setEnabled(enableType && !enablePoint);
}

void InfoWidgetPar::setType(int type){
  if(type>=0 && type<nbTypes){
    selColorBt->setIcon(pSpec[type].color.getIcon(20,20));
    transpSl->setValue(255-pSpec[type].color().alpha());
    radiusLe->setText(QString("%1").arg(pSpec[type].radius));
    segsBx->setValue(pSpec[type].segs);
    ptSizeSb->setValue(pSpec[type].pointSize);
    activeTypeBx->setCheckState(pSpec[type].active?Qt::Checked:Qt::Unchecked);
    pointBx->setCheckState(pSpec[type].showPoints?Qt::Checked:Qt::Unchecked);
    enableWidgets();
  }
}

void InfoWidgetPar::changeActiveType(bool enable){
  if(pSpec!=NULL){
    pSpec[typeBx->currentIndex()].active=enable;
    enableWidgets();
    emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
  }
}

void InfoWidgetPar::changeColor(){
  if(minMaxParticles!=NULL){
    MyColor newColor=ColorDialog::getColor(minMaxParticles,&(pSpec[typeBx->currentIndex()].color));
    if(newColor.isValid()){
      newColor.setAlpha(255-transpSl->value());
      pSpec[typeBx->currentIndex()].color=newColor;
      selColorBt->setIcon(newColor.getIcon(20,20));
      emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
    }
  }
}

void InfoWidgetPar::changeAllColor(){
  for(int i=0;i<nbTypes;i++){
    MyColor newColor=pSpec[typeBx->currentIndex()].color;
    newColor.setAlpha(pSpec[i].color.alpha());
    pSpec[i].color=newColor;
  }
  emit allPartSpecChanged(pSpec);
}

void InfoWidgetPar::changeTransparence(){
  if(pSpec!=NULL){
    if(pSpec[typeBx->currentIndex()].color.alpha()!=255-transpSl->value()){
      pSpec[typeBx->currentIndex()].color.setAlpha(255-transpSl->value());
      emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
    }
  }
}

void InfoWidgetPar::changeAllTransparence(){
  for(int i=0;i<nbTypes;i++)
    pSpec[i].color.setAlpha(pSpec[typeBx->currentIndex()].color.alpha());
  emit allPartSpecChanged(pSpec);
}

void InfoWidgetPar::changeShowPoints(bool enable){
  if(pSpec!=NULL){
    pSpec[typeBx->currentIndex()].showPoints=enable;
    enableWidgets();
    emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
  }
}

void InfoWidgetPar::changeAllShowPoints(){
  for(int i=0;i<nbTypes;i++)
    pSpec[i].showPoints=pSpec[typeBx->currentIndex()].showPoints;
  emit allPartSpecChanged(pSpec);
}

void InfoWidgetPar::changePtSize(int size){
  if(pSpec!=NULL){
    pSpec[typeBx->currentIndex()].pointSize=size;
    emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
  }
}

void InfoWidgetPar::changeAllPtSize(){
  for(int i=0;i<nbTypes;i++)
    pSpec[i].pointSize=pSpec[typeBx->currentIndex()].pointSize;
  emit allPartSpecChanged(pSpec);
}

void InfoWidgetPar::changeRadius(){
  if(pSpec!=NULL){
    QString str=radiusLe->text();
    bool ok;
    float newRadius=str.toFloat(&ok);
    if(ok){
      pSpec[typeBx->currentIndex()].radius=newRadius;
      emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
    }
  }
}

void InfoWidgetPar::changeAllRadius(){
  for(int i=0;i<nbTypes;i++)
    pSpec[i].radius=pSpec[typeBx->currentIndex()].radius;
  emit allPartSpecChanged(pSpec);
}

void InfoWidgetPar::changeSegs(int newSegs){
  if(pSpec!=NULL){
    pSpec[typeBx->currentIndex()].segs=newSegs;
    emit partSpecChanged(typeBx->currentIndex(),pSpec[typeBx->currentIndex()]);
  }
}

void InfoWidgetPar::changeAllSegs(){
  for(int i=0;i<nbTypes;i++)
    pSpec[i].segs=pSpec[typeBx->currentIndex()].segs;
  emit allPartSpecChanged(pSpec);
}
