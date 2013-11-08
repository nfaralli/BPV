#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <QList>
#include <QColor>
#include <QPoint>

typedef struct{
  float x;
  float y;
  float z;
} PointF3D;

typedef struct{
  int i;
  int j;
  int k;
} PointI3D;

typedef struct{
  bool    active;
  int     lengthsegs;
  int     widthsegs;
  int     heightsegs;
  float   length;
  float   width;
  float   height;
  float   pos[3];
  int     color[4];
  float   transform[16];
  QString name;
} MyBox;

typedef struct{
  bool    active;
  int     heightsegs;
  int     capsegs;
  int     sides;
  float   height;
  float   radius;
  float   pos[3];
  int     color[4];
  float   transform[16];
  QString name;
} MyCylinder;

typedef struct{
  bool    active;
  int     segs;
  float   radius;
  float   pos[3];
  int     color[4];
  float   transform[16];
  QString name;
} MySphere;

typedef struct{
  bool    active;
  QList<PointF3D> points;
  int     nbpts;
  bool    close;
  int     color[4];
  float   transform[16];
  QString name;
} MyLines;

typedef struct{
  bool    active;
  QList<PointF3D> vertices;
  QList<PointI3D> faces;
  int     color[4];
  float   transform[16];
  QString name;
} MyMesh;

typedef struct{
  int nbObj;
  QList<MyBox>      boxes;
  QList<MyCylinder> cylinders;
  QList<MySphere>   spheres;
  QList<MyLines>    lines;
  QList<MyMesh>     meshes;
} MyGeometry;

MyGeometry *loadGeometry(QString &fileName);
void       freeGeometry(MyGeometry *geometry);

#endif
