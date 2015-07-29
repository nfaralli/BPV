#include "glwidget.h"
#include "fonts.h"
#include <QColor>
#include <QtGui>
#include <QtOpenGL>
#include <QMessageBox>
#include <QString>
#include <stdarg.h>


GLdouble v2[4],v3[4]; //test. to be removed!!!

typedef struct _Image{
  unsigned int width;
  unsigned int height;
  unsigned char (*data)[3]; // image data (RGB, stored from left to right, top to bottom.
                            // data[0]=top left corner)
} Image;
Image* loadBMP(unsigned char *fdata);

//forward declaration of static functions
static int getStreamSize(const char *stream, int charWidth[95]);
static GLdouble* inverseScaled(GLdouble x, GLdouble y, GLdouble z);
static GLdouble* inverseOrtho(GLdouble left, GLdouble right,
                              GLdouble bottom, GLdouble top,
                              GLdouble near, GLdouble far);
static GLdouble* inverseFrustum(GLdouble left, GLdouble right,
                                GLdouble bottom, GLdouble top,
                                GLdouble near, GLdouble far);
static GLdouble* rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
static GLdouble* translated(GLdouble x, GLdouble y, GLdouble z);
static GLdouble* copyMatrixd44(GLdouble *m, GLdouble *dest);
static GLdouble* multMatrixd444(GLdouble *m1, GLdouble *m2, GLdouble *dest);
static GLdouble* multMatrixd441(GLdouble *m1, GLdouble *m2, GLdouble *dest);

/*get size, in pixel, of stream.*/
static int getStreamSize(const char *stream, int charWidth[95]){
  int len=0;
  const char *c=stream;

  while(*c!='\0')
    if(*c>=0x20 && *c<=0x7E)
      len+=charWidth[(*c++)-0x20];
  return len;
}

/*returns the inverse of the matrix obtained with glScalef(x,y,z)*/
static GLdouble* inverseScaled(GLdouble x, GLdouble y, GLdouble z){
  static GLdouble m[16];
  static bool firstTime=true;

  if(firstTime){
    m[1]=m[2]=m[3]=0;
    m[4]=m[6]=m[7]=0;
    m[8]=m[9]=m[11]=0;
    m[12]=m[13]=m[14]=0;
    m[15]=1.;
    firstTime=false;
  }
  if(x==0 || y==0 || z==0)
    return NULL;
  m[0]=1.f/x;
  m[5]=1.f/y;
  m[10]=1.f/z;
  return m;
}

/*returns the inverse of the matrix obtained with glOrtho(l,r,b,t,n,f)*/
static GLdouble* inverseOrtho(GLdouble l, GLdouble r,
                              GLdouble b, GLdouble t,
                              GLdouble n, GLdouble f){
  static GLdouble m[16];
  static bool firstTime=true;

  if(firstTime){
    m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[11]=0;
    m[15]=1.;
    firstTime=false;
  }
  if(l>=r || b>=t || n>=f)
    return NULL;
  m[0]=(r-l)/2.;
  m[5]=(t-b)/2.;
  m[10]=-(f-n)/2.;
  m[12]=(r+l)/2.;
  m[13]=(t+b)/2.;
  m[14]=-(f+n)/2.;
  return m;
}

/*returns the inverse of the matrix obtained with glFrustum(l,r,b,t,n,f)*/
static GLdouble* inverseFrustum(GLdouble l, GLdouble r,
                                GLdouble b, GLdouble t,
                                GLdouble n, GLdouble f){
  static GLdouble m[16];
  static bool firstTime=true;

  if(firstTime){
    m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[10]=0;
    m[14]=-1;
    firstTime=false;
  }
  if(l>=r || b>=t || n>=f || n<=0)
    return NULL;
  m[0]=(r-l)/(2.*n);
  m[5]=(t-b)/(2.*n);
  m[11]=-(f-n)/(2.*f*n);
  m[12]=(r+l)/(2.*n);
  m[13]=(t+b)/(2.*n);
  m[15]=(f+n)/(2.*f*n);
  return m;
}

/*returns a rotation matrix.
 * (x,y,z) is the normal defining the axis of the rotation*/
static GLdouble* rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z){
  static GLdouble m[16];
  static bool firstTime=true;
  GLdouble c,s,coef;

  if(firstTime){
    m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0;
    m[15]=1.;
    firstTime=false;
  }
  angle*=0.0174532925;
  c=cos(angle);
  s=sin(angle);
  coef=1-c;
  m[0]=m[1]=m[2]=x*coef;
  m[0]=x*m[0]+c;
  m[1]=m[4]=y*m[1];
  m[2]=m[8]=z*m[2];
  m[5]=m[6]=y*coef;
  m[5]=y*m[5]+c;
  m[6]=m[9]=z*m[6];
  m[10]=z*z*coef+c;
  coef=z*s;
  m[1]+=coef;
  m[4]-=coef;
  coef=y*s;
  m[2]-=coef;
  m[8]+=coef;
  coef=x*s;
  m[6]+=coef;
  m[9]-=coef;
  return m;
}

/*returns a translation matrix
 * (x,y,z) is the vector defining this translation*/
static GLdouble* translated(GLdouble x, GLdouble y, GLdouble z){
  static GLdouble m[16];
  static bool firstTime=true;

  if(firstTime){
    m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[11]=0;
    m[0]=m[5]=m[10]=m[15]=1.;
    firstTime=false;
  }
  m[12]=x;
  m[13]=y;
  m[14]=z;
  return m;
}

/*returns a copy of m
 * if dest != NULL, copy m into dest and return dest*/
static GLdouble* copyMatrixd44(GLdouble *m, GLdouble *dest){
  GLdouble *out,*elout,*elm;

  if(m==NULL)
    return NULL;
  if(dest==m)
    return dest;
  if(dest==NULL)
    out=(GLdouble*)malloc(16*sizeof(GLdouble));
  else
    out=dest;
  for(elout=out,elm=m;elm<m+16;*(elout++)=*(elm++));
  return out;
}

/*returns m1*m2, where m1 and m2 are two 4x4 matrices.
 * the matrices are stored columnwise.
 * if dest!=NULL, stores the result in dest.
 * dest can be m1 and/or m2*/
static GLdouble* multMatrixd444(GLdouble *m1, GLdouble *m2, GLdouble *dest){
  GLdouble *out,*elout;
  GLdouble *elm1,*elm2;
  int i,j;

  if(m1==NULL || m2==NULL)
    return NULL;
  if(dest==NULL || dest==m1 || dest==m2)
    out=(GLdouble*)malloc(16*sizeof(GLdouble));
  else
    out=dest;
  for(j=0,elout=out;j<4;j++)
    for(i=0;i<4;i++,elout++)
      for(*elout=0,elm1=m1+i,elm2=m2+4*j;elm1<=m1+i+12;elm1+=4,elm2++)
        *elout+=(*elm1)*(*elm2);
  if(dest==m1 || dest==m2){
    for(i=0;i<16;i++)
      dest[i]=out[i];
    free(out);
    return dest;
  }
  return out;
}

/*returns m1*m2, where m1 is a 4x4 matrix, and m2 is a 4*1 matrix.
 * the matrices are stored columnwise.
 * if dest!=NULL, stores the result in dest.
 * dest can be m2*/
static GLdouble* multMatrixd441(GLdouble *m1, GLdouble *m2, GLdouble *dest){
  GLdouble *out,*elout;
  GLdouble *elm1,*elm2;
  int i;

  if(m1==NULL || m2==NULL)
    return NULL;
  if(dest==NULL || dest==m2)
    out=(GLdouble*)malloc(4*sizeof(GLdouble));
  else
    out=dest;
  for(i=0,elout=out;i<4;i++,elout++)
    for(*elout=0,elm1=m1+i,elm2=m2;elm1<=m1+i+12;elm1+=4,elm2++)
      *elout+=(*elm1)*(*elm2);
  if(dest==m2){
    for(i=0;i<4;i++)
      dest[i]=out[i];
    free(out);
    return dest;
  }
  return out;
}

GLWidget::GLWidget(QWidget *parent) :
  QGLWidget(parent){

  setMinimumSize(200, 200);
  setFocusPolicy(Qt::ClickFocus);
  bgColor = MyColor(0, 0, 0);
  shiftKeyPressed = false;
  xRot = -(int)(90*ANGLE_DIV);
  yRot = 0;
  zRot = 0;
  depth = -1;
  depth0 = -1;
  xCen = yCen = 0;
  xCenterSc = yCenterSc = zCenterSc = 0;
  fovy = 45*0.0174532925; //field of view in the y direction for the perspective, in radiant
  particles = NULL;
  geometry = NULL;
  step = 0;
  enabledAxis = true;
  perspective = false;
  totScale = 1;
  nbParticles = 0;
  nbBonds = 0;
  nbMeshes = 0;
  nbTypes = 0;
  enabledParticles = false;
  enabledBonds = false;
  enabledMeshes = false;
  enabledGeometry = false;
  oldMaxDim = 0;
  animTra = true;
  useGlList = true;
  resetBlanks();
  resetTrajectory();
  fontList=0;
  fontPixelsPos=NULL;
  fontPixelsNum=NULL;
  timeStr[0]=timeStr[99]='\0';
  timeXPos=timeYPos=0;
  showText=true;
  fontColor[0]=fontColor[1]=fontColor[2]=1.;
  isInitialized=false;

  fps=-1;
  fpsTimer = new QTimer(this);
  fpsTimer->setInterval(2000); //when fps changed, show new value for 2s
  fpsTimer->setSingleShot(true);
  connect(fpsTimer,SIGNAL(timeout()),this,SLOT(updateGL()));

  //test:
  v2[0]=v2[1]=v2[2]=v2[3]=0;
  v3[0]=v3[1]=v3[2]=v3[3]=0;
}

void GLWidget::initializeGL(){
  float diffuseLight[] = {1.0, 1.0, 1.0, 1.0};
  float positionLight1[] = {0.0, 0.0, 2.0, 1.0};
  float positionLight2[] = {-50, -50, 200.0, 1.0};
  float positionLight3[] = {150, 150, 200.0, 1.0};
  qglClearColor(bgColor());
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
  glLightfv(GL_LIGHT1, GL_POSITION, positionLight1);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseLight);
  glLightfv(GL_LIGHT2, GL_POSITION, positionLight2);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuseLight);
  glLightfv(GL_LIGHT3, GL_POSITION, positionLight3);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LIGHT1);
  glEnable( GL_LIGHTING);
  glShadeModel( GL_SMOOTH);
  glEnable( GL_DEPTH_TEST);
  glEnable( GL_CULL_FACE);
  if(!isInitialized)
    emit initializationDone(); //GLWidget::renderPixmap doesn't like this signal...
}

void GLWidget::paintGL(){
  isInitialized=true;
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
  glLoadIdentity();
  if(perspective) glTranslatef(xCen, yCen, depth); // transform the origin.
  else glTranslatef(xCen, yCen, depth0);
  glRotatef(xRot/ANGLE_DIV, 1.0, 0.0, 0.0);
  glRotatef(yRot/ANGLE_DIV, 0.0, 1.0, 0.0);
  glRotatef(zRot/ANGLE_DIV, 0.0, 0.0, 1.0);
  glTranslatef(xCenterSc, yCenterSc, zCenterSc);

  /*glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  glColor3f(1,1,1);
  glVertex4dv(v2);
  glVertex4dv(v3);
  glEnd();
  glEnable(GL_LIGHTING);*/

  drawScene(true); // draw opaque objects first
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  drawScene(false); // draw transparent object
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  if(enabledAxis){
    glClear(GL_DEPTH_BUFFER_BIT);
    drawAxis();
  }
  if(showText){
    printScr(timeXPos,timeYPos,timeStr);
    if(fpsTimer->isActive())
      printScr(viewWidth-1-getStreamSize("fps=000",charWidth),0,"fps=%d",fps);
    //printMat(GL_PROJECTION_MATRIX);
  }
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // for testing purpose
}

void GLWidget::printMat(GLenum mode){
  GLfloat m[16];

  glGetFloatv(mode, m);
  printScr(0,0,"%+e %+e %+e %+e\n%+e %+e %+e %+e\n%+e %+e %+e %+e\n%+e %+e %+e %+e\n",
      m[0],m[4],m[8],m[12],
      m[1],m[5],m[9],m[13],
      m[2],m[6],m[10],m[14],
      m[3],m[7],m[11],m[15]);
}

bool GLWidget::blankParticle(int index, int theStep){
  float value;
  int i, k;
  for(i = 0; i<nbBlanksUsed; i++){
    k = blankIndex[i];
    if(blanks[k].active){
      if(blanks[k].allTypes||(!blanks[k].allTypes&&particles->pType[index]==blanks[k].typeIndex)){
        if(blanks[k].varIndex==0) value = particles->time[theStep];
        else value = particles->pPos[theStep][index][blanks[k].varIndex-1];
        switch(blanks[k].opIndex){
        case 0:
          if(value<blanks[k].value) return true;
          break;
        case 1:
          if(value<=blanks[k].value) return true;
          break;
        case 2:
          if(value==blanks[k].value) return true;
          break;
        case 3:
          if(value>=blanks[k].value) return true;
          break;
        case 4:
          if(value>blanks[k].value) return true;
          break;
        case 5:
          if(value!=blanks[k].value) return true;
          break;
        }
      }
    }
  }
  return false;
}

void GLWidget::drawScene(bool opaque){
  unsigned char type;
  unsigned int index, indexMesh;
  int k;
  unsigned char type1, type2, type3;
  QColor color, color1, color2, color3;
  unsigned int *bond, *triangle;
  float u[3], nn[2], norm, angle;
  float *pos, *pos1, *pos2, *pos3, halfpos[3], diffuseColor[4];
  float pt0[3], pt1[3], pt2[3], c0[4], c1[4], c2[4];
  MeshSpec *mSpec;
  bool showParticle = true;
  bool showBond = true;
  bool showMesh = true;

  if(particles!=NULL){
    if(nbTrajActive>0&&opaque) for(k = 0; k<nbTrajActive; k++)
      drawTrajectory(trajectory[trajIndex[k]]);
    if(enabledParticles){
      for(index = 0; index<particles->nbParticles; index++){
        type = particles->pType[index];
        showParticle = particles->pSpec[type].active;
        if(nbBlanksUsed>0) showParticle = showParticle&&(!blankParticle(index, step));
        if(showParticle){
          pos = particles->pPos[step][index];
          color = getParticleColor(particles, step, index);
          diffuseColor[0] = color.redF();
          diffuseColor[1] = color.greenF();
          diffuseColor[2] = color.blueF();
          diffuseColor[3] = color.alphaF();
          if(((opaque&&(color.alpha()==255))||(!opaque&&(color.alpha()<255)))&&pos[0]!=INFINITY){
            if(particles->pSpec[type].showPoints){
              glPointSize(particles->pSpec[type].pointSize);
              glDisable( GL_LIGHTING);
              glBegin( GL_POINTS);
              glColor4fv(diffuseColor);
              glVertex3f(pos[0], pos[1], pos[2]);
              glEnd();
              glEnable(GL_LIGHTING);
            }else{
              glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
              glPushMatrix();
              glTranslatef(pos[0], pos[1], pos[2]);
              if(useGlList) glCallList(partsList+type);
              else drawGeosphere(particles->pSpec[type].radius, particles->pSpec[type].segs);
              glPopMatrix();
            }
          }
        }
      }
    }
    if(enabledBonds){
      for(index = 0; index<particles->nbBonds; index++){
        bond = particles->bonds[index];
        type1 = particles->pType[bond[0]];
        type2 = particles->pType[bond[1]];
        showBond = particles->pSpec[type1].active&&particles->pSpec[type2].active;
        if(nbBlanksUsed>0) showBond = showBond&&(!blankParticle(bond[0], step))
            &&(!blankParticle(bond[1], step));
        if(showBond){
          pos1 = particles->pPos[step][bond[0]];
          pos2 = particles->pPos[step][bond[1]];
          color1 = getParticleColor(particles, step, bond[0]);
          color2 = getParticleColor(particles, step, bond[1]);
          if((((opaque&&(color1.alpha()==255))||(!opaque&&(color1.alpha()<255)))||((opaque
              &&(color2.alpha()==255))||(!opaque&&(color2.alpha()<255))))&&pos1[0]!=INFINITY
              &&pos2[0]!=INFINITY){
            if(particles->bSpec.showWires){
              glDisable( GL_LIGHTING);
              glBegin( GL_LINES);
              halfpos[0] = (pos1[0]+pos2[0])/2;
              halfpos[1] = (pos1[1]+pos2[1])/2;
              halfpos[2] = (pos1[2]+pos2[2])/2;
              if((opaque&&(color1.alpha()==255))||(!opaque&&(color1.alpha()<255))){
                glColor4f(color1.redF(), color1.greenF(), color1.blueF(), color1.alphaF());
                glVertex3f(pos1[0], pos1[1], pos1[2]);
                glVertex3f(halfpos[0], halfpos[1], halfpos[2]);
              }
              if((opaque&&(color2.alpha()==255))||(!opaque&&(color2.alpha()<255))){
                glColor4f(color2.redF(), color2.greenF(), color2.blueF(), color2.alphaF());
                glVertex3f(halfpos[0], halfpos[1], halfpos[2]);
                glVertex3f(pos2[0], pos2[1], pos2[2]);
              }
              glEnd();
              glEnable(GL_LIGHTING);
            }else{
              u[0] = pos2[0]-pos1[0];
              u[1] = pos2[1]-pos1[1];
              u[2] = pos2[2]-pos1[2];
              norm = sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
              u[0] /= norm;
              u[1] /= norm;
              u[2] /= norm;
              if(fabs(u[2])>1) u[2] = u[2]>0 ? 1 : -1;
              angle = -acos(u[2])*57.29577951;
              if(1-fabs(u[2])<1e-5){
                if(u[2]>0) angle = 0;
                else angle = 180;
                nn[0] = 1;
                nn[1] = 0;
              }else{
                nn[0] = u[1];
                nn[1] = -u[0];
              }
              glPushMatrix();
              glTranslatef(pos1[0], pos1[1], pos1[2]);
              glRotatef(angle, nn[0], nn[1], 0);
              if((opaque&&(color1.alpha()==255))||(!opaque&&(color1.alpha()<255))){
                diffuseColor[0] = color1.redF();
                diffuseColor[1] = color1.greenF();
                diffuseColor[2] = color1.blueF();
                diffuseColor[3] = color1.alphaF();
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
                drawCylinder(particles->bSpec.radius, particles->bSpec.radius, norm/2,
                    particles->bSpec.slices, particles->bSpec.stacks, 0);
              }
              if((opaque&&(color2.alpha()==255))||(!opaque&&(color2.alpha()<255))){
                diffuseColor[0] = color2.redF();
                diffuseColor[1] = color2.greenF();
                diffuseColor[2] = color2.blueF();
                diffuseColor[3] = color2.alphaF();
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
                glTranslatef(0, 0, norm/2);
                drawCylinder(particles->bSpec.radius, particles->bSpec.radius, norm/2,
                    particles->bSpec.slices, particles->bSpec.stacks, 0);
              }
              glPopMatrix();
            }
          }
        }
      }
    }
    if(enabledMeshes){
        for(index=0; index<particles->nbMeshes; index++){
            mSpec=particles->mSpec+index;
            if(!mSpec->active || mSpec->nbTriangles==0){
                continue;
            }
            type = particles->pType[mSpec->indices[0][0]];
            if(!mSpec->showWires){
                // The following translation is actually needed,
                // otherwise lighting can be wrong sometimes...
                // e.g. meshes and points are enabled (not spheres).
                // Don't know why though...
                glTranslatef(0, 0, 0);
            }
            // Special case where all the particles have the same type and have a constant color.
            if(mSpec->sameType && !particles->pSpec[type].color.isGradColorEnabled()){
                if(!particles->pSpec[type].active) {
                    continue;
                }
                color = getParticleColor(particles, step, mSpec->indices[0][0]);
                if((!opaque&&(color.alpha()==255)) || (opaque&&(color.alpha()<255))){
                    continue;
                }
                if(mSpec->showWires){
                    glDisable(GL_LIGHTING);
                    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
                } else {
                    c0[0]=color.redF();
                    c0[1]=color.greenF();
                    c0[2]=color.blueF();
                    c0[3]=color.alphaF();
                    glMaterialfv(GL_FRONT,GL_DIFFUSE,c0);
                }
                for(indexMesh=0; indexMesh<mSpec->nbTriangles; indexMesh++){
                    triangle=mSpec->indices[indexMesh];
                    if(nbBlanksUsed>0) {
                        if(blankParticle(triangle[0], step) ||
                           blankParticle(triangle[1], step) ||
                           blankParticle(triangle[2], step)) {
                            continue;
                        }
                    }
                    pos1 = particles->pPos[step][triangle[0]];
                    pos2 = particles->pPos[step][triangle[1]];
                    pos3 = particles->pPos[step][triangle[2]];
                    if(pos1[0]==INFINITY || pos2[0]==INFINITY || pos3[0]==INFINITY){
                        continue;
                    }
                    if(mSpec->showWires){
                        glBegin(GL_LINE_LOOP);
                        glVertex3f(pos1[0], pos1[1], pos1[2]);
                        glVertex3f(pos2[0], pos2[1], pos2[2]);
                        glVertex3f(pos3[0], pos3[1], pos3[2]);
                        glEnd();
                    } else {
                        drawTriangle(pos1, pos2, pos3);
                        if(!mSpec->cullBackFace){
                            drawTriangle(pos1, pos3, pos2);
                        }
                    }
                }
                if(mSpec->showWires){
                    glEnable(GL_LIGHTING);
                }
            } else {
                for(indexMesh=0; indexMesh<mSpec->nbTriangles; indexMesh++){
                    triangle=mSpec->indices[indexMesh];
                    type1 = particles->pType[triangle[0]];
                    type2 = particles->pType[triangle[1]];
                    type3 = particles->pType[triangle[2]];
                    showMesh = particles->pSpec[type1].active&&particles->pSpec[type2].active&&
                            particles->pSpec[type3].active;
                    if(nbBlanksUsed>0) showMesh = showMesh&&(!blankParticle(triangle[0], step))
                        &&(!blankParticle(triangle[1], step))&&(!blankParticle(triangle[2], step));
                    if(!showMesh){
                        continue;
                    }
                    pos1 = particles->pPos[step][triangle[0]];
                    pos2 = particles->pPos[step][triangle[1]];
                    pos3 = particles->pPos[step][triangle[2]];
                    if(pos1[0]==INFINITY || pos2[0]==INFINITY || pos3[0]==INFINITY){
                        continue;
                    }
                    color1 = getParticleColor(particles, step, triangle[0]);
                    color2 = getParticleColor(particles, step, triangle[1]);
                    color3 = getParticleColor(particles, step, triangle[2]);
                    if((!opaque&&color1.alpha()==255&&color2.alpha()==255&&color3.alpha()==255) ||
                       (opaque&&(color1.alpha()<255||color2.alpha()<255||color3.alpha()<255))){
                        continue;
                    }
                    if(mSpec->showWires){
                        glDisable(GL_LIGHTING);
                        glBegin(GL_LINE_LOOP);
                        glColor4f(color1.redF(), color1.greenF(), color1.blueF(), color1.alphaF());
                        glVertex3f(pos1[0], pos1[1], pos1[2]);
                        glColor4f(color2.redF(), color2.greenF(), color2.blueF(), color2.alphaF());
                        glVertex3f(pos2[0], pos2[1], pos2[2]);
                        glColor4f(color3.redF(), color3.greenF(), color3.blueF(), color3.alphaF());
                        glVertex3f(pos3[0], pos3[1], pos3[2]);
                        glEnd();
                        glEnable(GL_LIGHTING);
                    } else {
                        pt0[0]=pos1[0]; pt0[1]=pos1[1]; pt0[2]=pos1[2];
                        pt1[0]=pos2[0]; pt1[1]=pos2[1]; pt1[2]=pos2[2];
                        pt2[0]=pos3[0]; pt2[1]=pos3[1]; pt2[2]=pos3[2];
                        c0[0]=color1.redF(); c0[1]=color1.greenF(); c0[2]=color1.blueF(); c0[3]=color1.alphaF();
                        c1[0]=color2.redF(); c1[1]=color2.greenF(); c1[2]=color2.blueF(); c1[3]=color2.alphaF();
                        c2[0]=color3.redF(); c2[1]=color3.greenF(); c2[2]=color3.blueF(); c2[3]=color3.alphaF();
                        drawTriangle(pt0, pt1, pt2, c0, c1, c2);
                        if(!mSpec->cullBackFace){
                            drawTriangle(pt0, pt2, pt1, c0, c2, c1);
                        }
                    }
                }
            }
        }
    }
  }
  if(geometry!=NULL&&enabledGeometry){
    glPushMatrix();
    for(index = k = 0; k<geometry->boxes.size(); k++, index++){
      if(geometry->boxes[k].active){
        if((opaque&&(geometry->boxes[k].color[3]==255))||(!opaque
            &&(geometry->boxes[k].color[3]<255))){
          if(useGlList) glCallList(geomsList+index);
          else drawBox(geometry->boxes[k]);
        }
      }
    }
    for(k = 0; k<geometry->cylinders.size(); k++, index++){
      if(geometry->cylinders[k].active){
        if((opaque&&(geometry->cylinders[k].color[3]==255))||(!opaque
            &&(geometry->cylinders[k].color[3]<255))){
          if(useGlList) glCallList(geomsList+index);
          else drawCylinder(geometry->cylinders[k]);
        }
      }
    }
    for(k = 0; k<geometry->spheres.size(); k++, index++){
      if(geometry->spheres[k].active){
        if((opaque&&(geometry->spheres[k].color[3]==255))||(!opaque&&(geometry->spheres[k].color[3]
            <255))){
          if(useGlList) glCallList(geomsList+index);
          else drawGeosphere(geometry->spheres[k]);
        }
      }
    }
    for(k = 0; k<geometry->lines.size(); k++, index++){
      if(geometry->lines[k].active){
        if((opaque&&(geometry->lines[k].color[3]==255))||(!opaque
            &&(geometry->lines[k].color[3]<255))){
          if(useGlList) glCallList(geomsList+index);
          else drawLines(geometry->lines[k]);
        }
      }
    }
    for(k = 0; k<geometry->meshes.size(); k++, index++){
      if(geometry->meshes[k].active){
        if((opaque&&(geometry->meshes[k].color[3]==255))||(!opaque&&(geometry->meshes[k].color[3]
            <255))){
          if(useGlList) glCallList(geomsList+index);
          else drawMesh(geometry->meshes[k]);
        }
      }
    }
    glPopMatrix();
  }
}

void GLWidget::drawAxis(){
  float xx, yy, zz;
  float bRadius, heightCyl;
  float radiusCone, heightCone;
  float diffuseColor[3][4] = {{1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}};
  float rotAngle[3] = {-90.0, 90.0, 180.0};
  float rotXAxis[3] = {0.0, 1.0, 1.0};
  float rotYAxis[3] = {1.0, 0.0, 0.0};
  float rotZAxis[3] = {0.0, 0.0, 0.0};
  int xyz;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixf(axisMatrix);
  glMatrixMode( GL_MODELVIEW);
  xx = 50;
  yy = 50;
  zz = 0;
  bRadius = 2; //=2  pixels
  heightCyl = 42; //=42 pixels
  radiusCone = 5; //=5  pixels
  heightCone = 8; //=8  pixels

  glDisable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glEnable(GL_LIGHT3);
  glLoadIdentity();
  glTranslatef(xx, yy, zz);
  glRotatef(xRot/ANGLE_DIV, 1.0, 0.0, 0.0);
  glRotatef(yRot/ANGLE_DIV, 0.0, 1.0, 0.0);
  glRotatef(zRot/ANGLE_DIV, 0.0, 0.0, 1.0);
  for(xyz = 0; xyz<3; xyz++){
    glPushMatrix();
    glRotatef(rotAngle[xyz], rotXAxis[xyz], rotYAxis[xyz], rotZAxis[xyz]);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor[xyz]);
    glTranslatef(0.0, 0.0, -heightCyl);
    drawCylinder(bRadius,bRadius,heightCyl,10,1,2);
    glTranslatef(0.0, 0.0, -heightCone);
    drawCylinder(0,radiusCone,heightCone,10,1,2);
    glPopMatrix();
  }
  glEnable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  glDisable(GL_LIGHT3);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GLWidget::resizeGL(int width, int height){

  float positionLight2[] = {-50, -50, 200.0, 1.0};
  float positionLight3[] = {150, 150, 200.0, 1.0};
  float maxdim;
  float t, r;

  viewWidth = width;
  viewHeight = height;
  maxdim = oldMaxDim*1.1;
  if(maxdim>0){
    zNear = maxdim/100;
    zFar = maxdim*100;
    if(!perspective){
      if(width>height){
        bottom = -maxdim/2;
        left = bottom*width/height;
      }else{
        left = -maxdim/2;
        bottom = left*height/width;
      }
      top = -bottom;
      right = -left;
    }
  }else{
    zNear = 1;
    zFar = 10;
    left = bottom = -1;
    right = top = 1;
    totScale = 1;
  }
  glLightfv(GL_LIGHT2, GL_POSITION, positionLight2);
  glLightfv(GL_LIGHT3, GL_POSITION, positionLight3);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.5, width-0.5, 0.5, height-0.5, -50, 50);
  glGetFloatv(GL_PROJECTION_MATRIX, axisMatrix);
  glLoadIdentity();
  if(perspective) {
    t=zNear*tan(fovy/2.);
    r=t*((GLfloat)width/(GLfloat)height);
    glFrustum(-r, r, -t, t, zNear, zFar);
    copyMatrixd44(inverseFrustum(-r, r, -t, t, zNear, zFar), invProjectionMatrix);
  }
  else{
    glOrtho(left, right, bottom, top, 0, zFar);
    glScaled(totScale, totScale, 1.0);
    multMatrixd444(inverseScaled(totScale, totScale, 1.0),
        inverseOrtho(left, right, bottom, top, 0, zFar), invProjectionMatrix);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  timeXPos=viewWidth-getStreamSize(timeStr,charWidth)-5;
  timeYPos=viewHeight-charHeight-5;
}

void GLWidget::zoomFit(){
  GLfloat matrix[16];
  float trans[16];
  float *dims;
  float tmpw, tmph;
  float maxdim;
  float t, r;

  dims = getMinMax();
  if(dims!=NULL){
    glMatrixMode( GL_MODELVIEW);
    glPushMatrix();
    glRotatef(xRot/ANGLE_DIV, 1.0, 0.0, 0.0);
    glRotatef(yRot/ANGLE_DIV, 0.0, 1.0, 0.0);
    glRotatef(zRot/ANGLE_DIV, 0.0, 0.0, 1.0);
    glTranslatef(xCenterSc, yCenterSc, zCenterSc);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();
    for(int k = 0; k<16; k++)
      trans[k] = matrix[k];
    transformBoxDims(dims, trans);
    xCen = -(dims[0]+dims[1])/2;
    yCen = -(dims[2]+dims[3])/2;
    oldMaxDim = maxdim = getMaxDim();
    resizeGL(viewWidth, viewHeight); //to reset left, rigth, etc.
    zNear = maxdim/100;
    zFar = maxdim*100;
    glMatrixMode( GL_PROJECTION);
    glLoadIdentity();
    if(perspective){
      //it's not the perfect way to do it... at all!
      depth = -1.5*maxdim;
      t=zNear*tan(fovy/2.);
      r=t*((GLfloat)viewWidth/(GLfloat)viewHeight);
      glFrustum(-r, r, -t, t, zNear, zFar);
      copyMatrixd44(inverseFrustum(-r, r, -t, t, zNear, zFar), invProjectionMatrix);
    }else{
      depth0 = -1.5*maxdim;
      tmpw = tmph = -1;
      if(dims[1]>dims[0]) tmpw = (right-left)/(dims[1]-dims[0]);
      if(dims[3]>dims[2]) tmph = (top-bottom)/(dims[3]-dims[2]);
      if(tmpw>0||tmph>0){
        if(tmpw<=0) totScale = tmph;
        else if(tmph<=0) totScale = tmpw;
        else totScale = tmpw<tmph ? tmpw : tmph;
      }
      totScale /= 1.1;
      glOrtho(left, right, bottom, top, 0, zFar);
      glScaled(totScale, totScale, 1.0);
      multMatrixd444(inverseScaled(totScale, totScale, 1.0),
          inverseOrtho(left, right, bottom, top, 0, zFar), invProjectionMatrix);
    }
    glMatrixMode(GL_MODELVIEW);
    delete[] dims;
    updateGL();
  }
}

void GLWidget::center(){
  float *dims;
  dims = getMinMax();
  if(dims!=NULL){
    xCenterSc = -(dims[0]+dims[1])/2;
    yCenterSc = -(dims[2]+dims[3])/2;
    zCenterSc = -(dims[4]+dims[5])/2;
    delete[] dims;
    updateGL();
    emit centerChanged(-xCenterSc, -yCenterSc, -zCenterSc);
  }
}

void GLWidget::setCenter(float newXCenterSc, float newYCenterSc, float newZCenterSc){
  xCenterSc = -newXCenterSc;
  yCenterSc = -newYCenterSc;
  zCenterSc = -newZCenterSc;
  updateGL();
}

void GLWidget::setRotAngles(float newXRot, float newYRot, float newZRot){
  xRot = (int)(newXRot*ANGLE_DIV);
  yRot = (int)(newYRot*ANGLE_DIV);
  zRot = (int)(newZRot*ANGLE_DIV);
  updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event){
  event->accept();
  if(event->key()==Qt::Key_Shift) shiftKeyPressed = true;
  else event->ignore();
}

void GLWidget::keyReleaseEvent(QKeyEvent *event){
  event->accept();
  if(event->key()==Qt::Key_Shift) shiftKeyPressed = false;
  else event->ignore();
}

void GLWidget::mousePressEvent(QMouseEvent *event){
  lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event){
  if((event->button()&Qt::LeftButton)||(event->button()&Qt::RightButton))
    emit rotAnglesChanged(xRot/ANGLE_DIV, yRot/ANGLE_DIV, zRot/ANGLE_DIV);

  //test nico
  if(event->button()&Qt::RightButton){
    GLdouble v[4];
    GLdouble m[16];
    int i;

    copyMatrixd44(translated(-xCenterSc, -yCenterSc, -zCenterSc),m);
    multMatrixd444(m,rotated(-zRot/ANGLE_DIV,0,0,1),m);
    multMatrixd444(m,rotated(-yRot/ANGLE_DIV,0,1,0),m);
    multMatrixd444(m,rotated(-xRot/ANGLE_DIV,1,0,0),m);
    if(perspective)
      multMatrixd444(m,translated(-xCen, -yCen, -depth),m);
    else
      multMatrixd444(m,translated(-xCen, -yCen, -depth0),m);

    v[0]=(2./viewWidth)*event->x()-1.;  //go back to normalize coordinates
    v[1]=(2./viewHeight)*(viewHeight-event->y())-1.;
    v[2]=-1;
    v[3]=1;
    multMatrixd441(invProjectionMatrix,v,v2);
    for(i=0;i<4;i++)
      v2[i]/=v2[3];
    multMatrixd441(m,v2,v2);

    v[2]=1;
    multMatrixd441(invProjectionMatrix,v,v3);
    for(i=0;i<4;i++)
      v3[i]/=v3[3];
    multMatrixd441(m,v3,v3);
    updateGL();
  }
  //end test nico
}

void GLWidget::mouseMoveEvent(QMouseEvent *event){
  static int coef = (int)(ANGLE_DIV*360);
  int dx = event->x()-lastPos.x();
  int dy = event->y()-lastPos.y();
  bool doUpdate = false;

  if(event->buttons()&Qt::LeftButton){
    if(dy!=0){
      xRot = (xRot+(shiftKeyPressed ? 1 : 12)*dy)%coef;
      while(xRot<0)
        xRot += coef;
      doUpdate = true;
    }
    if(dx!=0){
      zRot = (zRot+(shiftKeyPressed ? 1 : 12)*dx)%coef;
      while(zRot<0)
        zRot += coef;
      doUpdate = true;
    }
  }else if(event->buttons()&Qt::RightButton){
    if(dx!=0){
      yRot = (yRot+(shiftKeyPressed ? 1 : 12)*dx)%coef;
      while(yRot<0)
        yRot += coef;
      doUpdate = true;
    }
  }else if(event->buttons()&Qt::MidButton){
    float coef;
    if(perspective){
      coef = depth*tan(fovy/2.)/(viewHeight/2);
      xCen -= dx*coef;
      yCen += dy*coef;
    }else{
      coef = (right-left)/totScale/viewWidth;
      xCen += dx*coef;
      yCen -= dy*coef;
    }
    doUpdate = true;
  }
  lastPos = event->pos();
  if(doUpdate==true) updateGL();
}

void GLWidget::wheelEvent(QWheelEvent *event){
  float isoScale;
  if(perspective) depth *= pow(2, -event->delta()/360.0/(shiftKeyPressed ? 20 : 1));
  else{
    isoScale = pow(2, event->delta()/360.0/(shiftKeyPressed ? 20 : 1));
    totScale *= isoScale;
    glMatrixMode(GL_PROJECTION);
    glScalef(isoScale, isoScale, 1.0);
    multMatrixd444(inverseScaled(isoScale, isoScale, 1.0),
        invProjectionMatrix, invProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
  }
  updateGL();
}

void GLWidget::createParticlesList(){
  partsList = glGenLists(particles->nbTypes);
  for(int i = 0; i<particles->nbTypes; i++){
    glNewList(partsList+i, GL_COMPILE);
    drawGeosphere(particles->pSpec[i].radius, particles->pSpec[i].segs);
    glEndList();
  }
}

void GLWidget::loadParticles(Particles *part){
  particles = part;
  nbParticles = part->nbParticles;
  nbBonds = part->nbBonds;
  nbMeshes = part->nbMeshes;
  nbTypes = part->nbTypes;
  createParticlesList();
  enabledAxis = true;
  perspective = false;
  enabledParticles = true;
  enabledBonds = true;
  enabledMeshes = true;
  step = 0;
  center();
  xRot = -(int)(90*ANGLE_DIV);
  yRot = 0;
  zRot = 0;
  xCen = 0.0;
  yCen = 0.0;
  oldMaxDim = getMaxDim();
  depth = depth0 = -1.5*oldMaxDim;
  totScale = 1;
  resizeGL(viewWidth, viewHeight);
  resetBlanks();
  resetTrajectory();
  updateTimeStr(step);
  updateGL();
}

void GLWidget::clearParticles(){
  if(particles!=NULL){
    glDeleteLists(partsList, nbTypes);
    step = 0;
    nbParticles = 0;
    nbBonds = 0;
    nbMeshes = 0;
    nbTypes = 0;
    particles = NULL;
    updateTimeStr(step);
  }
}

void GLWidget::loadGeometry(MyGeometry *geo){
  geometry = geo;
  createGeometryList();
  enabledGeometry = true;
  center();
  xRot = -(int)(90*ANGLE_DIV);
  yRot = 0;
  zRot = 0;
  xCen = 0.0;
  yCen = 0.0;
  oldMaxDim = getMaxDim();
  depth = depth0 = -1.5*oldMaxDim;
  totScale = 1;
  resizeGL(viewWidth, viewHeight);
  updateGL();
}

void GLWidget::clearGeometry(){
  if(geometry!=NULL){
    glDeleteLists(geomsList, geometry->nbObj);
    geometry = NULL;
  }
}

void GLWidget::goToStep(int nextStep){
  if(particles==NULL){
    if(step!=0){
      step = 0;
      updateTimeStr(step);
      emit stepChanged(step);
      updateGL();
    }
    return;
  }
  if(nextStep>=(int)(particles->nbSteps)){
    if(step!=(int)(particles->nbSteps-1)){
      step = particles->nbSteps-1;
      updateTimeStr(step);
      emit stepChanged(step);
      updateGL();
    }
  }else if(nextStep<0){
    if(step!=0){
      step = 0;
      updateTimeStr(step);
      emit stepChanged(step);
      updateGL();
    }
  }else if(step!=nextStep){
    step = nextStep;
    updateTimeStr(step);
    emit stepChanged(step);
    updateGL();
  }
}

void GLWidget::setEnabledAxis(bool enabled){
  enabledAxis = enabled;
  updateGL();
}

void GLWidget::switchView(bool persp){
  perspective = persp;
  xCen = yCen = 0;
  resizeGL(viewWidth, viewHeight);
  updateGL();
}

float GLWidget::getMaxDim(){
  float dimx, dimy, dimz, *dims;
  dims = getMinMax();
  if(dims!=NULL){
    dimx = dims[1]-dims[0];
    dimy = dims[3]-dims[2];
    dimz = dims[5]-dims[4];
    delete[] dims;
    return sqrt(dimx*dimx+dimy*dimy+dimz*dimz);
  }
  return 0;
}

void GLWidget::setEnabledBonds(bool enabled){
  enabledBonds = enabled;
  updateGL();
}

void GLWidget::setEnabledParticles(bool enabled){
  enabledParticles = enabled;
  updateGL();
}

void GLWidget::setEnabledMeshes(bool enabled){
  enabledMeshes = enabled;
  updateGL();
}

void GLWidget::setEnabledGeometry(bool enabled){
  enabledGeometry = enabled;
  updateGL();
}

void GLWidget::enableGlList(bool enabled){
  useGlList = enabled;
}

void GLWidget::updateBackground(MyColor color){
  bgColor = color;
  qglClearColor(bgColor());
  updateGL();
}

void GLWidget::setAnimTrajectory(bool enabled){
  animTra = enabled;
  updateGL();
}

void GLWidget::resetBlanks(){
  for(int i = 0; i<NB_BLANK; i++){
    blanks[i].active = false;
    blanks[i].allTypes = true;
    blanks[i].typeIndex = 0;
    blanks[i].varIndex = 0;
    blanks[i].opIndex = 0;
    blanks[i].value = 0;
  }
  nbBlanksUsed = 0;
}

void GLWidget::setBlank(int index, Blank blank){
  if(index<NB_BLANK){
    blanks[index] = blank;
    nbBlanksUsed = 0;
    for(int i = 0; i<NB_BLANK; i++){
      if(blanks[i].active) blankIndex[nbBlanksUsed++] = i;
    }
    updateGL();
  }
}

void GLWidget::resetTrajectory(){ //check TrajectoryDialog::setDialog()
  for(int i = 0; i<NB_TRAJECTORIES; i++){
    trajectory[i].active = false;
    trajectory[i].single = (i==0 ? false : true);
    trajectory[i].index = 0;
    trajectory[i].color = MyColor(0, 0, 255);
  }
  nbTrajActive = 0;
}

void GLWidget::setTrajectory(int index, Trajectory traj){
  int i;
  if(index<NB_TRAJECTORIES){
    trajectory[index] = traj;
    for(i = 0; i<NB_TRAJECTORIES; i++){
      if(trajectory[i].single!=(i==0 ? false : true)){
        QMessageBox::critical(NULL, QObject::tr("GLWidget"),
                              QObject::tr("Problem with trajectories"));
        exit(1);
      }
    }
    nbTrajActive = 0;
    for(i = 0; i<NB_TRAJECTORIES; i++){
      if(trajectory[i].active){
        trajIndex[nbTrajActive++] = i;
        if(i==0) break;
      }
    }
    updateGL();
  }
}

void GLWidget::setPartSpec(int type, PartSpec spec){
  if(type>=0&&type<nbTypes){
    particles->pSpec[type] = spec;
    glDeleteLists(partsList, nbTypes);
    createParticlesList();
    updateGL();
  }
}

void GLWidget::setAllPartSpec(PartSpec *specs){
  if(particles!=NULL){
    memcpy(particles->pSpec, specs, nbTypes*sizeof(PartSpec));
    glDeleteLists(partsList, nbTypes);
    createParticlesList();
    updateGL();
  }
}

void GLWidget::setBondSpec(BondSpec spec){
  particles->bSpec = spec;
  updateGL();
}

void GLWidget::setMeshSpec(int index, MeshSpec spec){
    if(index>=0 && index<nbMeshes){
        particles->mSpec[index] = spec;
        updateGL();
    }
}

void GLWidget::setAllMeshSpec(MeshSpec *specs){
    memcpy(particles->mSpec, specs, nbMeshes*sizeof(MeshSpec));
    updateGL();
}

void GLWidget::resetGeometry(){
  if(geometry!=NULL){
    glDeleteLists(geomsList, geometry->nbObj);
    createGeometryList();
    updateGL();
  }
}

/*creates elements from .geo files*/
void GLWidget::createGeometryList(){
  int i, index;

  geomsList = glGenLists(geometry->nbObj);
  index = 0;
  for(i = 0; i<geometry->boxes.size(); i++, index++){
    glNewList(geomsList+index, GL_COMPILE);
    drawBox(geometry->boxes[i]);
    glEndList();
  }
  for(i = 0; i<geometry->cylinders.size(); i++, index++){
    glNewList(geomsList+index, GL_COMPILE);
    drawCylinder(geometry->cylinders[i]);
    glEndList();
  }
  for(i = 0; i<geometry->spheres.size(); i++, index++){
    glNewList(geomsList+index, GL_COMPILE);
    drawGeosphere(geometry->spheres[i]);
    glEndList();
  }
  for(i = 0; i<geometry->lines.size(); i++, index++){
    glNewList(geomsList+index, GL_COMPILE);
    drawLines(geometry->lines[i]);
    glEndList();
  }
  for(i = 0; i<geometry->meshes.size(); i++, index++){
    glNewList(geomsList+index, GL_COMPILE);
    drawMesh(geometry->meshes[i]);
    glEndList();
  }
}

/* draw the particles trajectory.*/
void GLWidget::drawTrajectory(Trajectory &traj){
  int theStep, startStep, endStep;
  int index, startIndex, endIndex;
  QColor c;

  if(particles!=NULL){
    if(animTra){
      if(traj.nbPrevSteps==0 || step<traj.nbPrevSteps)
        startStep=0;
      else
        startStep=step-traj.nbPrevSteps;
      endStep=step;
    }
    else{
      startStep=0;
      endStep=particles->nbSteps-1;
    }
    if(endStep>=(int)particles->nbSteps||traj.index<0||traj.index>=(int)particles->nbParticles) return;
    if(traj.single) startIndex = endIndex = traj.index;
    else{
      startIndex = 0;
      endIndex = particles->nbParticles-1;
    }
    glDisable( GL_LIGHTING);
    for(index = startIndex; index<=endIndex; index++){
      if(particles->pSpec[particles->pType[index]].active){
        glBegin( GL_LINE_STRIP);
        for(theStep = startStep; theStep<=endStep; theStep++){
          if(particles->pPos[theStep][index][0]!=INFINITY&&!blankParticle(index, theStep)){
            c = getColor(particles, traj.color, theStep, index);
            glColor3f(c.redF(), c.greenF(), c.blueF());
            glVertex3fv(particles->pPos[theStep][index]);
          }else{
            glEnd();
            glBegin(GL_LINE_STRIP);
          }
        }
        glEnd();
      }
    }
    glEnable(GL_LIGHTING);
  }
}

/*returns a array of 6 floats: {minx,maxx,miny,maxy,minz,maxz}*/
/*returns null if scene is empty*/
float* GLWidget::getMinMax(){
  float minx = 0, maxx = 0;
  float miny = 0, maxy = 0;
  float minz = 0, maxz = 0;
  float *tmps;
  float *out = NULL;
  float pm1[2] = {-1, 1};
  bool foundOne;
  unsigned int index;
  int i, j;

  tmps = new float[6];
  if(particles!=NULL&&(enabledParticles||enabledBonds||enabledMeshes||nbTrajActive>0)){
    if(particles->nbParticles>0){
      for(index = 0; index<particles->nbParticles; index++){
        if(particles->pPos[step][index][0]!=INFINITY&&!blankParticle(index, step)){
          out = new float[6];
          for(j = 0; j<6; j++)
            tmps[j] = particles->pPos[step][index][j/2]+pm1[j&1]
                *particles->pSpec[particles->pType[index]].radius;
          minx = tmps[0];
          maxx = tmps[1];
          miny = tmps[2];
          maxy = tmps[3];
          minz = tmps[4];
          maxz = tmps[5];
          break;
        }
      }
      for(index++; index<particles->nbParticles; index++){
        if(particles->pPos[step][index][0]!=INFINITY&&!blankParticle(index, step)){
          for(j = 0; j<6; j++)
            tmps[j] = particles->pPos[step][index][j/2]+pm1[j&1]
                *particles->pSpec[particles->pType[index]].radius;
          minx = tmps[0]<minx ? tmps[0] : minx;
          maxx = tmps[1]>maxx ? tmps[1] : maxx;
          miny = tmps[2]<miny ? tmps[2] : miny;
          maxy = tmps[3]>maxy ? tmps[3] : maxy;
          minz = tmps[4]<minz ? tmps[4] : minz;
          maxz = tmps[5]>maxz ? tmps[5] : maxz;
        }
      }
    }
  }
  if(geometry!=NULL&&enabledGeometry){
    if(geometry->nbObj>0){
      if(out==NULL){
        foundOne = false;
        for(i = 0; i<geometry->boxes.size(); i++){
          if(geometry->boxes[i].active==true){
            getMinMaxBox(geometry->boxes[i], tmps);
            foundOne = true;
            break;
          }
        }
        if(!foundOne){
          for(i = 0; i<geometry->cylinders.size(); i++){
            if(geometry->cylinders[i].active==true){
              getMinMaxCylinder(geometry->cylinders[i], tmps);
              foundOne = true;
              break;
            }
          }
        }
        if(!foundOne){
          for(i = 0; i<geometry->spheres.size(); i++){
            if(geometry->spheres[i].active==true){
              getMinMaxSphere(geometry->spheres[i], tmps);
              foundOne = true;
              break;
            }
          }
        }
        if(!foundOne){
          for(i = 0; i<geometry->lines.size(); i++){
            if(geometry->lines[i].active==true){
              getMinMaxLines(geometry->lines[i], tmps);
              foundOne = true;
              break;
            }
          }
        }
        if(!foundOne){
          for(i = 0; i<geometry->meshes.size(); i++){
            if(geometry->meshes[i].active==true){
              getMinMaxMesh(geometry->meshes[i], tmps);
              foundOne = true;
              break;
            }
          }
        }
        if(foundOne){
          out = new float[6];
          minx = tmps[0];
          maxx = tmps[1];
          miny = tmps[2];
          maxy = tmps[3];
          minz = tmps[4];
          maxz = tmps[5];
        }
      }
      if(out!=NULL){
        for(i = 0; i<geometry->boxes.size(); i++){
          if(geometry->boxes[i].active){
            getMinMaxBox(geometry->boxes[i], tmps);
            minx = tmps[0]<minx ? tmps[0] : minx;
            maxx = tmps[1]>maxx ? tmps[1] : maxx;
            miny = tmps[2]<miny ? tmps[2] : miny;
            maxy = tmps[3]>maxy ? tmps[3] : maxy;
            minz = tmps[4]<minz ? tmps[4] : minz;
            maxz = tmps[5]>maxz ? tmps[5] : maxz;
          }
        }
        for(i = 0; i<geometry->cylinders.size(); i++){
          if(geometry->cylinders[i].active){
            getMinMaxCylinder(geometry->cylinders[i], tmps);
            minx = tmps[0]<minx ? tmps[0] : minx;
            maxx = tmps[1]>maxx ? tmps[1] : maxx;
            miny = tmps[2]<miny ? tmps[2] : miny;
            maxy = tmps[3]>maxy ? tmps[3] : maxy;
            minz = tmps[4]<minz ? tmps[4] : minz;
            maxz = tmps[5]>maxz ? tmps[5] : maxz;
          }
        }
        for(i = 0; i<geometry->spheres.size(); i++){
          if(geometry->spheres[i].active){
            getMinMaxSphere(geometry->spheres[i], tmps);
            minx = tmps[0]<minx ? tmps[0] : minx;
            maxx = tmps[1]>maxx ? tmps[1] : maxx;
            miny = tmps[2]<miny ? tmps[2] : miny;
            maxy = tmps[3]>maxy ? tmps[3] : maxy;
            minz = tmps[4]<minz ? tmps[4] : minz;
            maxz = tmps[5]>maxz ? tmps[5] : maxz;
          }
        }
        for(i = 0; i<geometry->lines.size(); i++){
          if(geometry->lines[i].active){
            getMinMaxLines(geometry->lines[i], tmps);
            minx = tmps[0]<minx ? tmps[0] : minx;
            maxx = tmps[1]>maxx ? tmps[1] : maxx;
            miny = tmps[2]<miny ? tmps[2] : miny;
            maxy = tmps[3]>maxy ? tmps[3] : maxy;
            minz = tmps[4]<minz ? tmps[4] : minz;
            maxz = tmps[5]>maxz ? tmps[5] : maxz;
          }
        }
        for(i = 0; i<geometry->meshes.size(); i++){
          if(geometry->meshes[i].active){
            getMinMaxMesh(geometry->meshes[i], tmps);
            minx = tmps[0]<minx ? tmps[0] : minx;
            maxx = tmps[1]>maxx ? tmps[1] : maxx;
            miny = tmps[2]<miny ? tmps[2] : miny;
            maxy = tmps[3]>maxy ? tmps[3] : maxy;
            minz = tmps[4]<minz ? tmps[4] : minz;
            maxz = tmps[5]>maxz ? tmps[5] : maxz;
          }
        }
      }
    }
  }
  delete[] tmps;
  if(out!=NULL){
    out[0] = minx;
    out[1] = maxx;
    out[2] = miny;
    out[3] = maxy;
    out[4] = minz;
    out[5] = maxz;
  }
  return out;
}

/*return data[0]+data[1]<<8+...+data[nbytes-1]<<(8*(nbytes-1)). data is incremented.*/
static unsigned int extractData(unsigned char **data, int nbytes){
  unsigned int out;
  int i, offset=0;

  for(i=0, out=0; i<nbytes; i++, offset+=8)
    out|=(*(*data)++)<<offset;
  return out;
}

/*reads a .bmp file and returns an Image structure containing the pixels*/
Image* loadBMP(unsigned char *fdata){
  Image *im=NULL;
  struct {
    unsigned char  id[2];     // should be "BM"
    unsigned int   size;      // file size
    unsigned short reserved1; // reserved
    unsigned short reserved2; // reserved
    unsigned int   offset;    // offset giving beginning of data
  } header;
  struct {
    unsigned int   headerSize;
    int            width;
    int            height;
    unsigned short numColorPlanes;
    unsigned short bitsPerPixel;
    unsigned int   compression;
    unsigned int   imageSize;
    int            hResolution;
    int            vResolution;
    unsigned int   numColorPalette;
    unsigned int   numImportantColor;
  } infoHeader;
  long   rowOffset;
  unsigned char (*data)[3],tmp;

  // read BMP header
  header.id[0]=*fdata++;
  header.id[1]=*fdata++;
  header.size=extractData(&fdata, 4);
  header.reserved1=extractData(&fdata, 2);
  header.reserved2=extractData(&fdata, 2);
  header.offset=extractData(&fdata, 4);
  if(header.offset!=54)
    return NULL;

  // read BMP info header
  infoHeader.headerSize=extractData(&fdata, 4);
  infoHeader.width=extractData(&fdata, 4);
  infoHeader.height=extractData(&fdata, 4);
  infoHeader.numColorPlanes=extractData(&fdata, 2);
  infoHeader.bitsPerPixel=extractData(&fdata, 2);
  infoHeader.compression=extractData(&fdata, 4);
  infoHeader.imageSize=extractData(&fdata, 4);
  infoHeader.hResolution=extractData(&fdata, 4);
  infoHeader.vResolution=extractData(&fdata, 4);
  infoHeader.numColorPalette=extractData(&fdata, 4);
  infoHeader.numImportantColor=extractData(&fdata, 4);
  if(infoHeader.headerSize!=40 || infoHeader.numColorPlanes!=1 ||
     infoHeader.compression!=0 || infoHeader.numColorPalette!=0)
    return NULL;
  // read data
  im=(Image*)calloc(1,sizeof(Image));
  im->height=abs(infoHeader.height);
  im->width=abs(infoHeader.width);
  im->data=(unsigned char(*)[3])calloc(im->height*im->width,sizeof(unsigned char[3]));
  rowOffset=(4-((im->width*3)&3))&3; // &3 <=> %4
  if(infoHeader.height<0){
    for(data=im->data;data<im->data+im->width*im->height;data+=im->width){
      memcpy(data, fdata, 3*im->width);
      fdata+=3*im->width+rowOffset;
    }
  }
  else{
    for(data=im->data+im->width*(im->height-1);data>=im->data;data-=im->width){
      memcpy(data, fdata, 3*im->width);
      fdata+=3*im->width+rowOffset;
    }
  }
  for(data=im->data;data<im->data+im->width*im->height;data++){
    tmp=(*data)[0];
    (*data)[0]=(*data)[2];
    (*data)[2]=tmp;
  }

  return im;
}

/*create fontList from the BMP file located at fontBmp, containing the representation*/
/*of 95 ASCII characters, starting from the space (0x20) and ending with '~' (0x7E)*/
/*the corresponding .txt file contains the size of each character*/
/*The corresponding .txt file must have the same name. Only the extension changes.*/
/*the origin for each character is located at the bottom left corner*/
void GLWidget::loadFont(const char *fontBmp){
  char *fontTxt;
  Image *im;
  int rowOffset,colOffset;
  int c,i,j,indexBmp,indexTxt;

  // first, check if .bmp and .txt file exist.
  for(indexBmp=0; indexBmp<(int)(sizeof(FONTS_FILE_NAMES)/sizeof(*FONTS_FILE_NAMES)); indexBmp++){
    if(!strcmp(fontBmp, FONTS_FILE_NAMES[indexBmp]))
      break;
  }
  if(indexBmp==(int)(sizeof(FONTS_FILE_NAMES)/sizeof(*FONTS_FILE_NAMES)))
    return;
  fontTxt=new char[strlen(fontBmp)+1];
  strcpy(fontTxt,fontBmp);
  strcpy(fontTxt+strlen(fontTxt)-3,"txt");
  for(indexTxt=0; indexTxt<(int)(sizeof(FONTS_FILE_NAMES)/sizeof(*FONTS_FILE_NAMES)); indexTxt++){
    if(!strcmp(fontTxt, FONTS_FILE_NAMES[indexTxt]))
      break;
  }
  delete [] fontTxt;
  if(indexTxt==(int)(sizeof(FONTS_FILE_NAMES)/sizeof(*FONTS_FILE_NAMES)))
    return;

  fontTxt=(char*)FONTS_FILE_DATA[indexTxt];
  for(c=0;c<95;c++)
    charWidth[c]=strtol(fontTxt, &fontTxt, 10);
  im=loadBMP(FONTS_FILE_DATA[indexBmp]);
  if(im==NULL)
    return;
  charHeight=im->height/3;

  rowOffset=-charHeight*im->width;
  colOffset=0;
  if(fontList){  //clean before creating new list
    glDeleteLists(fontList, 95);
    fontList=0;
  }
  if(fontPixelsPos){ //used only for GLWidget::renderPixmap()
    for(c=0;c<95;c++)
      free(fontPixelsPos[c]);
    free(fontPixelsPos);
    fontPixelsPos=NULL;
  }
  if(fontPixelsNum){
    free(fontPixelsNum);
    fontPixelsNum=NULL;
  }
  fontList = glGenLists(95);
  fontPixelsPos=(int(**)[2])calloc(95,sizeof(int(*)[2]));
  fontPixelsNum=(int*)calloc(95,sizeof(int));
  for(c=0;c<95;c++){
    if(c%32==0){
      rowOffset+=charHeight*im->width;
      colOffset=0;
    }
    glNewList(fontList+c,GL_COMPILE);
    glPointSize(1);
    glBegin(GL_POINTS);
    fontPixelsNum[c]=0;
    fontPixelsPos[c]=(int(*)[2])calloc(charHeight*charWidth[c],sizeof(int[2]));
    for(j=0;j<charHeight;j++)
      for(i=0;i<charWidth[c];i++)
        if(im->data[i+colOffset+j*im->width+rowOffset][0]==0){
          glVertex2i(i,charHeight-1-j);
          fontPixelsPos[c][fontPixelsNum[c]][0]=i;
          fontPixelsPos[c][fontPixelsNum[c]++][1]=charHeight-1-j;
        }
    fontPixelsPos[c]=(int(*)[2])realloc(fontPixelsPos[c],fontPixelsNum[c]*sizeof(int[2]));
    glEnd();
    glEndList();
    colOffset+=charWidth[c];
  }
  delete [] im->data;
  delete im;
  if(isInitialized){
    timeXPos=viewWidth-getStreamSize(timeStr,charWidth)-6;
    timeYPos=viewHeight-charHeight-5;
    updateGL();
  }
}

/* prints format on screen */
/* (x,y) : position, in pixels, of the top left rectangle containing text */
/* (0,0) corresponds to the top left corner of the window */
/* format and "..." can be used exactly like in printf(format, ...) */
void GLWidget::printScr(int x, int y, const char *format, ...){
  static char string[256];
  unsigned char c;
  va_list ap;
  int i,j,size;

  if(fontList){
    va_start(ap,format);
    vsprintf(string,format,ap);
    va_end(ap);
    size=strlen(string);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(axisMatrix); // use the same projection matrix as the one used to draw the axis
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(x,viewHeight-y-charHeight,0);
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPointSize(1);
    glColor3fv(fontColor);
    for(i=0;i<size;i++){
      c=(unsigned char)string[i];
      if(c>=0x20 && c<=0x7E){
        c-=0x20;
        if(useGlList) glCallList(fontList+c); //Cf GLWidget::renderPixmap doesn't like lists
        else{
          glBegin(GL_POINTS);
          for(j=0;j<fontPixelsNum[c];j++)
            glVertex2i(fontPixelsPos[c][j][0],fontPixelsPos[c][j][1]);
          glEnd();
        }
        glTranslatef(charWidth[c],0,0);
      }
      else if(c=='\n'){
        glPopMatrix();
        glTranslatef(0,-charHeight,0);
        glPushMatrix();
      }
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
}

void GLWidget::updateTimeStr(unsigned int theStep){
  static char unit[8][3]={"ps","ns","us","ms","s ","m ","h ","d "};
  if(particles!=NULL){
    if(theStep<particles->nbSteps){
      float t=particles->time[theStep]*1e12;
      int i;
      for(i=0;i<4;i++){
        if(t<1e3)
          break;
        t/=1e3;
      }
      if(i<4 && t>=999.995){
        i++;
        t/=1e3;
      }
      if(i==4 && t>=59.995){
        t/=60;
        i++;
        if(t>=59.995){
          t/=60;
          i++;
          if(t>=23.995){
            t/=24;
            i++;
          }
        }
      }
      sprintf(timeStr,"%5.2f %s",t,unit[i]);
    }
  }
  else
    sprintf(timeStr,"0 s");
  timeXPos=viewWidth-getStreamSize(timeStr,charWidth)-6;
}

void GLWidget::setShowText(bool show){
  if(show!=showText){
    showText=show;
    updateGL();
  }
}

void GLWidget::updateTextColor(QColor color){
  fontColor[0] = color.redF();
  fontColor[1] = color.greenF();
  fontColor[2] = color.blueF();
  updateGL();
}

void GLWidget::updateFps(int newFps){
  if(fps!=newFps){
    if(fpsTimer->isActive())
      fpsTimer->stop();
    fps=newFps;
    fpsTimer->start();
    updateGL();
  }
}
