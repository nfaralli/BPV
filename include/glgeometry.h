#ifndef __GLGEOMETRY_H__
#define __GLGEOMETRY_H__

#include <math.h>
#include <QGLWidget>
#include "geometry.h"

#define PI (3.14159265358979323846) 

void   drawBox            (MyBox&);
void   drawCylinder       (float rb, float rt, float h, int slices, int stacks, int mode);
void   drawCylinder       (MyCylinder&);
void   drawSphere         (MySphere&);
void   drawGeosphere      (float, int);
void   drawGeosphere      (MySphere&);
void   drawLines          (MyLines&);
void   drawMesh           (MyMesh&);
void   getMinMaxBox       (MyBox&, float*);
void   getMinMaxCylinder  (MyCylinder&, float*);
void   getMinMaxSphere    (MySphere&, float*);
void   getMinMaxLines     (MyLines&, float*);
void   getMinMaxMesh      (MyMesh&, float*);
void   transformBoxDims   (float*, float[16]);

#endif
