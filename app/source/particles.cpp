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
  nbMeshes                (1 unsigned int)
  name mesh1              (NAME_LENGTH bytes)
  nbTriangles mesh1       (1 unsigned int)
  indices triangle1 mesh1 (3 unsigned int)
  indices triangle2 mesh1 (3 unsigded int)
  etc.
  name mesh2              (NAME_LENGTH bytes)
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

/*
    PAR (3 bytes)
*/
static bool loadParSection(FILE *file, Particles*, QString *error) {
    size_t nread;
    char filetype[3];
    nread=fread(filetype,1,3,file);
    if(nread!=3 || strncmp(filetype,"PAR",3)){
        *error = "Invalid file: file must start with \"PAR\".";
        return false;
    }
    return true;
}

/*
    nbTypes        (1 byte)
    name    type 1 (NAME_LENGTH bytes)
    radius  type 1 (1 float)
    red     type 1 (1 byte)
    green   type 1 (1 byte)
    blue    type 1 (1 byte)
    name    type 2 (NAME_LENGTH bytes)
    radius  type 2 (1 float)
    etc.
*/
static bool loadTypesSection(FILE *file, Particles *particles, QString *error) {
    unsigned char nbTypes, i, r, g, b;
    long fPos;

    if(fread(&nbTypes,sizeof(unsigned char),1,file)!=1){
      *error = "Invalid file: missing types section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos < (long)(nbTypes*(sizeof(float)+(NAME_LENGTH+3)*sizeof(unsigned char)))){
      *error = "Invalid file: invalid types section.";
      return false;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbTypes = nbTypes;
    particles->pSpec=(PartSpec*)calloc(nbTypes, sizeof(PartSpec));
    for(i=0; i<nbTypes; i++){
      fread(particles->pSpec[i].name,sizeof(char),NAME_LENGTH,file);
      fread(&(particles->pSpec[i].radius),sizeof(float),1,file);
      fread(&r,sizeof(unsigned char),1,file);
      fread(&g,sizeof(unsigned char),1,file);
      fread(&b,sizeof(unsigned char),1,file);
      particles->pSpec[i].segs=4;
      particles->pSpec[i].color=MyColor(r,g,b);
      particles->pSpec[i].showPoints=true;
      particles->pSpec[i].pointSize=1;
      particles->pSpec[i].active=true;
    }
    return true;
}

/*
    nbVariables        (1 byte)
    name    x  axis    (NAME_LENGTH bytes)
    name    y  axis    (NAME_LENGTH bytes)
    name    z  axis    (NAME_LENGTH bytes)
    name    variable 1 (NAME_LENGTH bytes)
    name    variable 2 (NAME_LENGTH bytes)
*/
static bool loadVariablesSection(FILE *file, Particles *particles, QString *error) {
    unsigned char nbVariables, i;
    long fPos;

    if(fread(&nbVariables,sizeof(unsigned char),1,file)!=1){
      *error = "Invalid file: missing variables section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos < (long)((3+nbVariables)*NAME_LENGTH*sizeof(unsigned char))){
      *error = "Invalid file: invalid variables section.";
      return false;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbVariables = nbVariables;
    particles->varName = (char(*)[NAME_LENGTH+1])calloc(3+nbVariables, sizeof(char[NAME_LENGTH+1]));
    for(i=0;i<3+nbVariables;i++){
      fread(particles->varName[i],sizeof(char),NAME_LENGTH,file);
    }
    return true;
}

/*
    nbParticles        (1 unsigned int)
    type of particle 1 (1 byte)
    type of particle 2 (1 byte)
    etc.
*/
static bool loadParticlesTypeSection(FILE *file, Particles *particles, QString *error) {
    unsigned int nbParticles;
    long fPos;

    if(fread(&nbParticles,sizeof(unsigned int),1,file)!=1){
      *error = "Invalid file: missing particles type section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos < (long)(nbParticles*sizeof(unsigned char))){
      *error = "Invalid file: invalid particles type section.";
      return false;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbParticles = nbParticles;
    particles->pType=(unsigned char*)calloc(nbParticles, sizeof(unsigned char));
    fread(particles->pType,sizeof(unsigned char),nbParticles,file);
    return true;
}

/*
    nbBonds             (1 unsigned int)
    bonds radius        (1 float)
    1st index of bond 1 (1 unsigned int)
    2nd index of bond 1 (1 unsigned int)
    1st index of bond 2 (1 unsigned int)
    etc.
*/
static bool loadBondsSection(FILE *file, Particles *particles, QString *error) {
    unsigned int nbBonds, i;
    long fPos;

    if(fread(&nbBonds,sizeof(unsigned int),1,file)!=1){
      *error = "Invalid file: missing bonds section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos < (long)(sizeof(float)+2*nbBonds*sizeof(unsigned int))){
      *error = "Invalid file: invalid bonds section.";
      return false;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbBonds = nbBonds;
    fread(&(particles->bSpec.radius),sizeof(float),1,file);
    particles->bSpec.slices=32;
    particles->bSpec.stacks=1;
    particles->bSpec.showWires=true;
    particles->bonds=(unsigned int(*)[2])calloc(nbBonds, sizeof(unsigned int[2]));
    for(i=0;i<nbBonds;i++){
      fread(particles->bonds[i], sizeof(unsigned int), 2, file);
    }
    return true;
}

/*
    nbMeshes                (1 unsigned int)
    name mesh1              (NAME_LENGTH bytes)
    nbTriangles mesh1       (1 unsigned int)
    indices triangle1 mesh1 (3 unsigned int)
    indices triangle2 mesh1 (3 unsigded int)
    etc.
    name mesh2              (NAME_LENGTH bytes)
    etc.
*/
static bool loadMeshesSection(FILE *file, Particles *particles, QString *error) {
    MeshSpec *mSpec;
    unsigned int nbMeshes, nbTriangles, i, j;
    unsigned char type;
    long fPos;

    if(fread(&nbMeshes,sizeof(unsigned int),1,file)!=1){
      *error = "Invalid file: missing meshes section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos == (long)(nbMeshes*(1+particles->nbParticles*(3+particles->nbVariables))*sizeof(float))){
      // Special case for backward compatibility:
      // old versions didn't contain meshes section, and nbMeshes here is actually nbSteps.
      fseek(file, fPos-sizeof(unsigned int), SEEK_SET);
      return true;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbMeshes = nbMeshes;
    particles->mSpec = (MeshSpec*)calloc(nbMeshes, sizeof(MeshSpec));
    for(mSpec=particles->mSpec; mSpec<particles->mSpec+nbMeshes; mSpec++){
        if(fread(mSpec->name,sizeof(char),NAME_LENGTH,file)!=NAME_LENGTH){
          *error = "Invalid file: invalid meshes section.";
          return false;
        }
        if(fread(&nbTriangles,sizeof(unsigned int),1,file)!=1){
          *error = "Invalid file: invalid meshes section.";
          return false;
        }
        fPos = ftell(file);
        fseek(file, 0, SEEK_END);
        if(ftell(file)-fPos < (long)(nbTriangles*3*sizeof(unsigned int))){
          *error = "Invalid file: invalid meshes section.";
          return false;
        }
        fseek(file, fPos, SEEK_SET);
        mSpec->active=true;
        mSpec->showWires=false;
        mSpec->cullBackFace=true;
        mSpec->nbTriangles = nbTriangles;
        mSpec->indices = (unsigned int(*)[3])calloc(nbTriangles, sizeof(unsigned int[3]));
        for(i=0; i<mSpec->nbTriangles; i++){
            fread(mSpec->indices+i, sizeof(unsigned int), 3, file);
        }
        mSpec->sameType=true;
        if(nbTriangles>0){
            type=particles->pType[mSpec->indices[0][0]];
            for(i=0; i<mSpec->nbTriangles; i++){
                for(j=0; j<3; j++){
                    if(particles->pType[mSpec->indices[i][0]]!=type){
                        mSpec->sameType=false;
                        break;
                    }
                }
                if(j<3){
                    break;
                }
            }
        }
    }
    return true;
}

/*
    nbSteps                        (1 unsigned int)
    time step 1                    (1 float)
    x coordinate particle 1 step 1 (1 float)
    y coordinate particle 1 step 1 (1 float)
    z coordinate particle 1 step 1 (1 float)
    variable 1 particle 1 step 1   (1 float)
    variable 2 particle 1 step 1   (1 float)
    etc.
    x coordinate particle 2 step 1 (1 float)
    etc.
    time step 2                    (1 float)
    etc.
*/
static bool loadStepsSection(FILE *file, Particles *particles, QString *error) {
    unsigned int nbSteps, i, j;
    long fPos;

    if(fread(&nbSteps,sizeof(unsigned int),1,file)!=1){
      *error = "Invalid file: missing steps section.";
      return false;
    }
    fPos = ftell(file);
    fseek(file, 0, SEEK_END);
    if(ftell(file)-fPos != (long)(nbSteps*(1+particles->nbParticles*(3+particles->nbVariables))*sizeof(float))){
      *error = "Invalid file: invalid steps section.";
      return false;
    }
    fseek(file, fPos, SEEK_SET);
    particles->nbSteps=nbSteps;
    particles->time=(float*)calloc(nbSteps, sizeof(float));
    particles->pPos=(float***)calloc(nbSteps, sizeof(float**));
    for(i=0;i<nbSteps;i++){
      fread(particles->time+i,sizeof(float),1,file);
      particles->pPos[i]=(float**)calloc(particles->nbParticles, sizeof(float*));
      for(j=0;j<particles->nbParticles;j++){
        particles->pPos[i][j]=(float*)calloc(3+particles->nbVariables, sizeof(float));
        fread(particles->pPos[i][j],sizeof(float),3+particles->nbVariables,file);
      }
    }
    return true;
}

Particles *loadParticles(QString &fileName){
    Particles *particles=NULL;
    FILE    *file;
    QString error;
    unsigned int i;
    bool (*loadFunctions[])(FILE*, Particles*, QString*) = {
        loadParSection,
        loadTypesSection,
        loadVariablesSection,
        loadParticlesTypeSection,
        loadBondsSection,
        loadMeshesSection,
        loadStepsSection
    };
    file=fopen(fileName.toStdString().data(),"rb");
    if(file==NULL) {
        QMessageBox::warning(NULL, QObject::tr("Load Particles"), "Failed to open file.");
        return NULL;
    }
    particles = (Particles*)calloc(1, sizeof(Particles));
    for(i=0; i<sizeof(loadFunctions)/sizeof(*loadFunctions); i++) {
        if(!loadFunctions[i](file, particles, &error)){
            QMessageBox::warning(NULL, QObject::tr("Load Particles"), error);
            freeParticles(particles);
            delete particles;
            return NULL;
        }
    }
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
  unsigned int i,j,k,m;

  if(par!=NULL){
    particles=(Particles*)calloc(1, sizeof(Particles));
    particles->nbTypes = par->nbTypes;
    particles->pSpec=new PartSpec[par->nbTypes];
    memcpy(particles->pSpec,par->pSpec,(par->nbTypes)*sizeof(PartSpec));
    memcpy(&(particles->bSpec),&(par->bSpec),sizeof(BondSpec));
    particles->nbVariables = par->nbVariables;
    particles->nbParticles = par->nbTypes;
    particles->nbBonds = 0;
    particles->nbMeshes = 0;
    particles->nbSteps = par->nbSteps>0?2:0;
    particles->varName=(char(*)[NAME_LENGTH+1])calloc(3+particles->nbVariables, sizeof(char[NAME_LENGTH+1]));
    for(i=0;i<3u+particles->nbVariables;i++){
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
      for(i=0;i<3u+particles->nbVariables;i++){
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
  unsigned int i,j;
  if(particles==NULL)
    return;
  if(particles->pSpec!=NULL)    delete [] particles->pSpec;
  if(particles->pType!=NULL)    delete [] particles->pType;
  if(particles->varName!=NULL)  delete [] particles->varName;
  if(particles->time!=NULL)      delete [] particles->time;
  if(particles->bonds!=NULL)    delete [] particles->bonds;
  if(particles->mSpec!=NULL){
      for(i=0; i<particles->nbMeshes; i++){
          if(particles->mSpec[i].indices!=NULL) delete [] particles->mSpec[i].indices;
      }
      delete [] particles->mSpec;
  }
  if(particles->pPos!=NULL){
      for(i=0;i<particles->nbSteps;i++){
          if(particles->pPos[i]!=NULL){
              for(j=0;j<particles->nbParticles;j++)
                  if(particles->pPos[i][j]!=NULL)  delete [] particles->pPos[i][j];
              delete [] particles->pPos[i];
          }
      }
      delete [] particles->pPos;
  }
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
