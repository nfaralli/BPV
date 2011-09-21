#include "trajectorydialog.h"
#include "colordialog.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

TrajectoryDialog::TrajectoryDialog(QWidget *parent) :
  QDialog(parent){
  setWindowTitle(tr("Trajectory"));

  minMaxParticles = NULL;
  animateCB = new QCheckBox("Animated");
  activeAllTraCB = new QCheckBox("All Trajectories");
  activeSingleTraCB = new QCheckBox;
  allTraColorLb = new QLabel("Color:");
  allTraColorPB = new QPushButton();
  allTraColorPB->setMinimumWidth(50);
  allTraLengthLb = new QLabel("Length:");
  allTraLengthSB = new QSpinBox;
  allTraLengthSB->setMinimumWidth(50);
  allTraLengthSB->setAlignment(Qt::AlignRight);
  selSingleTraCB = new QComboBox;
  for(int i = 1; i<NB_TRAJECTORIES; i++)
    selSingleTraCB->addItem(QString("Trajectory #%1").arg(i));
  indexparticleLb = new QLabel("Index:");
  indexParticleSB = new QSpinBox;
  indexParticleSB->setAlignment(Qt::AlignRight);
  indexParticleSB->setMinimumWidth(70);
  singleTraColorLb = new QLabel("Color:");
  singleTraColorPB = new QPushButton();
  singleTraLengthLb = new QLabel("Length:");
  singleTraLengthSB = new QSpinBox;
  singleTraLengthSB->setMinimumWidth(50);
  singleTraLengthSB->setAlignment(Qt::AlignRight);
  closeBB = new QDialogButtonBox(QDialogButtonBox::Close);

  connect(closeBB, SIGNAL(rejected()), this, SLOT(reject()));
  connect(animateCB, SIGNAL(clicked(bool)), this, SIGNAL(animTraChanged(bool)));
  connect(activeAllTraCB, SIGNAL(clicked(bool)), this, SLOT(changeAllTraBt(bool)));
  connect(allTraColorPB, SIGNAL(pressed()), this, SLOT(changeAllTraColor()));
  connect(allTraLengthSB, SIGNAL(valueChanged(int)), this, SLOT(changeAllTraLength(int)));
  connect(activeSingleTraCB, SIGNAL(clicked(bool)), this, SLOT(changeSingleTraBt(bool)));
  connect(selSingleTraCB, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSingleTraIndex(int)));
  connect(indexParticleSB, SIGNAL(valueChanged(int)), this, SLOT(changeParticleIndex(int)));
  connect(singleTraColorPB, SIGNAL(pressed()), this, SLOT(changeSingleTraColor()));
  connect(singleTraLengthSB, SIGNAL(valueChanged(int)), this, SLOT(changeSingleTraLength(int)));

  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->addWidget(animateCB);
  QGridLayout *gLayout = new QGridLayout;
  gLayout->addWidget(activeAllTraCB, 0, 0);
  gLayout->addWidget(allTraColorLb, 0, 2);
  gLayout->addWidget(allTraColorPB, 0, 3);
  gLayout->addWidget(allTraLengthLb, 1, 2);
  gLayout->addWidget(allTraLengthSB, 1, 3);
  gLayout->setColumnStretch(1, 1);
  vLayout->addLayout(gLayout);
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->addWidget(activeSingleTraCB);
  hLayout->addWidget(selSingleTraCB);
  hLayout->setStretchFactor(selSingleTraCB, 1);
  vLayout->addLayout(hLayout);
  gLayout = new QGridLayout;
  gLayout->addWidget(indexparticleLb, 0, 1);
  gLayout->addWidget(indexParticleSB, 0, 2);
  gLayout->addWidget(singleTraColorLb, 1, 1);
  gLayout->addWidget(singleTraColorPB, 1, 2);
  gLayout->addWidget(singleTraLengthLb, 2, 1);
  gLayout->addWidget(singleTraLengthSB, 2, 2);
  gLayout->setColumnStretch(0, 1);
  gLayout->setColumnStretch(3, 3);
  vLayout->addLayout(gLayout);
  vLayout->addWidget(closeBB);
  setLayout(vLayout);

  allowEmit = true;
  setDialog( NULL);
  allTraColorPB->setIcon(trajectory[0].color.getIcon(20, 20));
  singleTraColorPB->setIcon(trajectory[1].color.getIcon(20, 20));
  setFixedSize(sizeHint());
}

TrajectoryDialog::~TrajectoryDialog(){
  freeParticles(minMaxParticles);
}

void TrajectoryDialog::setDialog(Particles *par){
  int i;

  freeParticles(minMaxParticles);
  minMaxParticles = NULL;
  for(i = 0; i<NB_TRAJECTORIES; i++){
    trajectory[i].active = false;
    trajectory[i].single = (i==0 ? false : true);
    trajectory[i].index = 0;
    trajectory[i].nbPrevSteps = 5;
    trajectory[i].color = MyColor(0, 0, 255);
  }
  allowEmit = false;
  animateCB->setChecked(true);
  activeAllTraCB->setChecked(false);
  activeSingleTraCB->setChecked(false);
  allTraColorLb->setEnabled(false);
  allTraColorPB->setEnabled(false);
  allTraLengthLb->setEnabled(false);
  allTraLengthSB->setEnabled(false);
  indexparticleLb->setEnabled(false);
  indexParticleSB->setEnabled(false);
  singleTraColorLb->setEnabled(false);
  singleTraColorPB->setEnabled(false);
  singleTraLengthLb->setEnabled(false);
  singleTraLengthSB->setEnabled(false);
  if(par!=NULL){
    minMaxParticles = minMaxParticleCopy(par);
    if(par->nbParticles==0){
      allTraLengthSB->setRange(0, 0);
      allTraLengthSB->setValue(0);
      indexParticleSB->setRange(0, 0);
      indexParticleSB->setValue(0);
      singleTraLengthSB->setRange(0, 0);
      singleTraLengthSB->setValue(0);
      setEnabled(false);
    }else{
      allTraLengthSB->setRange(0, par->nbSteps-1);
      allTraLengthSB->setValue(5);
      indexParticleSB->setRange(1, par->nbParticles);
      indexParticleSB->setValue(1);
      singleTraLengthSB->setRange(0, par->nbSteps-1);
      singleTraLengthSB->setValue(5);
      setEnabled(true);
    }
  }else{
    allTraLengthSB->setRange(0, 0);
    allTraLengthSB->setValue(0);
    indexParticleSB->setRange(0, 0);
    indexParticleSB->setValue(0);
    singleTraLengthSB->setRange(0, 0);
    singleTraLengthSB->setValue(0);
    setEnabled(false);
  }
  allowEmit = true;
}

void TrajectoryDialog::changeAllTraBt(bool enable){
  allTraColorLb->setEnabled(enable);
  allTraColorPB->setEnabled(enable);
  allTraLengthLb->setEnabled(enable);
  allTraLengthSB->setEnabled(enable);
  activeSingleTraCB->setEnabled(!enable);
  selSingleTraCB->setEnabled(!enable);
  indexparticleLb->setEnabled(!enable&&activeSingleTraCB->isChecked());
  indexParticleSB->setEnabled(!enable&&activeSingleTraCB->isChecked());
  singleTraColorLb->setEnabled(!enable&&activeSingleTraCB->isChecked());
  singleTraColorPB->setEnabled(!enable&&activeSingleTraCB->isChecked());
  singleTraLengthLb->setEnabled(!enable&&activeSingleTraCB->isChecked());
  singleTraLengthSB->setEnabled(!enable&&activeSingleTraCB->isChecked());
  trajectory[0].active = enable;
  if(allowEmit) emit trajectoryChanged(0, trajectory[0]);
}

void TrajectoryDialog::changeAllTraColor(){
  if(minMaxParticles!=NULL){
    MyColor newColor = ColorDialog::getColor(minMaxParticles, &(trajectory[0].color));
    if(newColor.isValid()){
      trajectory[0].color = newColor;
      allTraColorPB->setIcon(newColor.getIcon(20, 20));
      if(allowEmit) emit trajectoryChanged(0, trajectory[0]);
    }
  }
}

void TrajectoryDialog::changeAllTraLength(int length){
  trajectory[0].nbPrevSteps=length;
  if(allowEmit) emit trajectoryChanged(0, trajectory[0]);
}

void TrajectoryDialog::changeSingleTraBt(bool enable){
  int traIndex = selSingleTraCB->currentIndex()+1;
  trajectory[traIndex].active = enable;
  indexparticleLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  indexParticleSB->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraColorLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraColorPB->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraLengthLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraLengthSB->setEnabled(enable&&!activeAllTraCB->isChecked());
  if(allowEmit) emit trajectoryChanged(traIndex, trajectory[traIndex]);
}

void TrajectoryDialog::changeSingleTraIndex(int index){
  bool enable;
  index++;
  enable = trajectory[index].active;
  activeSingleTraCB->setChecked(enable);
  indexParticleSB->setValue(trajectory[index].index+1);
  singleTraColorPB->setIcon(trajectory[index].color.getIcon(20, 20));
  indexparticleLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  indexParticleSB->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraColorLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraColorPB->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraLengthLb->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraLengthSB->setEnabled(enable&&!activeAllTraCB->isChecked());
  singleTraLengthSB->setValue(trajectory[index].nbPrevSteps);
}

void TrajectoryDialog::changeParticleIndex(int index){
  int traIndex = selSingleTraCB->currentIndex()+1;
  trajectory[traIndex].index = index-1;
  if(allowEmit) emit trajectoryChanged(traIndex, trajectory[traIndex]);
}

void TrajectoryDialog::changeSingleTraColor(){
  int traIndex = selSingleTraCB->currentIndex()+1;
  MyColor newColor = ColorDialog::getColor(minMaxParticles, &(trajectory[traIndex].color));
  if(newColor.isValid()){
    trajectory[traIndex].color = newColor;
    singleTraColorPB->setIcon(newColor.getIcon(20, 20));
    if(allowEmit) emit trajectoryChanged(traIndex, trajectory[traIndex]);
  }
}

void TrajectoryDialog::changeSingleTraLength(int length){
  int traIndex = selSingleTraCB->currentIndex()+1;
  trajectory[traIndex].nbPrevSteps=length;
  if(allowEmit) emit trajectoryChanged(traIndex, trajectory[traIndex]);
}
