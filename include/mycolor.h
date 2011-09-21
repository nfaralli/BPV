#ifndef __MYCOLOR_H__
#define __MYCOLOR_H__

#include <QColor>
#include <QIcon>
#include <math.h>

class MyColor
{
public:
  MyColor();
  MyColor(QColor c);
  MyColor(int r, int g, int b, int a=255);
	void setUseGradColor(bool enabled){useGradColor=enabled;}
	bool isGradColorEnabled(){return useGradColor;}
  void setVarIndex(int index){varIndex=index;}
  int  getVarIndex(){return varIndex;}
  void setAlpha(int a);
  int  alpha(){return alphaValue;}
  void setValid(bool v){valid=v;}
  bool isValid(){return valid;}
  QColor  getColor(){return color;}
	void    setColor(QColor c){color=c;}
  float   getMin(){return min;}
  float   getMax(){return max;}
	void    setMinMax(float theMin, float theMax);
  bool    isLogScale(){return logScale;}
	void    setLogScale(bool enabled){logScale=enabled;}
  bool    isGreyscale(){return blackWhite;}
	void		setGreyscale(bool enabled){blackWhite=enabled;}
	QIcon		getIcon(int w,int h);
  QColor& operator()(float v=0);
  
protected:
  QColor  color;
  QColor  gradColor;
	bool		useGradColor;
  int     varIndex;
  float   min,max;
  bool    logScale;
  bool    blackWhite;
  float   coefLin,coefExp;
  int     alphaValue;
  bool    valid;
};

#endif
