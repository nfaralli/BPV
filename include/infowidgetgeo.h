#ifndef __INFOWIDGETGEO_H__
#define __INFOWIDGETGEO_H__

#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QSlider>
#include "geometry.h"

class InfoWidgetGeo : public QGroupBox{
  
	Q_OBJECT
	
public:
	InfoWidgetGeo(MyGeometry *geom=0, QWidget *parent=0);
	void loadGeometry(MyGeometry *geom);
	
signals:
	void geometryChanged();
	
private slots:
	void setGeo(int geo);
	void changeActiveGeo(bool enable);
	void changeColor();
	void changeTransparence();
	
private:
  void enableWidgets();
	
private:
	MyGeometry  *geometry;
  int         nbObj;
	int         nbBoxes;
	int         nbCylinders;
	int         nbSpheres;
	int         nbLines;
	int         nbMeshes;
	QLabel      *colorLb;
	QLabel      *transpLb;
	QCheckBox   *activeGeoBx;
	QComboBox   *geoBx;
	QPushButton *selColorBt;
	QSlider     *transpSl;
};

#endif
