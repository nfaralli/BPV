#include "mycolor.h"
#include <QPixmap>
#include <QLinearGradient>
#include <QPainter>

MyColor::MyColor(){
  color=QColor(255,0,0);
  gradColor=QColor(255,0,0);
  alphaValue=255;
  varIndex=0;
  min=max=0;
  logScale=false;
  blackWhite=false;
  valid=true;
	useGradColor=false;
}

MyColor::MyColor(QColor c){
  color=c;
  gradColor=QColor(255,0,0);
  alphaValue=255;
  varIndex=0;
  min=max=0;
  logScale=false;
  blackWhite=false;
  valid=true;
	useGradColor=false;
}

MyColor::MyColor(int r, int g, int b, int a){
  color=QColor(r,g,b,a);
  gradColor=QColor(255,0,0);
  alphaValue=255;
  varIndex=0;
  min=max=0;
  logScale=false;
  blackWhite=false;
  valid=true;
	useGradColor=false;
}

void MyColor::setAlpha(int a){
	if(a>=0 && a<256)
	alphaValue=a;
	color.setAlpha(a);
}

void MyColor::setMinMax(float theMin, float theMax){
	if(theMin<=theMax){
		min=theMin;
		max=theMax;
		if(min<max){
			if(min>0) coefExp=1./(log(max)-log(min));
			coefLin=1./(max-min);
		}
	}
}

QColor& MyColor::operator()(float v){
  if(useGradColor){
    if(v>max)
      blackWhite?gradColor.setHsvF(0,0,1):gradColor.setHsvF(0,1,1);
    else if(v>min && min<max){
      if(logScale && min>0)
        blackWhite?gradColor.setHsvF(0,0,(log(v)-log(min))*coefExp):
          gradColor.setHsvF((log(max)-log(v))*coefExp*2./3.,1,1);
      else
        blackWhite?gradColor.setHsvF(0,0,(v-min)*coefLin):
          gradColor.setHsvF((max-v)*coefLin*2./3.,1,1);
    }
    else
      blackWhite?gradColor.setHsvF(0,0,0):gradColor.setHsvF(2./3.,1,1);
		gradColor.setAlpha(alphaValue);
    return gradColor;
  }
  else
    return color;
}

QIcon	MyColor::getIcon(int w,int h){
	QPixmap colorPixmap(w,h);
	if(useGradColor){
		QLinearGradient gradient(0,0,w,0);
		if(blackWhite){
			gradient.setColorAt(0,Qt::black);
			gradient.setColorAt(1,Qt::white);
		}
		else{
			gradient.setColorAt(0,Qt::blue);
			gradient.setColorAt(0.25,Qt::cyan);
			gradient.setColorAt(0.5,Qt::green);
			gradient.setColorAt(0.75,Qt::yellow);
			gradient.setColorAt(1,Qt::red);
		}
		QPainter painter(&colorPixmap);
		painter.fillRect(0, 0, w, h, gradient);
	}
	else{
		QColor tmpColor(color);
		tmpColor.setAlpha(255);
		colorPixmap.fill(tmpColor);
	}
	return QIcon(colorPixmap);
}
