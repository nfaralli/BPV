#ifndef __INFOWIDGETBGD_H__
#define __INFOWIDGETBGD_H__

#include <QObject>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include "mycolor.h"

class InfoWidgetBgd : public QGroupBox{
	
	Q_OBJECT
	
public:
	InfoWidgetBgd(QWidget *parent=0);
	
signals:
	void backgroundChanged(MyColor);
  
private slots:
	void changeColor();
	
private:
  MyColor bgColor;
	QLabel  *colorLb;
	QPushButton *selColorBt;
};

#endif
