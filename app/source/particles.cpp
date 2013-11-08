#include "particles.h"
#include <stdio.h>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QObject>

/*reads a .par file
  this file is built as follow (byte=unsigned char):
  PAR     (3 bytes)
  nbTypes (1 byte)
  name    type 1  (NAME_LENGTH bytes)
  radius  type 1  (1 float)
  red     type 1  (1 byte)
  green   type 1  (1 byte)
  blue    type 1  (1 byte)
  name    type 2  (NAME_LENGTH bytes)
  radius  type 2  (1 float)
  etc.
  nbVariables (1 byte)
  name    x  axis    (NAME_LENGTH bytes)
  name    y  axis    (NAME_LENGTH bytes)
  name    z  axis    (NAME_LENGTH bytes)
  name    variable 1 (NAME_LENGTH bytes)
  name    variable 2 (NAME_LENGTH bytes)
  etc.
  nbParticles (1 unsigned int)
  type of particle 1  (1 byte)
  type of particle 2  (1 byte)
  etc.
  nbBonds       (1 unsigned int)
  bonds radius  (1 float)
  1st index of bond 1 (1 unsigned int)
  2nd index of bond 1 (1 unsigned int)
  1st index of bond 2 (1 unsigned int)
  etc.
  nbSteps       (1 unsigned int)
  time step 1   (1 float)
  x coordinate particle 1 step 1  (1 float)
  y coordinate particle 1 step 1  (1 float)
  z coordinate particle 1 step 1  (1 float)
  variable 1 particle 1 step 1    (1 float)
  variable 2 particle 1 step 1    (1 float)
  etc.
  x coordinate particle 2 step 1  (1 float)
  etc.
  time step 2   (1 float)
  etc.
*/
Particles *loadParticlesPAR(QString &fileName){
  Particles *particles=NULL;
  FILE    *file;
  size_t  nread;
  long    fpos;
  char    filetype[3];
  unsigned char nbTypes;
  unsigned char nbVariables;
  unsigned int  nbParticles;
  unsigned int  nbBonds;
  unsigned int  nbSteps;
  unsigned char r,g,b,type;
  unsigned int  index1,index2;
  float         t,x,y,z,var,radius,bRadius;
  int k,n,i;

  /*check if file is OK before setting particles*/
  file=fopen(fileName.toAscii().constData(),"rb");
  nread=fread(filetype,1,3,file);
  if(nread!=3 || strncmp(filetype,"PAR",3)){
    QMessageBox::warning(NULL,QObject::tr("Load Particles"),QObject::tr("Wrong file."));
    fclose(file);
    return NULL;
  }
  if(fread(&nbTypes,sizeof(unsigned char),1,file)!=1){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (1)"));
    fclose(file);
    return NULL;
  }
  if(fseek(file,nbTypes*(sizeof(float)+(NAME_LENGTH+3)*sizeof(unsigned char)),SEEK_CUR)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fseek failed (1)"));
    fclose(file);
    return NULL;
  }
  if(fread(&nbVariables,sizeof(unsigned char),1,file)!=1){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (2)"));
    fclose(file);
    return NULL;
  }
  if(fseek(file,(3+nbVariables)*NAME_LENGTH*sizeof(unsigned char),SEEK_CUR)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fseek failed (2)"));
    fclose(file);
    return NULL;
  }
  if(fread(&nbParticles,sizeof(unsigned int),1,file)!=1){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (3)"));
    fclose(file);
    return NULL;
  }
  if(fseek(file,nbParticles*sizeof(unsigned char),SEEK_CUR)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fseek failed (3)"));
    fclose(file);
    return NULL;
  }
  if(fread(&nbBonds,sizeof(unsigned int),1,file)!=1){
    QMessageBox::warning(NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (4)"));
    fclose(file);
    return NULL;
  }
  if(fread(&bRadius,sizeof(float),1,file)!=1){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (5)"));
    fclose(file);
    return NULL;
  }
  if(fseek(file,2*nbBonds*sizeof(unsigned int),SEEK_CUR)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fseek failed (4)"));
    fclose(file);
    return NULL;
  }
  if(fread(&nbSteps,sizeof(unsigned int),1,file)!=1){
    QMessageBox::warning(
       NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fread failed (6)"));
    fclose(file);
    return NULL;
  }
  if(fseek(file,(1+(3+nbVariables)*nbParticles)*nbSteps*sizeof(float),SEEK_CUR)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),QObject::tr("Corrupted File. fseek failed (5)"));
    fclose(file);
    return NULL;
  }
  fpos=ftell(file);
  fseek(file,0,SEEK_END);
  if(fpos!=ftell(file)){
    QMessageBox::warning(
        NULL,QObject::tr("Load Particles"),
        QObject::tr("Corrupted File. wrong file size. got %1 Bytes, expected %2 Bytes").
            arg(ftell(file)).arg(fpos));
    fclose(file);
    return NULL;
  }
  /*get the data and set particles*/
  particles=new Particles;
  fseek(file,4,SEEK_SET);
  particles->nbTypes=nbTypes;
  particles->nbVariables=nbVariables;
  particles->pSpec=new PartSpec[nbTypes];
  for(k=0;k<(int)nbTypes;k++){
    nread=fread(particles->pSpec[k].name,sizeof(char),NAME_LENGTH,file);
    nread=fread(&radius,sizeof(float),1,file);
    nread=fread(&r,sizeof(unsigned char),1,file);
    nread=fread(&g,sizeof(unsigned char),1,file);
    nread=fread(&b,sizeof(unsigned char),1,file);
    particles->pSpec[k].name[NAME_LENGTH]='\0';
    particles->pSpec[k].radius=radius;
    particles->pSpec[k].segs=4;
    particles->pSpec[k].color=MyColor(r,g,b);
    particles->pSpec[k].showPoints=true;
    particles->pSpec[k].pointSize=1;
    particles->pSpec[k].active=true;
  }
  fseek(file,sizeof(unsigned char),SEEK_CUR);
  particles->nbVariables=nbVariables;
  particles->varName=new char*[3+nbVariables];
  for(k=0;k<3+nbVariables;k++){
    particles->varName[k]=new char[NAME_LENGTH+1];
    nread=fread(particles->varName[k],sizeof(char),NAME_LENGTH,file);
    particles->varName[k][NAME_LENGTH]='\0';
  }
  fseek(file,sizeof(unsigned int),SEEK_CUR);
  particles->nbParticles=nbParticles;
  particles->pType=new unsigned char[nbParticles];
  for(k=0;k<(int)nbParticles;k++){
    nread=fread(&type,sizeof(unsigned char),1,file);
    particles->pType[k]=type;
  }
  fseek(file,sizeof(unsigned int)+sizeof(float),SEEK_CUR);
  particles->nbBonds=nbBonds;
  particles->bSpec.radius=bRadius;
  particles->bSpec.slices=32;
  particles->bSpec.stacks=1;
  particles->bSpec.showWires=true;
  particles->bonds=new int*[nbBonds];
  for(k=0;k<(int)nbBonds;k++){
    nread=fread(&index1,sizeof(unsigned int),1,file);
    nread=fread(&index2,sizeof(unsigned int),1,file);
    particles->bonds[k]=new int[2];
    particles->bonds[k][0]=index1;
    particles->bonds[k][1]=index2;
  }
  fseek(file,sizeof(unsigned int),SEEK_CUR);
  particles->nbSteps=nbSteps;
  particles->time=new float[nbSteps];
  particles->pPos=new float**[nbSteps];
  for(k=0;k<(int)nbSteps;k++){
    nread=fread(&t,sizeof(float),1,file);
    particles->time[k]=t;
    particles->pPos[k]=new float*[nbParticles];
    for(n=0;n<(int)nbParticles;n++){
      particles->pPos[k][n]=new float[3+nbVariables];
      nread=fread(&x,sizeof(float),1,file);
      nread=fread(&y,sizeof(float),1,file);
      nread=fread(&z,sizeof(float),1,file);
      particles->pPos[k][n][0]=x;
      particles->pPos[k][n][1]=y;
      particles->pPos[k][n][2]=z;
      for(i=0;i<nbVariables;i++){
        nread=fread(&var,sizeof(float),1,file);
        particles->pPos[k][n][i+3]=var;
      }
    }
  }
  fclose(file);
  return particles;
}

/*
  Dummy copy function.
  reduce the number of step to 2, number of particles to par->nbTypes (one particle per type),
  and number of bounds to 0;
  particles->time[0]=min of par->times[all steps]
  particles->time[1]=max of par->times[all steps]
  particles->pPos[0][i][j]=min of par->pPos[all steps][all particles with type i][j]
  particles->pPos[1][i][j]=max of par->pPos[all steps][all particles with type i][j]
  this function is used for colorDialog.
*/
Particles *minMaxParticleCopy(Particles *par){
  Particles *particles=NULL;
  int i,j,k,m;

  if(par!=NULL){
    particles=new Particles;
    particles->nbTypes = par->nbTypes;
    particles->pSpec=new PartSpec[par->nbTypes];
    memcpy(particles->pSpec,par->pSpec,(par->nbTypes)*sizeof(PartSpec));
    memcpy(&(particles->bSpec),&(par->bSpec),sizeof(BondSpec));
    particles->nbVariables = par->nbVariables;
    particles->nbParticles = par->nbTypes;
    particles->nbBonds = 0;
    particles->nbSteps = par->nbSteps>0?2:0;
    particles->varName=new char*[3+particles->nbVariables];
    for(i=0;i<3+particles->nbVariables;i++){
      particles->varName[i]=new char[NAME_LENGTH+1];
      memcpy(particles->varName[i],par->varName[i],(NAME_LENGTH+1)*sizeof(char));
    }
    particles->pType=NULL;
    particles->bonds=NULL;
    if(par->nbSteps>0){
      particles->time=new float[2];
      particles->pPos=new float**[2];
      for(i=0;i<2;i++){
        particles->pPos[i]=new float*[particles->nbParticles];
        for(j=0;j<particles->nbParticles;j++)
        particles->pPos[i][j]=new float[3+particles->nbVariables];
      }
    }
    else{
      particles->time=NULL;
      particles->pPos=NULL;
    }

    //set the min/max of time
    if(par->nbSteps>0)
      particles->time[0]=particles->time[1]=par->time[0];
    for(i=1;i<par->nbSteps;i++){
      particles->time[0]=par->time[i]<particles->time[0]?par->time[i]:particles->time[0];
      particles->time[1]=par->time[i]>particles->time[1]?par->time[i]:particles->time[1];
    }
    //set the min/max of variables (including position)
    if(par->nbSteps>0){
      for(i=0;i<3+particles->nbVariables;i++){
        for(j=0;j<particles->nbParticles;j++){
          for(k=0;k<par->nbParticles;k++){
            if(par->pType[k]==j){
              for(m=0;m<par->nbSteps;m++){
                if(par->pPos[m][k][0]!=INFINITY){
                  particles->pPos[0][j][i]=par->pPos[m][k][i];
                  particles->pPos[1][j][i]=par->pPos[m][k][i];
                  break;
                }
              }
              if(m<par->nbSteps)
              break;
            }
          }
          if(k<par->nbParticles){
            for(;k<par->nbParticles;k++){
              if(par->pType[k]==j){
                for(m=0;m<par->nbSteps;m++){
                  if(par->pPos[m][k][0]!=INFINITY){
                    particles->pPos[0][j][i]=par->pPos[m][k][i]<particles->pPos[0][j][i]?
                        par->pPos[m][k][i]:particles->pPos[0][j][i];
                    particles->pPos[1][j][i]=par->pPos[m][k][i]>particles->pPos[1][j][i]?
                        par->pPos[m][k][i]:particles->pPos[1][j][i];
                  }
                }
              }
            }
          }
          else
            particles->pPos[0][j][i]=particles->pPos[1][j][i]=0;
        }
      }
    }
  }
  return particles;
}

void freeParticles(Particles *particles){
  int i,j;
  if(particles==NULL)
    return;
  if(particles->pSpec!=NULL)    delete [] particles->pSpec;
  if(particles->pType!=NULL)    delete [] particles->pType;
  if(particles->time!=NULL)      delete [] particles->time;
  for(i=0;i<particles->nbBonds;i++)
    if(particles->bonds[i]!=NULL)  delete [] particles->bonds[i];
  if(particles->bonds!=NULL)    delete [] particles->bonds;
  for(i=0;i<particles->nbSteps;i++){
    for(j=0;j<particles->nbParticles;j++)
      if(particles->pPos[i][j]!=NULL)  delete [] particles->pPos[i][j];
    if(particles->pPos[i]!=NULL)  delete [] particles->pPos[i];
  }
  if(particles->pPos!=NULL)      delete [] particles->pPos;
  for(i=0;i<3+particles->nbVariables;i++)
    if(particles->varName[i]!=NULL)  delete [] particles->varName[i];
  if(particles->varName!=NULL)  delete [] particles->varName;
}

QColor getParticleColor(Particles *particles, int step, int pIndex){
  MyColor *color=&(particles->pSpec[particles->pType[pIndex]].color);
  int varIndex=color->getVarIndex();
  if(!color->isGradColorEnabled())
    return (*color)();
  else if(varIndex==0)
    return (*color)(particles->time[step]);
  else if(varIndex<=3+particles->nbVariables)
    return (*color)(particles->pPos[step][pIndex][varIndex-1]);
  return QColor(0,0,0);
}

QColor getColor(Particles *particles, MyColor &color, int step, int pIndex){
  int varIndex=color.getVarIndex();
  if(!color.isGradColorEnabled())
    return color();
  else if(varIndex==0)
    return color(particles->time[step]);
  else if(varIndex<=3+particles->nbVariables)
    return color(particles->pPos[step][pIndex][varIndex-1]);
  return QColor(0,0,0);
}
