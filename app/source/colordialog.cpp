#include "colordialog.h"

#include <math.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QIcon>
#include <QColorDialog>
#include <QDoubleValidator>
#include <QMessageBox>

ColorDialog::ColorDialog(QWidget *parent) : QDialog(parent){

  setWindowTitle(tr("Select Color"));
  particles=NULL;

  //create objects
  cstColRb=new QRadioButton(tr("Use Constant Color:"));
  varColRb=new QRadioButton(tr("Use Variable Color:"));
  QButtonGroup buttonGroup;
  buttonGroup.addButton(cstColRb);
  buttonGroup.addButton(varColRb);
  cstColRb->setChecked(true);
  cstColBt=new QPushButton;
  cstColBt->setMinimumWidth(50);
  resetBt=new QPushButton("Reset Range");
  QLabel *selVarLb=new QLabel("Select Variable:");
  QLabel *minLb=new QLabel("Min.");
  QLabel *maxLb=new QLabel("Max.");
  selVarCb=new QComboBox;
  selVarCb->setMinimumWidth(200);
  QDoubleValidator *vd=new QDoubleValidator(this);
  minLe=new QLineEdit;
  minLe->setValidator(vd);
  minLe->setText("0");
  minLe->setMinimumWidth(100);
  maxLe=new QLineEdit;
  maxLe->setValidator(vd);
  maxLe->setText("0");
  maxLe->setMinimumWidth(100);
  expCb=new QCheckBox("Exponential Distribution");
  greyCb=new QCheckBox("Greyscale");
  buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  //connect objects
  QObject::connect(cstColBt,SIGNAL(pressed()),this,SLOT(changeColor()));
  QObject::connect(buttonBox,SIGNAL(accepted()),this,SLOT(checkValues()));
  QObject::connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
  QObject::connect(resetBt,SIGNAL(pressed()),this,SLOT(resetMinMax()));

  //set layout
  QGridLayout *radioLayout=new QGridLayout;
  radioLayout->addWidget(cstColRb,0,0);
  radioLayout->addWidget(cstColBt,0,2);
  radioLayout->addWidget(varColRb,1,0);
  radioLayout->setColumnStretch(1,1);
  radioLayout->setColumnStretch(3,5);

  QHBoxLayout *selVarLayout=new QHBoxLayout;
  selVarLayout->addWidget(selVarLb);
  selVarLayout->addWidget(selVarCb);
  QGridLayout *minMaxLayout=new QGridLayout;
  minMaxLayout->addWidget(minLb,0,0);
  minMaxLayout->addWidget(minLe,0,1);
  minMaxLayout->addWidget(resetBt,0,3);
  minMaxLayout->addWidget(maxLb,1,0);
  minMaxLayout->addWidget(maxLe,1,1);
  minMaxLayout->setColumnStretch(2,1);
  minMaxLayout->setColumnStretch(4,4);
  QHBoxLayout *expGscLayout=new QHBoxLayout;
  expGscLayout->addWidget(expCb);
  expGscLayout->addSpacing(10);
  expGscLayout->addWidget(greyCb);
  expGscLayout->addStretch(1);
  QGridLayout *varColLayout=new QGridLayout;
  varColLayout->addLayout(selVarLayout,0,1);
  varColLayout->addLayout(minMaxLayout,1,1);
  varColLayout->addLayout(expGscLayout,2,1);
  varColLayout->setColumnStretch(0,1);

  QVBoxLayout  *vLayout=new QVBoxLayout;
  vLayout->addLayout(radioLayout);
  vLayout->addStretch(1);
  vLayout->addLayout(varColLayout);
  vLayout->addStretch(3);
  vLayout->addWidget(buttonBox);
  setLayout(vLayout);
  setFixedSize(sizeHint());
}

void ColorDialog::changeColor(){
  QColor newColor = QColorDialog::getColor(cstColor);
  if(newColor.isValid()){
    cstColor=newColor;
    QPixmap colorPixmap(20,20);
    colorPixmap.fill(newColor);
    QIcon colorIcon(colorPixmap);
    cstColBt->setIcon(colorIcon);
  }
}

void ColorDialog::setColor(MyColor *c){
  if(c!=NULL){
    cstColor=c->getColor();
    QColor tmpColor=cstColor;
    tmpColor.setAlpha(255);
    QPixmap colorPixmap(20,20);
    colorPixmap.fill(tmpColor);
    QIcon colorIcon(colorPixmap);
    cstColBt->setIcon(colorIcon);
    minLe->setText(QString("%1").arg(c->getMin()));
    maxLe->setText(QString("%1").arg(c->getMax()));
    expCb->setChecked(c->isLogScale());
    greyCb->setChecked(c->isGreyscale());
    if(c->isGradColorEnabled())
      varColRb->setChecked(true);
    else
      cstColRb->setChecked(true);
    selVarCb->setCurrentIndex(c->getVarIndex());
  }
  else{
    cstColor=QColor(0,0,0);
    QPixmap colorPixmap(20,20);
    colorPixmap.fill(c->getColor());
    QIcon colorIcon(colorPixmap);
    cstColBt->setIcon(colorIcon);
    minLe->setText(QString("0.0"));
    maxLe->setText(QString("0.0"));
    expCb->setChecked(false);
    greyCb->setChecked(false);
    cstColRb->setChecked(true);
    selVarCb->setCurrentIndex(0);
  }
}

MyColor ColorDialog::color(){
  MyColor result;

  result.setColor(cstColor);
  if(cstColRb->isChecked())
    result.setUseGradColor(false);
  else
    result.setUseGradColor(true);
  result.setVarIndex(selVarCb->currentIndex());
  result.setMinMax(minLe->text().toFloat(),maxLe->text().toFloat());
  result.setLogScale(expCb->isChecked());
  result.setGreyscale(greyCb->isChecked());

  return result;
}
void ColorDialog::checkValues(){
  float min,max;
  min=minLe->text().toFloat();
  max=maxLe->text().toFloat();
  if(min>max)
    QMessageBox::warning(NULL,QObject::tr("Select Color"),QObject::tr("must have min<=max"));
  else if(expCb->isChecked() && min<=0)
    QMessageBox::warning(NULL,QObject::tr("Select Color"),
                         QObject::tr("must have min>0 when using Exponential Distribution"));
  else
    accept();
}

void ColorDialog::resetMinMax(){
  if(particles!=NULL){
    int index=selVarCb->currentIndex()-1;
    float min,max,value;
    int i,j=0;
    if(index<0){
      min=max=particles->time[0];
      for(i=1;i<particles->nbSteps;i++){
        value=particles->time[i];
        min=value<min?value:min;
        max=value>max?value:max;
      }
    }
    else{
      for(i=0;i<particles->nbSteps;i++){
        for(j=0;j<particles->nbParticles;j++)
          if(particles->pPos[i][j][0]!=INFINITY)
            break;
        if(j<particles->nbParticles)
          break;
      }
      if(i<particles->nbSteps){
        min=max=particles->pPos[i][j][index];
        for(i=0;i<particles->nbSteps;i++){
          for(j=0;j<particles->nbParticles;j++){
            if(particles->pPos[i][j][0]!=INFINITY){
              value=particles->pPos[i][j][index];
              min=value<min?value:min;
              max=value>max?value:max;
            }
          }
        }
      }
      else
        min=max=0;
    }
    minLe->setText(QString("%1").arg(min));
    maxLe->setText(QString("%1").arg(max));
  }
}

MyColor ColorDialog::getColor(Particles *particles, MyColor *c){
  ColorDialog *cdlg=new ColorDialog;
  int i;
  
  cdlg->setParticles(particles);
  cdlg->selVarCb->addItem("time [s]");
  for(i=0;i<3+particles->nbVariables;i++)
    cdlg->selVarCb->addItem(particles->varName[i]);
  cdlg->setColor(c);
  int resultCode=cdlg->exec();
  MyColor result;
  result.setValid(false);
  if(resultCode==QDialog::Accepted)
    result=cdlg->color();
  delete cdlg;
  return result;
}
