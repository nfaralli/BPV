#include "infowidgetmsh.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

InfoWidgetMsh::InfoWidgetMsh(Particles *part,QWidget *parent) : QGroupBox("Meshes",parent){

    setCheckable(true);
    setChecked(false);
    setEnabled(false);

    nbMeshes = 0;
    mSpec = NULL;
    activeMeshBx = new QCheckBox();
    meshBx = new QComboBox;
    wireBx = new QCheckBox("Wired mesh");
    wireBt = new QPushButton("Apply to all");
    cullBackFaceBx = new QCheckBox("Cull back face");
    cullBackFaceBt = new QPushButton("Apply to all");

    QObject::connect(activeMeshBx,SIGNAL(clicked(bool)),this,SLOT(changeActiveMesh(bool)));
    QObject::connect(meshBx,SIGNAL(currentIndexChanged(int)),this,SLOT(setMesh(int)));
    QObject::connect(wireBx,SIGNAL(clicked(bool)),this,SLOT(changeShowWires(bool)));
    QObject::connect(wireBt,SIGNAL(pressed()),this,SLOT(changeAllShowWires()));
    QObject::connect(cullBackFaceBx,SIGNAL(clicked(bool)),this,SLOT(changeCullBackFace(bool)));
    QObject::connect(cullBackFaceBt,SIGNAL(pressed()),this,SLOT(changeAllCullBackFace()));

    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(activeMeshBx);
    hLayout->addWidget(meshBx);
    hLayout->setStretchFactor(meshBx,1);
    vLayout->addLayout(hLayout);
    QGridLayout *gLayout = new QGridLayout;
    gLayout->setColumnStretch(0,1);
    gLayout->addWidget(wireBx,0,0);
    gLayout->addWidget(wireBt,0,1);
    gLayout->addWidget(cullBackFaceBx,1,0);
    gLayout->addWidget(cullBackFaceBt,1,1);
    vLayout->addLayout(gLayout);
    setLayout(vLayout);
    loadParticles(part);
}

void InfoWidgetMsh::loadParticles(Particles *particles){
    int k;
    if(mSpec!=NULL)
      free(mSpec);
    mSpec=NULL;
    nbMeshes=0;
    if(particles!=NULL){
        nbMeshes=particles->nbMeshes;
        mSpec=(MeshSpec*)calloc(nbMeshes,sizeof(MeshSpec));
        memcpy(mSpec,particles->mSpec,nbMeshes*sizeof(MeshSpec));
        while(meshBx->count())
          meshBx->removeItem(meshBx->count()-1);
        for(k=0;k<nbMeshes;k++)
          meshBx->addItem(mSpec[k].name);
        meshBx->setCurrentIndex(0);
        wireBx->setChecked(mSpec[0].showWires);
        cullBackFaceBx->setChecked(mSpec[0].cullBackFace);
        setMesh(0);
        setChecked(true);
        setEnabled(true);
    }
    else {
        while(meshBx->count())
          meshBx->removeItem(meshBx->count()-1);
        setChecked(false);
        setEnabled(false);
    }
}

void InfoWidgetMsh::enableWidgets(){
  bool enableMesh,wireMesh;
  enableMesh=activeMeshBx->checkState()==Qt::Checked;
  wireMesh=mSpec[meshBx->currentIndex()].showWires;
  wireBx->setEnabled(enableMesh);
  wireBt->setEnabled(enableMesh);
  cullBackFaceBx->setEnabled(enableMesh&&!wireMesh);
  cullBackFaceBt->setEnabled(enableMesh&&!wireMesh);
}

void InfoWidgetMsh::setMesh(int meshIndex){
    if(meshIndex>=0 && meshIndex<nbMeshes){
        activeMeshBx->setCheckState(mSpec[meshIndex].active?Qt::Checked:Qt::Unchecked);
        wireBx->setCheckState(mSpec[meshIndex].showWires?Qt::Checked:Qt::Unchecked);
        cullBackFaceBx->setCheckState(mSpec[meshIndex].cullBackFace?Qt::Checked:Qt::Unchecked);
        enableWidgets();
    }
}

void InfoWidgetMsh::changeActiveMesh(bool enable){
    if(mSpec!=NULL){
        mSpec[meshBx->currentIndex()].active=enable;
        enableWidgets();
        emit meshSpecChanged(meshBx->currentIndex(),mSpec[meshBx->currentIndex()]);
    }
}

void InfoWidgetMsh::changeShowWires(bool enable){
    if(mSpec!=NULL){
        mSpec[meshBx->currentIndex()].showWires=enable;
        enableWidgets();
        emit meshSpecChanged(meshBx->currentIndex(),mSpec[meshBx->currentIndex()]);
    }
}

void InfoWidgetMsh::changeAllShowWires(){
    for(int i=0;i<nbMeshes;i++)
        mSpec[i].showWires=mSpec[meshBx->currentIndex()].showWires;
    emit allMeshSpecChanged(mSpec);
}

void InfoWidgetMsh::changeCullBackFace(bool enable){
    if(mSpec!=NULL){
        mSpec[meshBx->currentIndex()].cullBackFace=enable;
        enableWidgets();
        emit meshSpecChanged(meshBx->currentIndex(),mSpec[meshBx->currentIndex()]);
    }
}

void InfoWidgetMsh::changeAllCullBackFace(){
    for(int i=0;i<nbMeshes;i++)
        mSpec[i].cullBackFace=mSpec[meshBx->currentIndex()].cullBackFace;
    emit allMeshSpecChanged(mSpec);
}
