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
  bool            showWires;     //show wires instead of cylinders if set to true
} BondSpec;

typedef struct {
  bool            active;         //show the mesh if set to true
  char            name[NAME_LENGTH+1];
  unsigned int    nbTriangles;
  unsigned int    (*indices)[3];  //array of nbTriangles unsigned int[3]
  bool            showWires;      //show wires instead of triangles if set to true
  bool            cullBackFace;   //hide back face if set to true (default=true);
  bool            sameType;       //true if all the particles in the mesh are of the same type.
} MeshSpec;

typedef struct {
  unsigned char   nbTypes;
  unsigned char   nbVariables;
  unsigned int    nbParticles;
  unsigned int    nbBonds;
  unsigned int    nbMeshes;
  unsigned int    nbSteps;
  PartSpec        *pSpec;       //array of nbTypes PartSpec
  BondSpec        bSpec;
  MeshSpec        *mSpec;       //array of nbMeshes MeshSpec
  char            (*varName)[NAME_LENGTH+1];  //array of (3+nbVariables) char[NAME_LENGTH+1] (last char='\0')
  unsigned char   *pType;       //array of nbParticles uchar (particle type)
  unsigned int    (*bonds)[2];  //array of nbBonds unsigned int[2]
  float           *time;        //array of nbSteps float
  float           ***pPos;      //array of nbSteps*nbParticles*(3+nbVariables) floats
                                //(particle position+variables)
} Particles;

Particles *loadParticles(QString &fileName);
Particles *minMaxParticleCopy(Particles*);
void      freeParticles(Particles *particles);
QColor    getParticleColor(Particles *particles, int step, int pIndex);
QColor    getColor(Particles *particles, MyColor &color, int step, int pIndex);

#endif
