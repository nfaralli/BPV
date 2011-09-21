#include "geometry.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>

static QString      getString (QString str, QString *pstr, bool *ok);
static int          getInt    (QString str, QString *pstr, bool *ok);
static float        getFloat  (QString str, QString *pstr, bool *ok);
static QList<float> get3F     (QString str, QString *pstr, bool *ok);
static QList<int>   get3I     (QString str, QString *pstr, bool *ok);
static bool         getBool   (QString str, QString *pstr, bool *ok);
static QList<float> getMatrix3(QString str, QString *pstr, bool *ok);
static QList<int>   getColor  (QString str, QString *pstr, bool *ok);
static QList<PointF3D> getArray3F(QString str, QString *pstr, bool *ok);
static QList<PointI3D> getArray3I(QString str, QString *pstr, bool *ok);

static MyBox      getMyBox     (QString &data);
static MyCylinder getMyCylinder(QString &data);
static MySphere   getMySphere  (QString &data);
static MyLines    getMyLines   (QString &data);
static MyMesh     getMyMesh    (QString &data);

static void setNames(MyGeometry*);

MyGeometry *loadGeometry(QString &fileName){
	QTextStream   line;
  QString       tmp;
	QString       objectType;
  QFile         file(fileName);
	
  if (!file.open(QFile::ReadOnly | QFile::Text))
    return NULL;
  if(file.size()==0)
    return NULL;
	MyGeometry *geometry=new MyGeometry;
	QTextStream data(&file);
	while(!data.atEnd()){
		tmp=data.readLine();
		tmp.simplified();
		if(!tmp.isEmpty() && !tmp.startsWith("--")){
			line.setString(&tmp);
			line.seek(0);
			line >> objectType;
			if(objectType=="box")
				geometry->boxes.append(getMyBox(tmp));
			else if(objectType=="cylinder")
				geometry->cylinders.append(getMyCylinder(tmp));
			else if(objectType=="sphere")
				geometry->spheres.append(getMySphere(tmp));
			else if(objectType=="line")
				geometry->lines.append(getMyLines(tmp));
      else if(objectType=="mesh")
        geometry->meshes.append(getMyMesh(tmp));
			else{
				QMessageBox::warning(NULL,QObject::tr("Load Geometry"),QObject::tr("Wrong object type."));
				exit(1);
			}
		}
	}
  geometry->nbObj =geometry->boxes.size();
  geometry->nbObj+=geometry->cylinders.size();
  geometry->nbObj+=geometry->spheres.size();
  geometry->nbObj+=geometry->lines.size();
  geometry->nbObj+=geometry->meshes.size();
	setNames(geometry);
	return geometry;
}

void freeGeometry(MyGeometry *geometry){
  int i;
	if(geometry==NULL)
		return;
	geometry->boxes.clear();
	geometry->cylinders.clear();
	geometry->spheres.clear();
	for(i=0;i<geometry->lines.size();i++)
		geometry->lines[i].points.clear();
	geometry->lines.clear();
  for(i=0;i<geometry->meshes.size();i++){
    geometry->meshes[i].vertices.clear();
    geometry->meshes[i].faces.clear();
  }
  geometry->meshes.clear();
  geometry->nbObj=0;
}

/*check objects name, and set a new one if doesn't exist.*/
static void setNames(MyGeometry *geometry){
	int i,j,index;
	QString name;
	
	if(geometry!=NULL){
		for(i=0,index=1;i<geometry->boxes.size();i++){
			if(geometry->boxes[i].name.isEmpty()){
				name=index<100?QString("Box%1").arg(index,2,10,QChar('0')):QString("Box%1").arg(index);
				for(j=0;j<geometry->boxes.size();j++){
					if(geometry->boxes[j].name==name){
						name=++index<100?QString("Box%1").arg(index,2,10,QChar('0')):QString("Box%1").arg(index);
						j=0;
					}
				}
				geometry->boxes[i].name=name;
			}
		}
		for(i=0,index=1;i<geometry->cylinders.size();i++){
			if(geometry->cylinders[i].name.isEmpty()){
				name=index<100?QString("Cylinder%1").arg(index,2,10,QChar('0')):QString("Cylinder%1").arg(index);
				for(j=0;j<geometry->cylinders.size();j++){
					if(geometry->cylinders[j].name==name){
						name=++index<100?QString("Cylinder%1").arg(index,2,10,QChar('0')):QString("Cylinder%1").arg(index);
						j=0;
					}
				}
				geometry->cylinders[i].name=name;
			}
		}
		for(i=0,index=1;i<geometry->spheres.size();i++){
			if(geometry->spheres[i].name.isEmpty()){
				name=index<100?QString("Sphere%1").arg(index,2,10,QChar('0')):QString("Sphere%1").arg(index);
				for(j=0;j<geometry->spheres.size();j++){
					if(geometry->spheres[j].name==name){
						name=++index<100?QString("Sphere%1").arg(index,2,10,QChar('0')):QString("Sphere%1").arg(index);
						j=0;
					}
				}
				geometry->spheres[i].name=name;
			}
		}
		for(i=0,index=1;i<geometry->lines.size();i++){
			if(geometry->lines[i].name.isEmpty()){
				name=index<100?QString("Line%1").arg(index,2,10,QChar('0')):QString("Line%1").arg(index);
				for(j=0;j<geometry->lines.size();j++){
					if(geometry->lines[j].name==name){
						name=++index<100?QString("Line%1").arg(index,2,10,QChar('0')):QString("Line%1").arg(index);
						j=0;
					}
				}
				geometry->lines[i].name=name;
			}
		}
		for(i=0,index=1;i<geometry->meshes.size();i++){
			if(geometry->meshes[i].name.isEmpty()){
				name=index<100?QString("Mesh%1").arg(index,2,10,QChar('0')):QString("Mesh%1").arg(index);
				for(j=0;j<geometry->meshes.size();j++){
					if(geometry->meshes[j].name==name){
						name=++index<100?QString("Mesh%1").arg(index,2,10,QChar('0')):QString("Mesh%1").arg(index);
						j=0;
					}
				}
				geometry->meshes[i].name=name;
			}
		}
	}
}

/*reads a string from str. the string must be between ""*/
static QString getString(QString str, QString *pstr, bool *ok){
	QString out;
	int     iend=-1;
	QString substr;
	
	str=str.trimmed();
	if(str.indexOf("\"")!=0 || (iend=str.indexOf("\"",1))==-1){
		*ok=false;
		return "";
	}
	out=str.mid(1,iend-1);
	if(pstr!=NULL)
		*pstr=str.mid(iend+1).trimmed();
	*ok=true;
	return out;
}
/*reads a boolean from str. boolean must be yes or no*/
static bool    getBool  (QString str, QString *pstr, bool *ok){
	bool    out=false;
	int     iend;
	QString substr;
	
	str=str.trimmed();
	iend=str.indexOf(" ");
	if(iend==-1) substr=str.mid(0);
	else         substr=str.mid(0,iend);
	substr=substr.toLower();
	*ok=true;
	if(substr=="yes")     out=true;
	else if(substr=="no") out=false;
	else *ok=false;
	if(pstr!=NULL && *ok){
		if(iend==-1) *pstr="";
		else   		   *pstr=str.mid(iend).trimmed();
	}
	return out;	
}
/*reads an integer from str*/
static int getInt(QString str, QString *pstr, bool *ok){
	int     out=0;
	int     iend;
	QString substr;
	
	str=str.trimmed();
	iend=str.indexOf(" ");
	if(iend==-1) substr=str.mid(0);
	else         substr=str.mid(0,iend);
	out=substr.toInt(ok);
	if(pstr!=NULL && *ok){
		if(iend==-1) *pstr="";
		else   		   *pstr=str.mid(iend).trimmed();
	}
	return out;
}
/*reads a float from str*/
static float  getFloat(QString str, QString *pstr, bool *ok){
	float   out=0;
	int     iend;
	QString substr;
	
	str=str.trimmed();
	iend=str.indexOf(" ");
	if(iend==-1) substr=str.mid(0);
	else         substr=str.mid(0,iend);
	out=substr.toFloat(ok);
	if(pstr!=NULL && *ok){
		if(iend==-1) *pstr="";
		else   		   *pstr=str.mid(iend).trimmed();
	}
	return out;
}
/*reads 3 floats form str. syntax: str=[float1,float2,float3]*/
static QList<float> get3F(QString str, QString *pstr, bool *ok){
	QList<float> out;
	int          iend=-1;
	QString      substr,floatstr;
	bool         subok;
	
	str=str.trimmed();
	if(str.indexOf("[")!=0 || (iend=str.indexOf("]",1))==-1){
		*ok=false;
		return out;
	}
	substr=str.mid(1,iend-1);
	for(int k=0;k<3;k++){
		if(k<2){
			if(substr.indexOf(",")==-1)
				subok=false;
			else{
				floatstr=substr.mid(0,substr.indexOf(","));
				out.append(getFloat(floatstr, NULL, &subok));
				substr=substr.mid(substr.indexOf(",")+1);
			}
		}
		else
			out.append(getFloat(substr, NULL, &subok));
		if(!subok){
			*ok=false;
			out.clear();
			return out;
		}
	}
	if(pstr!=NULL && *ok)
		*pstr=str.mid(iend+1).trimmed();
	*ok=true;
	return out;
}
/*reads 3 integers form str. syntax: str=[int1,int2,int3]*/
static QList<int> get3I(QString str, QString *pstr, bool *ok){
	QList<int>   out;
	int          iend=-1;
	QString      substr,intstr;
	bool         subok;
	
	str=str.trimmed();
	if(str.indexOf("[")!=0 || (iend=str.indexOf("]",1))==-1){
		*ok=false;
		return out;
	}
	substr=str.mid(1,iend-1);
	for(int k=0;k<3;k++){
		if(k<2){
			if(substr.indexOf(",")==-1)
				subok=false;
			else{
				intstr=substr.mid(0,substr.indexOf(","));
				out.append(getInt(intstr, NULL, &subok));
				substr=substr.mid(substr.indexOf(",")+1);
			}
		}
		else
			out.append(getInt(substr, NULL, &subok));
		if(!subok){
			*ok=false;
			out.clear();
			return out;
		}
	}
	if(pstr!=NULL && *ok)
		*pstr=str.mid(iend+1).trimmed();
	*ok=true;
	return out;
}

/*reads a transformation matrix form str.*/
static QList<float> getMatrix3(QString str, QString *pstr, bool *ok){
  QList<float> out;
  QList<float> floatList;
  
  for(int i=0;i<15;i++)
    out << 0;
  out << 1;
  str=str.trimmed();
  if(str.startsWith("matrix3")){
    floatList=get3F(str.mid(7),&str,ok);
		if(*ok){
      out[0]=floatList[0];
      out[4]=floatList[1];
      out[8]=floatList[2];
      floatList.clear();
      floatList=get3F(str,&str,ok);
		}
		if(*ok){
      out[1]=floatList[0];
      out[5]=floatList[1];
      out[9]=floatList[2];
      floatList.clear();
      floatList=get3F(str,&str,ok);
		}
		if(*ok){
      out[2]=floatList[0];
      out[6]=floatList[1];
      out[10]=floatList[2];
      floatList.clear();
			floatList=get3F(str,&str,ok);
		}
    if(*ok){
      out[12]=floatList[0];
      out[13]=floatList[1];
      out[14]=floatList[2];
      floatList.clear();
    }
  }
  else
    *ok=false;
  if(!*ok){
    out.clear();
    return out;
  }
	if(pstr!=NULL)
		*pstr=str.trimmed();
  return out;
}

/*reads a color statement*/
static QList<int> getColor(QString str, QString *pstr, bool *ok){
  QList<int> out;
  int k,alpha;
  
  str=str.trimmed();
  if(str.startsWith("color")){
    str=str.mid(5);
    for(k=0;k<3;k++){
      out << getInt(str,&str,ok);
      if(!*ok) break;
    }
    if(k==3){
      alpha=getInt(str,&str,ok);
      out << (*ok?alpha:255);
      *ok=true;
    }
  }
  else
    *ok=false;
  if(!*ok){
    out.clear();
    return out;
  }
  if(pstr!=NULL)
    *pstr=str.trimmed();
  return out;
}
/*reads an array of 3 floats. syntax: str=#([f1,f2,f3],[f1,f2,f3],...)*/
static QList<PointF3D> getArray3F(QString str, QString *pstr, bool *ok){
  QList<PointF3D> out;
  QList<float>    floatList;
  PointF3D point;
  QString  substr;
  int      iend=0;
  
  if(str.startsWith("#(")){
    iend=str.indexOf(")");
    if(iend!=-1){
      substr=str.mid(2,iend-2).trimmed();
      while(!substr.isEmpty()){
        if(out.size()>0){
          if(!substr.startsWith(",")){
            *ok=false;
            break;
          }
          substr=substr.mid(1).trimmed();
        }
        floatList=get3F(substr,&substr,ok);
        if(!*ok) break;
        point.x=floatList[0];
        point.y=floatList[1];
        point.z=floatList[2];
        out << point;
      }
    }
    else
      *ok=false;
  }
  else
    *ok=false;
  if(!*ok){
    out.clear();
    return out;
  }
  if(pstr!=NULL)
    *pstr=str.mid(iend+1).trimmed();
  return out;
}
/*reads an array of 3 integers. syntax: str=#([i1,i2,i3],[i1,i2,i3],...)*/
static QList<PointI3D> getArray3I(QString str, QString *pstr, bool *ok){
  QList<PointI3D> out;
  QList<int>      intList;
  PointI3D point;
  QString  substr;
  int      iend=0;
  
  if(str.startsWith("#(")){
    iend=str.indexOf(")");
    if(iend!=-1){
      substr=str.mid(2,iend-2).trimmed();
      while(!substr.isEmpty()){
        if(out.size()>0){
          if(!substr.startsWith(",")){
            *ok=false;
            break;
          }
          substr=substr.mid(1).trimmed();
        }
        intList=get3I(substr,&substr,ok);
        if(!*ok) break;
        point.i=intList[0]-1;
        point.j=intList[1]-1;
        point.k=intList[2]-1;
        out << point;
      }
    }
    else
      *ok=false;
  }
  else
    *ok=false;
  if(!*ok){
    out.clear();
    return out;
  }
  if(pstr!=NULL)
    *pstr=str.mid(iend+1).trimmed();
  return out;
}

static MyBox getMyBox(QString &data){
	MyBox        box;
	QString      substr,colorstr;
	QList<float> floatList;
	QList<int>   intList;
	bool         ok=true;
	bool         posFound=false;
	int          k;
	
	data=data.simplified();
	if(!data.startsWith("box ")){
		QMessageBox::warning(NULL,QObject::tr("Load Box"),QObject::tr("Syntax Error."));
    exit(1);
	}
	box.lengthsegs=1;
	box.widthsegs=1;
	box.heightsegs=1;
	box.length=0;
	box.width=0;
	box.height=0;
	box.pos[0]=box.pos[1]=box.pos[2]=0;
	box.color[0]=(int)(rand()*256./((double)RAND_MAX+1));
	box.color[1]=(int)(rand()*256./((double)RAND_MAX+1));
	box.color[2]=(int)(rand()*256./((double)RAND_MAX+1));
	box.color[3]=255;
  for(k=0;k<16;k++) box.transform[k]=0;
  box.transform[0]=box.transform[5]=box.transform[10]=box.transform[15]=1;
	substr=data.mid(data.indexOf(" ")+1);
	while(!substr.isEmpty() && !substr.startsWith("--")){
		if(substr.startsWith("name:"))
			box.name=getString(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("lengthsegs:"))
			box.lengthsegs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("widthsegs:"))
			box.widthsegs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("heightsegs:"))
			box.heightsegs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("length:"))
			box.length=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("width:"))
			box.width=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("height:"))
			box.height=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("pos:")){
			floatList=get3F(substr.mid(substr.indexOf(":")+1),&substr,&ok);
			if(ok){
				for(k=0;k<3;k++)
					box.pos[k]=box.transform[12+k]=floatList[k];
				posFound=true;
			}
		}
		else if(substr.startsWith("wirecolor:")){
      substr=substr.mid(substr.indexOf(":")+1).trimmed();
      if(substr.startsWith("(") && substr.indexOf(")")>0){
        colorstr=substr.mid(1,substr.indexOf(")")-1).trimmed();
				if(colorstr.startsWith("color")){
          intList=getColor(colorstr,NULL,&ok);
          if(ok)
            for(k=0;k<4;k++)
              box.color[k]=intList[k];
				}
				else
					ok=false;
        if(ok)
          substr=substr.mid(substr.indexOf(")")+1).trimmed();
      }
			else
				ok=false;
		}
		else if(substr.startsWith("transform:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			if(substr.startsWith("(")){
				substr=substr.mid(substr.indexOf("(")+1).trimmed();
				if(substr.startsWith("matrix3")){
          floatList=getMatrix3(substr,&substr,&ok);
          if(ok){
            for(k=0;k<12;k++)
              box.transform[k]=floatList[k];
						if(!posFound)
							for(k=0;k<3;k++)
								box.pos[k]=box.transform[12+k]=floatList[12+k];
					}
				}
				else
					ok=false;
        if(ok){
          if(!substr.startsWith(")"))
            ok=false;
          else
            substr=substr.mid(substr.indexOf(")")+1).trimmed();
        }
			}
			else
				ok=false;
		}
		else{
			QMessageBox::warning(NULL,QObject::tr("Load Box"),QObject::tr("Syntax Error at:\n%1").arg(substr));
			exit(1);
		}
		if(ok==false){
			QMessageBox::warning(NULL,QObject::tr("Load Box"),QObject::tr("Syntax Error."));
			exit(1);
		}
	}
	ok=true;
	if(box.lengthsegs<1) ok=false;
	if(box.widthsegs<1)  ok=false;
	if(box.heightsegs<1) ok=false;
	if(box.length<0)     ok=false;
	if(box.width<0)      ok=false;
	if(box.height<0)     ok=false;
	for(k=0;k<4;k++) if(box.color[k]<0 || box.color[k]>255) ok=false;
	if(!ok){
		QMessageBox::warning(NULL,QObject::tr("Load Box"),QObject::tr("Wrong values."));
		exit(1);
	}
	box.active=true;
	return box;
}

static MyCylinder getMyCylinder(QString &data){
	MyCylinder   cylinder;
	QString      substr,colorstr;
	QList<float> floatList;
	QList<int>   intList;
	bool         ok=true;
	bool         posFound=false;
	int          k;
	
	data=data.simplified();
	if(!data.startsWith("cylinder ")){
		QMessageBox::warning(NULL,QObject::tr("Load Cylinder"),QObject::tr("Syntax Error."));
    exit(1);
	}
	cylinder.capsegs=1;
	cylinder.heightsegs=1;
	cylinder.sides=18;
	cylinder.radius=0;
	cylinder.height=0;
	cylinder.pos[0]=cylinder.pos[1]=cylinder.pos[2]=0;
	cylinder.color[0]=(int)(rand()*256./((double)RAND_MAX+1));
	cylinder.color[1]=(int)(rand()*256./((double)RAND_MAX+1));
	cylinder.color[2]=(int)(rand()*256./((double)RAND_MAX+1));
	cylinder.color[3]=255;
  for(k=0;k<16;k++) cylinder.transform[k]=0;
  cylinder.transform[0]=cylinder.transform[5]=cylinder.transform[10]=cylinder.transform[15]=1;
	substr=data.mid(data.indexOf(" ")+1);
	while(!substr.isEmpty() && !substr.startsWith("--")){
		if(substr.startsWith("name:"))
			cylinder.name=getString(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("capsegs:"))
			cylinder.capsegs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("heightsegs:"))
			cylinder.heightsegs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("sides:"))
			cylinder.sides=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("radius:"))
			cylinder.radius=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("height:"))
			cylinder.height=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("pos:")){
			floatList=get3F(substr.mid(substr.indexOf(":")+1),&substr,&ok);
			if(ok){
				for(k=0;k<3;k++)
					cylinder.pos[k]=cylinder.transform[12+k]=floatList[k];
				posFound=true;
			}
		}
		else if(substr.startsWith("wirecolor:")){
      substr=substr.mid(substr.indexOf(":")+1).trimmed();
      if(substr.startsWith("(") && substr.indexOf(")")>0){
        colorstr=substr.mid(1,substr.indexOf(")")-1).trimmed();
				if(colorstr.startsWith("color")){
          intList=getColor(colorstr,NULL,&ok);
          if(ok)
            for(k=0;k<4;k++)
              cylinder.color[k]=intList[k];
				}
				else
					ok=false;
        if(ok)
          substr=substr.mid(substr.indexOf(")")+1).trimmed();
      }
			else
				ok=false;
		}
		else if(substr.startsWith("transform:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			if(substr.startsWith("(")){
				substr=substr.mid(substr.indexOf("(")+1).trimmed();
				if(substr.startsWith("matrix3")){
          floatList=getMatrix3(substr,&substr,&ok);
          if(ok){
            for(k=0;k<12;k++)
              cylinder.transform[k]=floatList[k];
						if(!posFound)
							for(k=0;k<3;k++)
								cylinder.pos[k]=cylinder.transform[12+k]=floatList[12+k];
					}
				}
				else
					ok=false;
        if(ok){
          if(!substr.startsWith(")"))
            ok=false;
          else
            substr=substr.mid(substr.indexOf(")")+1).trimmed();
        }
			}
			else
				ok=false;
		}
		else{
			QMessageBox::warning(NULL,QObject::tr("Load Cylinder"),QObject::tr("Syntax Error at:\n%1").arg(substr));
			exit(1);
		}
		if(ok==false){
			QMessageBox::warning(NULL,QObject::tr("Load Cylinder"),QObject::tr("Syntax Error."));
			exit(1);
		}
	}
	ok=true;
	if(cylinder.capsegs<1)    ok=false;
	if(cylinder.heightsegs<1) ok=false;
	if(cylinder.sides<3)      ok=false;
	if(cylinder.radius<0)     ok=false;
	if(cylinder.height<0)     ok=false;
	for(k=0;k<4;k++) if(cylinder.color[k]<0 || cylinder.color[k]>255) ok=false;
	if(!ok){
		QMessageBox::warning(NULL,QObject::tr("Load Cylinder"),QObject::tr("Wrong values."));
		exit(1);
	}
	cylinder.active=true;
	return cylinder;
}

static MySphere getMySphere(QString &data){
	MySphere     sphere;
	QString      substr,colorstr;
	QList<float> floatList;
	QList<int>   intList;
	bool         ok=true;
	bool         posFound=false;
	int          k;
	
	data=data.simplified();
	if(!data.startsWith("sphere ")){
		QMessageBox::warning(NULL,QObject::tr("Load Sphere"),QObject::tr("Syntax Error."));
    exit(1);
	}
	sphere.segs=8;
	sphere.radius=0;
	sphere.pos[0]=sphere.pos[1]=sphere.pos[2]=0;
	sphere.color[0]=(int)(rand()*256./((double)RAND_MAX+1));
	sphere.color[1]=(int)(rand()*256./((double)RAND_MAX+1));
	sphere.color[2]=(int)(rand()*256./((double)RAND_MAX+1));
	sphere.color[3]=255;
  for(k=0;k<16;k++) sphere.transform[k]=0;
  sphere.transform[0]=sphere.transform[5]=sphere.transform[10]=sphere.transform[15]=1;
	substr=data.mid(data.indexOf(" ")+1);
	while(!substr.isEmpty() && !substr.startsWith("--")){
		if(substr.startsWith("name:"))
			sphere.name=getString(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("segs:"))
			sphere.segs=getInt(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("radius:"))
			sphere.radius=getFloat(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("pos:")){
			floatList=get3F(substr.mid(substr.indexOf(":")+1),&substr,&ok);
			if(ok){
				for(k=0;k<3;k++)
					sphere.pos[k]=sphere.transform[12+k]=floatList[k];
				posFound=true;
			}
		}
		else if(substr.startsWith("wirecolor:")){
      substr=substr.mid(substr.indexOf(":")+1).trimmed();
      if(substr.startsWith("(") && substr.indexOf(")")>0){
        colorstr=substr.mid(1,substr.indexOf(")")-1).trimmed();
				if(colorstr.startsWith("color")){
          intList=getColor(colorstr,NULL,&ok);
          if(ok)
            for(k=0;k<4;k++)
              sphere.color[k]=intList[k];
				}
				else
					ok=false;
        if(ok)
          substr=substr.mid(substr.indexOf(")")+1).trimmed();
      }
			else
				ok=false;
		}
		else if(substr.startsWith("transform:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			if(substr.startsWith("(")){
				substr=substr.mid(substr.indexOf("(")+1).trimmed();
				if(substr.startsWith("matrix3")){
          floatList=getMatrix3(substr,&substr,&ok);
          if(ok){
            for(k=0;k<12;k++)
              sphere.transform[k]=floatList[k];
						if(!posFound)
							for(k=0;k<3;k++)
								sphere.pos[k]=sphere.transform[12+k]=floatList[12+k];
					}
				}
				else
					ok=false;
        if(ok){
          if(!substr.startsWith(")"))
            ok=false;
          else
            substr=substr.mid(substr.indexOf(")")+1).trimmed();
        }
			}
			else
				ok=false;
		}
		else{
			QMessageBox::warning(NULL,QObject::tr("Load Sphere"),QObject::tr("Syntax Error at:\n%1").arg(substr));
			exit(1);
		}
		if(ok==false){
			QMessageBox::warning(NULL,QObject::tr("Load Sphere"),QObject::tr("Syntax Error."));
			exit(1);
		}
	}
	ok=true;
	if(sphere.segs<1)   ok=false;
	if(sphere.radius<0) ok=false;
	for(k=0;k<4;k++) if(sphere.color[k]<0 || sphere.color[k]>255) ok=false;
	if(!ok){
		QMessageBox::warning(NULL,QObject::tr("Load Sphere"),QObject::tr("Wrong values."));
		exit(1);
	}
	sphere.active=true;
	return sphere;
}

static MyLines getMyLines(QString &data){
	MyLines      lines;
	QString      substr,colorstr;
	QList<float> floatList;
	QList<int>   intList;
	PointF3D     point;
	bool         ok=true;
	int          k;
	
	data=data.simplified();
	if(!data.startsWith("line ")){
		QMessageBox::warning(NULL,QObject::tr("Load Lines"),QObject::tr("Syntax Error."));
    exit(1);
	}
	lines.nbpts=0;
	lines.color[0]=(int)(rand()*256./((double)RAND_MAX+1));
	lines.color[1]=(int)(rand()*256./((double)RAND_MAX+1));
	lines.color[2]=(int)(rand()*256./((double)RAND_MAX+1));
	lines.color[3]=255;
	lines.close=false;
  for(k=0;k<16;k++) lines.transform[k]=0;
  lines.transform[0]=lines.transform[5]=lines.transform[10]=lines.transform[15]=1;
	substr=data.mid(data.indexOf(" ")+1);
	while(!substr.isEmpty() && !substr.startsWith("--")){
		if(substr.startsWith("name:"))
			lines.name=getString(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		if(substr.startsWith("close:"))
			lines.close=getBool(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("points:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			while(substr.startsWith("[")){
				floatList=get3F(substr,&substr,&ok);
				if(ok){
					point.x=floatList[0];
					point.y=floatList[1];
					point.z=floatList[2];
					lines.points.append(point);
					lines.nbpts++;
				}
				else break;
			}
		}
		else if(substr.startsWith("wirecolor:")){
      substr=substr.mid(substr.indexOf(":")+1).trimmed();
      if(substr.startsWith("(") && substr.indexOf(")")>0){
        colorstr=substr.mid(1,substr.indexOf(")")-1).trimmed();
				if(colorstr.startsWith("color")){
          intList=getColor(colorstr,NULL,&ok);
          if(ok)
            for(k=0;k<4;k++)
              lines.color[k]=intList[k];
				}
				else
					ok=false;
        if(ok)
          substr=substr.mid(substr.indexOf(")")+1).trimmed();
      }
			else
				ok=false;
		}
		else if(substr.startsWith("transform:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			if(substr.startsWith("(")){
				substr=substr.mid(substr.indexOf("(")+1).trimmed();
				if(substr.startsWith("matrix3")){
          floatList=getMatrix3(substr,&substr,&ok);
          if(ok)
            for(k=0;k<16;k++)
              lines.transform[k]=floatList[k];
				}
				else
					ok=false;
        if(ok){
          if(!substr.startsWith(")"))
            ok=false;
          else
            substr=substr.mid(substr.indexOf(")")+1).trimmed();
        }
			}
			else
				ok=false;
		}
		else{
			QMessageBox::warning(NULL,QObject::tr("Load Lines"),QObject::tr("Syntax Error at:\n%1").arg(substr));
			exit(1);
		}
		if(ok==false){
			QMessageBox::warning(NULL,QObject::tr("Load Lines"),QObject::tr("Syntax Error."));
			exit(1);
		}
	}
	ok=true;
	if(lines.nbpts<2)  ok=false;
	for(k=0;k<4;k++) if(lines.color[k]<0 || lines.color[k]>255) ok=false;
	if(!ok){
		QMessageBox::warning(NULL,QObject::tr("Load Lines"),QObject::tr("Wrong values."));
		exit(1);
	}
	lines.active=true;
	return lines;
}

static MyMesh getMyMesh(QString &data){
  MyMesh       mesh;
	QString      substr,colorstr;
	QList<float> floatList;
	QList<int>   intList;
	bool         ok=true;
  int          k;

	data=data.simplified();
	if(!data.startsWith("mesh ")){
		QMessageBox::warning(NULL,QObject::tr("Load Mesh"),QObject::tr("Syntax Error."));
    exit(1);
	}
	mesh.color[0]=(int)(rand()*256./((double)RAND_MAX+1));
	mesh.color[1]=(int)(rand()*256./((double)RAND_MAX+1));
	mesh.color[2]=(int)(rand()*256./((double)RAND_MAX+1));
	mesh.color[3]=255;
  for(k=0;k<16;k++) mesh.transform[k]=0;
  mesh.transform[0]=mesh.transform[5]=mesh.transform[10]=mesh.transform[15]=1;
	substr=data.mid(data.indexOf(" ")+1);
	while(!substr.isEmpty() && !substr.startsWith("--")){
		if(substr.startsWith("name:"))
			mesh.name=getString(substr.mid(substr.indexOf(":")+1),&substr,&ok);
    else if(substr.startsWith("vertices:"))
      mesh.vertices<<getArray3F(substr.mid(substr.indexOf(":")+1),&substr,&ok);
    else if(substr.startsWith("faces:"))
      mesh.faces<<getArray3I(substr.mid(substr.indexOf(":")+1),&substr,&ok);
		else if(substr.startsWith("wirecolor:")){
      substr=substr.mid(substr.indexOf(":")+1).trimmed();
      if(substr.startsWith("(") && substr.indexOf(")")>0){
        colorstr=substr.mid(1,substr.indexOf(")")-1).trimmed();
				if(colorstr.startsWith("color")){
          intList=getColor(colorstr,NULL,&ok);
          if(ok)
            for(k=0;k<4;k++)
              mesh.color[k]=intList[k];
				}
				else
					ok=false;
        if(ok)
          substr=substr.mid(substr.indexOf(")")+1).trimmed();
      }
			else
				ok=false;
		}
		else if(substr.startsWith("transform:")){
			substr=substr.mid(substr.indexOf(":")+1).trimmed();
			if(substr.startsWith("(")){
				substr=substr.mid(substr.indexOf("(")+1).trimmed();
				if(substr.startsWith("matrix3")){
          floatList=getMatrix3(substr,&substr,&ok);
          if(ok)
            for(k=0;k<16;k++)
              mesh.transform[k]=floatList[k];
				}
				else
					ok=false;
        if(ok){
          if(!substr.startsWith(")"))
            ok=false;
          else
            substr=substr.mid(substr.indexOf(")")+1).trimmed();
        }
			}
			else
				ok=false;
		}
		else{
			QMessageBox::warning(NULL,QObject::tr("Load Meshes"),QObject::tr("Syntax Error at:\n%1").arg(substr));
			exit(1);
		}
		if(ok==false){
			QMessageBox::warning(NULL,QObject::tr("Load Meshes"),QObject::tr("Syntax Error."));
			exit(1);
		}
  }
	ok=true;
  if(mesh.vertices.size()<3) ok=false;
  if(mesh.faces.size()<1) ok=false;
  for(k=0;k<mesh.faces.size();k++){
    if(mesh.faces[k].i<0 || mesh.faces[k].i>=mesh.vertices.size()) ok=false;
    if(mesh.faces[k].j<0 || mesh.faces[k].j>=mesh.vertices.size()) ok=false;
    if(mesh.faces[k].k<0 || mesh.faces[k].k>=mesh.vertices.size()) ok=false;
  }
	for(k=0;k<4;k++) if(mesh.color[k]<0 || mesh.color[k]>255) ok=false;
	if(!ok){
		QMessageBox::warning(NULL,QObject::tr("Load Meshes"),QObject::tr("Wrong values."));
		exit(1);
	}
	mesh.active=true;
  return mesh;
}
