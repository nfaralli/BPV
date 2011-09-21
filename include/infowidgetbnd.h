#ifndef __INFOWIDGETBND_H__
#define __INFOWIDGETBND_H__

#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QObject>
#include "particles.h"

class InfoWidgetBnd : public QGroupBox{
	
	Q_OBJECT
	
public:
	InfoWidgetBnd(Particles *part=0,QWidget *parent=0);
	void loadParticles(Particles *part);
	
signals:
  void bondSpecChanged(BondSpec);
	
protected slots:
	void changeRadius();
	void changeSlices(int newValue);
	void setShowWires(bool enable);
	
private:
  BondSpec  bSpec;
	QCheckBox *wireBx;
	QLabel    *radiusLb;
	QLabel	  *slicesLb;
	QLineEdit *radiusLe;
	QSpinBox  *slicesBx;
};

#endif
