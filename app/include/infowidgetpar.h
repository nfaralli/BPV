#ifndef __INFOWIDGETPAR_H__
#define __INFOWIDGETPAR_H__

#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QObject>
#include "particles.h"

class InfoWidgetPar : public QGroupBox{

  Q_OBJECT

public:
  InfoWidgetPar(Particles *part=0,QWidget *parent=0);
  void loadParticles(Particles *part);

signals:
  void partSpecChanged(int,PartSpec);
  void allPartSpecChanged(PartSpec*);

private slots:
  void setType(int type);
  void changeActiveType(bool enable);
  void changeColor();
  void changeAllColor();
  void changeTransparency();
  void changeAllTransparency();
  void changeShowPoints(bool enable);
  void changeAllShowPoints();
  void changePtSize(int size);
  void changeAllPtSize();
  void changeRadius();
  void changeAllRadius();
  void changeSegs(int newValue);
  void changeAllSegs();

private:
  void enableWidgets();

private:
  Particles   *minMaxParticles;
  int         nbTypes;
  PartSpec    *pSpec;
  QCheckBox   *activeTypeBx;
  QCheckBox   *pointBx;
  QSpinBox    *ptSizeSb;
  QLabel      *ptSizeLb;
  QLabel      *colorLb;
  QLabel      *transpLb;
  QLabel      *radiusLb;
  QLabel      *segsLb;
  QComboBox   *typeBx;
  QPushButton *selColorBt;
  QSlider     *transpSl;
  QLineEdit   *radiusLe;
  QSpinBox    *segsBx;
  QPushButton *colorBt;
  QPushButton *pointBt;
  QPushButton *ptSizeBt;
  QPushButton *transpBt;
  QPushButton *radiusBt;
  QPushButton *segsBt;
};


#endif
