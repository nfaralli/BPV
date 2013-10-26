#ifndef __PARTICLES_H__
#define __PARTICLES_H__

#include <QList>
#include <QString>
#include "mycolor.h"

#define NAME_LENGTH 32

//particle specifications
typedef struct {
  bool            active;        //show the particle if set to true
  char            name[NAME_LENGTH+1];
  float           radius;        //sphere radius
  int             segs;          //nb segments used to draw the geosphere
  bool            showPoints;    //show a point instead of a sphere if set to true
  int             pointSize;     //point size. point=square of pointSize x pointSize pixels
  MyColor         color;         //sphere/point color
} PartSpec;

//bonds specifications
typedef struct {
  float           radius;        //cylinders radius
  int             slices;        //nb slices used to draw the cylinders
  int             stacks;        //nb stacks used to draw the cylinders
  bool            showWires;     //show wires instead of a cylinders if set to true
} BondSpec;

typedef struct {
  int             nbTypes;
  int             nbVariables;
  int             nbParticles;
  int             nbBonds;
  int             nbSteps;
  PartSpec        *pSpec;     //array of nbTypes PartSpec
  BondSpec        bSpec;
  char            **varName;  //array of (3+nbVariables)*(NAME_LENGTH+1) char (last char='\0')
  unsigned char   *pType;     //array of nbParticles uchar (particle type)
  int             **bonds;    //array of nbBonds*2 int
  float           *time;      //array of nbSteps float
  float           ***pPos;    //array of nbSteps*nbParticles*(3+nbVariables) floats
                              //(particle position+variables)
} Particles;

Particles *loadParticlesPAR(QString &fileName);
Particles *minMaxParticleCopy(Particles*);
void      freeParticles(Particles *particles);
QColor    getParticleColor(Particles *particles, int step, int pIndex);
QColor    getColor(Particles *particles, MyColor &color, int step, int pIndex);

#endif
