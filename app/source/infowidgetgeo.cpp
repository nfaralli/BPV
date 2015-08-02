#include "infowidgetgeo.h"
#include "mycolor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QColorDialog>

InfoWidgetGeo::InfoWidgetGeo(MyGeometry *geom,QWidget *parent) : QGroupBox("Geometry",parent){

  setCheckable(true);
  setChecked(false);
  setEnabled(false);

  activeGeoBx=new QCheckBox();
  geoBx=new QComboBox;
  colorLb=new QLabel("Color:");
  selColorBt=new QPushButton();
  transpLb=new QLabel("Transparency:");
  transpSl=new QSlider(Qt::Horizontal);
  transpSl->setRange(0,255);

  QObject::connect(activeGeoBx,SIGNAL(clicked(bool)),this,SLOT(changeActiveGeo(bool)));
  QObject::connect(geoBx,SIGNAL(currentIndexChanged (int)),this,SLOT(setGeo(int)));
  QObject::connect(selColorBt,SIGNAL(pressed()),this,SLOT(changeColor()));
  QObject::connect(transpSl,SIGNAL(valueChanged(int)),this,SLOT(changeTransparency()));

  QVBoxLayout *vLayout = new QVBoxLayout;
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->addWidget(activeGeoBx);
  hLayout->addWidget(geoBx);
  hLayout->setStretchFactor(geoBx,1);
  vLayout->addLayout(hLayout);
  QGridLayout *gLayout = new QGridLayout;
  gLayout->addWidget(colorLb,0,0);
  gLayout->addWidget(selColorBt,0,2);
  gLayout->addWidget(transpLb,1,0);
  gLayout->addWidget(transpSl,1,2);
  gLayout->setColumnStretch(1,1);
  gLayout->setColumnStretch(3,1);
  vLayout->addLayout(gLayout);
  setLayout(vLayout);
  loadGeometry(geom);
}

void InfoWidgetGeo::loadGeometry(MyGeometry *geom){
  int k;

  if(geom!=NULL){
    if(geom->nbObj>0){
      geometry=geom;
      nbObj=geom->nbObj;
      nbBoxes=geom->boxes.size();
      nbCylinders=geom->cylinders.size();
      nbSpheres=geom->spheres.size();
      nbLines=geom->lines.size();
      nbMeshes=geom->meshes.size();
      while(geoBx->count())      geoBx->removeItem(geoBx->count()-1);
      for(k=0;k<nbBoxes;k++)     geoBx->addItem(geom->boxes[k].name);
      for(k=0;k<nbCylinders;k++) geoBx->addItem(geom->cylinders[k].name);
      for(k=0;k<nbSpheres;k++)   geoBx->addItem(geom->spheres[k].name);
      for(k=0;k<nbLines;k++)     geoBx->addItem(geom->lines[k].name);
      for(k=0;k<nbMeshes;k++)    geoBx->addItem(geom->meshes[k].name);
      geoBx->setCurrentIndex(0);
      setGeo(0);
      setChecked(true);
      setEnabled(true);
    }
  }
  else{
    geometry=NULL;
    setChecked(false);
    setEnabled(false);
  }
}

void InfoWidgetGeo::enableWidgets(){
  bool enableObj=activeGeoBx->checkState()==Qt::Checked;
  colorLb->setEnabled(enableObj);
  selColorBt->setEnabled(enableObj);
  transpLb->setEnabled(enableObj);
  transpSl->setEnabled(enableObj);
}

void InfoWidgetGeo::setGeo(int geo){
  int  col[4];
  bool active;
  if(geo>=0 && geo<nbObj){
    if(geo<nbBoxes){
      memcpy(col,geometry->boxes[geo].color,4*sizeof(int));
      active=geometry->boxes[geo].active;
    }
    else if((geo-=nbBoxes)<nbCylinders){
      memcpy(col,geometry->cylinders[geo].color,4*sizeof(int));
      active=geometry->cylinders[geo].active;
    }
    else if((geo-=nbCylinders)<nbSpheres){
      memcpy(col,geometry->spheres[geo].color,4*sizeof(int));
      active=geometry->spheres[geo].active;
    }
    else if((geo-=nbSpheres)<nbLines){
      memcpy(col,geometry->lines[geo].color,4*sizeof(int));
      active=geometry->lines[geo].active;
    }
    else if((geo-=nbLines)<nbMeshes){
      memcpy(col,geometry->meshes[geo].color,4*sizeof(int));
      active=geometry->meshes[geo].active;
    }
    else return; //this should never happen...
    MyColor color(col[0],col[1],col[2],col[3]);
    selColorBt->setIcon(color.getIcon(20,20));
    transpSl->setValue(255-color().alpha());
    activeGeoBx->setCheckState(active?Qt::Checked:Qt::Unchecked);
    enableWidgets();
  }
}

void InfoWidgetGeo::changeActiveGeo(bool enable){
  if(geometry!=NULL){
    int geo=geoBx->currentIndex();
    if(geo<nbBoxes)                       geometry->boxes[geo].active=enable;
    else if((geo-=nbBoxes)<nbCylinders)   geometry->cylinders[geo].active=enable;
    else if((geo-=nbCylinders)<nbSpheres) geometry->spheres[geo].active=enable;
    else if((geo-=nbSpheres)<nbLines)     geometry->lines[geo].active=enable;
    else if((geo-=nbLines)<nbMeshes)      geometry->meshes[geo].active=enable;
    else return; //this should never happen...
    enableWidgets();
    emit geometryChanged();
  }
}

void InfoWidgetGeo::changeColor(){
  int col[4];
  int geo;
  if(geometry!=NULL){
    geo=geoBx->currentIndex();
    if(geo<nbBoxes)
      memcpy(col,geometry->boxes[geo].color,4*sizeof(int));
    else if((geo-=nbBoxes)<nbCylinders)
      memcpy(col,geometry->cylinders[geo].color,4*sizeof(int));
    else if((geo-=nbCylinders)<nbSpheres)
      memcpy(col,geometry->spheres[geo].color,4*sizeof(int));
    else if((geo-=nbSpheres)<nbLines)
      memcpy(col,geometry->lines[geo].color,4*sizeof(int));
    else if((geo-=nbLines)<nbMeshes)
      memcpy(col,geometry->meshes[geo].color,4*sizeof(int));
    else return; //this should never happen...
    QColor newColor=QColorDialog::getColor(QColor(col[0],col[1],col[2],col[3]));
    if(newColor.isValid()){
      MyColor newMyColor(newColor);
      newMyColor.setAlpha(255-transpSl->value());
      selColorBt->setIcon(newMyColor.getIcon(20,20));
      newMyColor().getRgb(col,col+1,col+2,col+3);
      geo=geoBx->currentIndex();
      if(geo<nbBoxes)
        memcpy(geometry->boxes[geo].color,col,4*sizeof(int));
      else if((geo-=nbBoxes)<nbCylinders)
        memcpy(geometry->cylinders[geo].color,col,4*sizeof(int));
      else if((geo-=nbCylinders)<nbSpheres)
        memcpy(geometry->spheres[geo].color,col,4*sizeof(int));
      else if((geo-=nbSpheres)<nbLines)
        memcpy(geometry->lines[geo].color,col,4*sizeof(int));
      else if((geo-=nbLines)<nbMeshes)
        memcpy(geometry->meshes[geo].color,col,4*sizeof(int));
      else return; //this should never happen...
      emit geometryChanged();
    }
  }
}

void InfoWidgetGeo::changeTransparency(){
  int col[4];
  int geo;
  if(geometry!=NULL){
    geo=geoBx->currentIndex();
    if(geo<nbBoxes)
      memcpy(col,geometry->boxes[geo].color,4*sizeof(int));
    else if((geo-=nbBoxes)<nbCylinders)
      memcpy(col,geometry->cylinders[geo].color,4*sizeof(int));
    else if((geo-=nbCylinders)<nbSpheres)
      memcpy(col,geometry->spheres[geo].color,4*sizeof(int));
    else if((geo-=nbSpheres)<nbLines)
      memcpy(col,geometry->lines[geo].color,4*sizeof(int));
    else if((geo-=nbLines)<nbMeshes)
      memcpy(col,geometry->meshes[geo].color,4*sizeof(int));
    else return; //this should never happen...
    if(col[3]!=255-transpSl->value()){
      col[3]=255-transpSl->value();
      geo=geoBx->currentIndex();
      if(geo<nbBoxes)
        memcpy(geometry->boxes[geo].color,col,4*sizeof(int));
      else if((geo-=nbBoxes)<nbCylinders)
        memcpy(geometry->cylinders[geo].color,col,4*sizeof(int));
      else if((geo-=nbCylinders)<nbSpheres)
        memcpy(geometry->spheres[geo].color,col,4*sizeof(int));
      else if((geo-=nbSpheres)<nbLines)
        memcpy(geometry->lines[geo].color,col,4*sizeof(int));
      else if((geo-=nbLines)<nbMeshes)
        memcpy(geometry->meshes[geo].color,col,4*sizeof(int));
      else return; //this should never happen...
      emit geometryChanged();
    }
  }
}
