#include "infowidgetbgd.h"
#include <QPixmap>
#include <QColorDialog>
#include <QGridLayout>

InfoWidgetBgd::InfoWidgetBgd(QWidget *parent) : QGroupBox("Background",parent){

  colorLb=new QLabel("Color:");
  colorLb->setMinimumWidth(35);
  selColorBt=new QPushButton();
  selColorBt->setMinimumWidth(60);
  QObject::connect(selColorBt,SIGNAL(pressed()),this,SLOT(changeColor()));
  bgColor=MyColor(0,0,0);
  QPixmap colorPixmap(20,20);
  colorPixmap.fill(bgColor());
  QIcon colorIcon(colorPixmap);
  selColorBt->setIcon(colorIcon);

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(colorLb,0,0);
  layout->addWidget(selColorBt,0,1);
  layout->setColumnStretch(2,1);
  setLayout(layout);
}

void InfoWidgetBgd::changeColor(){
  QColor newColor = QColorDialog::getColor(bgColor());
  if(newColor.isValid()){
    bgColor=MyColor(newColor);
    QPixmap colorPixmap(20,20);
    colorPixmap.fill(bgColor());
    QIcon colorIcon(colorPixmap);
    selColorBt->setIcon(colorIcon);
    emit backgroundChanged(bgColor);
  }
}
