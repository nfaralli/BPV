#ifndef __COLORDIALOG_H__
#define __COLORDIALOG_H__

#include <QDialog>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include "particles.h"
#include "mycolor.h"

class ColorDialog : public QDialog{
	
	Q_OBJECT
	
public:
	ColorDialog(QWidget *parent=0);
	static MyColor getColor(Particles *particles, MyColor *color=0);
  
private:
	MyColor color();
	void    setColor(MyColor *c);
	void    setParticles(Particles *par){particles=par;}
private slots:
	void    changeColor();
	void    checkValues();
	void    resetMinMax();
  
private:
	Particles				*particles;
  QColor          cstColor;
	QRadioButton 		*cstColRb;
	QRadioButton 		*varColRb;
	QPushButton			*cstColBt;
	QPushButton 		*resetBt;
	QComboBox   		*selVarCb;
	QLineEdit				*minLe;
	QLineEdit				*maxLe;
	QCheckBox				*expCb;
	QCheckBox				*greyCb;
  QDialogButtonBox *buttonBox;
};

#endif
