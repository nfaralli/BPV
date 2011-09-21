#include "glgeometry.h"

/*
typedef struct{
	float **vertices;
	int   **faces;
	int   nbVerts;
	int   nbFaces;
} Geosphere;
*/


typedef struct{
  float (*vertices)[3]; //array of float[3];
  int   (*faces)[3];
  int   nbVerts;
  int   nbFaces;
} Geosphere;


static Geosphere* getIcosahedron(float radius);
static void       relaxGeosphere(Geosphere *geo);
static Geosphere* getGeosphere(Geosphere *geoIni, int nbSegs);

/*draw a box according to the structure MyBox*/
void drawBox(MyBox &box){
	GLfloat diffuseColor[4];
	float x,y,z;
	float dx,dy,dz;
	int i,j,k;
	GLfloat matrix[16];
	
  glEnable(GL_NORMALIZE);
	for(k=0;k<4;k++)
		diffuseColor[k]=box.color[k]/255.;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=box.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	dx=box.width/box.widthsegs;
	dy=box.length/box.lengthsegs;
	dz=box.height/box.heightsegs;
	z=box.height;
  glNormal3f(0,0,1);
	for(i=0;i<box.lengthsegs;i++){
		y=box.length/2-i*dy;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.widthsegs+1;j++){
			x=-box.width/2+j*dx;
			glVertex3f(x,y,z);
			glVertex3f(x,y-dy,z);
		}
		glEnd();
	}
	z=0;
	glNormal3f(0,0,-1);
	for(i=0;i<box.lengthsegs;i++){
		y=-box.length/2+i*dy;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.widthsegs+1;j++){
			x=-box.width/2+j*dx;
			glVertex3f(x,y,z);
			glVertex3f(x,y+dy,z);
		}
		glEnd();
	}
	y=box.length/2;
	glNormal3f(0,1,0);
	for(i=0;i<box.heightsegs;i++){
		z=box.height-i*dz;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.widthsegs+1;j++){
			x=box.width/2-j*dx;
			glVertex3f(x,y,z);
			glVertex3f(x,y,z-dz);
		}
		glEnd();
	}
	y=-box.length/2;
	glNormal3f(0,-1,0);
	for(i=0;i<box.heightsegs;i++){
		z=i*dz;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.widthsegs+1;j++){
			x=box.width/2-j*dx;
			glVertex3f(x,y,z);
			glVertex3f(x,y,z+dz);
		}
		glEnd();
	}
	x=box.width/2;
	glNormal3f(1,0,0);
	for(i=0;i<box.heightsegs;i++){
		z=box.height-i*dz;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.lengthsegs+1;j++){
			y=-box.length/2+j*dy;
			glVertex3f(x,y,z);
			glVertex3f(x,y,z-dz);
		}
		glEnd();
	}
	x=-box.width/2;
	glNormal3f(-1,0,0);
	for(i=0;i<box.heightsegs;i++){
		z=i*dz;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<box.lengthsegs+1;j++){
			y=-box.length/2+j*dy;
			glVertex3f(x,y,z);
			glVertex3f(x,y,z+dz);
		}
		glEnd();
	}
  glPopMatrix();
  glDisable(GL_NORMALIZE);
}

/*draw a cylinder according to the structure MyCylinder*/
void drawCylinder(MyCylinder &cylinder){
	GLfloat diffuseColor[4];
	float z,dz;
	float radius,dtheta,dr,r1,r2;
	float *ctheta,*stheta;
	int i,j,k;
	GLfloat matrix[16];
	
  glEnable(GL_NORMALIZE);
	for(k=0;k<4;k++)
		diffuseColor[k]=cylinder.color[k]/255.;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=cylinder.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	dtheta=2*PI/cylinder.sides;
	ctheta=new float[cylinder.sides+1];
	stheta=new float[cylinder.sides+1];
	for(j=0;j<cylinder.sides+1;j++){
		ctheta[j]=cos(j*dtheta);
		stheta[j]=sin(j*dtheta);
	}
	radius=cylinder.radius;
	dr=radius/cylinder.capsegs;
	dz=cylinder.height/cylinder.heightsegs;
	z=cylinder.height;
	glNormal3f(0,0,1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,z);
	for(j=0;j<cylinder.sides+1;j++)
		glVertex3f(dr*ctheta[j],dr*stheta[j],z);
	glEnd();
	for(i=1;i<cylinder.capsegs;i++){
		glBegin(GL_QUAD_STRIP);
		r1=i*dr;
		r2=(i+1)*dr;
		for(j=0;j<cylinder.sides+1;j++){
			glVertex3f(r1*ctheta[j],r1*stheta[j],z);
			glVertex3f(r2*ctheta[j],r2*stheta[j],z);
		}
		glEnd();
	}
	z=0;
	glNormal3f(0,0,-1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,z);
	for(j=cylinder.sides;j>=0;j--)
		glVertex3f(dr*ctheta[j],dr*stheta[j],z);
	glEnd();
	for(i=1;i<cylinder.capsegs;i++){
		glBegin(GL_QUAD_STRIP);
		r1=i*dr;
		r2=(i+1)*dr;
		for(j=cylinder.sides;j>=0;j--){
			glVertex3f(r1*ctheta[j],r1*stheta[j],z);
			glVertex3f(r2*ctheta[j],r2*stheta[j],z);
		}
		glEnd();
	}
	for(i=0;i<cylinder.heightsegs;i++){
		z=i*dz;
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<cylinder.sides+1;j++){
			glNormal3f(ctheta[j],stheta[j],0);
			glVertex3f(radius*ctheta[j],radius*stheta[j],z+dz);
			glVertex3f(radius*ctheta[j],radius*stheta[j],z);
		}
		glEnd();
	}
	delete [] ctheta;
	delete [] stheta;
  glPopMatrix();
  glDisable(GL_NORMALIZE);
}

/* draw a "cylinder" where the radius at the bottom and the top can differ */
/* rb: radius at the bottom of cylinder (z=0) */
/* rt: radius at the top of cylinder (z=h)*/
/* h: height of cylinder */
/* slices: number of subdivisions around the z axis (must be >0) */
/* stacks: number of subdivisions along the z axis (must be >0) */
/* mode: flag indicating if the bottom and/or top surface have to be drawn */
/*       set first bit (LSB) to 1 to draw the bottom face */
/*       set second bit to 1 to draw the top face */
void drawCylinder(float rb, float rt, float h, int slices, int stacks, int mode){
  float dz,dr,z1,z2,r1,r2;
  float sphi,cphi;
  float *ctheta,*stheta,dtheta;
  int i,j;

  dz=h/stacks;
  dr=(rt-rb)/stacks;
  sphi=1/sqrt(h*h+(rb-rt)*(rb-rt));
  cphi=(rb-rt)*sphi;
  sphi*=h;
  dtheta=2*PI/slices;
  ctheta=new float[slices+1];
  stheta=new float[slices+1];
  for(j=0;j<=slices;j++){
    ctheta[j]=cos(j*dtheta);
    stheta[j]=sin(j*dtheta);
  }
  z2=0;
  r2=rb;
  for(i=0;i<stacks;i++){ //draw cylinder sides
    z1=z2;
    z2=(i+1)*dz;
    r1=r2;
    r2=rb+(i+1)*dr;
    glBegin(GL_QUAD_STRIP);
    for(j=0;j<=slices;j++){
      glNormal3f(ctheta[j]*sphi,stheta[j]*sphi,cphi);
      glVertex3f(r2*ctheta[j],r2*stheta[j],z2);
      glVertex3f(r1*ctheta[j],r1*stheta[j],z1);
    }
    glEnd();
  }
  if(mode&1){ //draw bottom face
    glNormal3f(0,0,-1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for(j=slices;j>=0;j--)
      glVertex3f(rb*ctheta[j],rb*stheta[j],0);
    glEnd();
  }
  if(mode&2){ //draw top face
    glNormal3f(0,0,1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,h);
    for(j=0;j<=slices;j++)
      glVertex3f(rt*ctheta[j],rt*stheta[j],h);
    glEnd();
  }
  delete [] ctheta;
  delete [] stheta;
}

/*draw a sphere according to the structure MySphere*/
/*advice: use drawGeosphere instead, much faster to render*/
void drawSphere(MySphere &sphere){
	GLfloat diffuseColor[4];
	float x,y,z;
	float radius;
	float dtheta,dphi;
	float *ctheta,*stheta;
	float *cphi,*sphi;
	int   slices,stacks;
	int i,j,k;
	GLfloat matrix[16];
	
  glEnable(GL_NORMALIZE);
	for(k=0;k<4;k++)
		diffuseColor[k]=sphere.color[k]/255.;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=sphere.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	slices=sphere.segs;
	stacks=(int)(slices/2.+0.5);
	radius=sphere.radius;
	dphi=2*PI/slices;
	dtheta=PI/stacks;
	cphi=new float[slices+1];
	sphi=new float[slices+1];
	for(j=0;j<slices+1;j++){
		cphi[j]=cos(j*dphi);
		sphi[j]=sin(j*dphi);
	}
	ctheta=new float[stacks+1];
	stheta=new float[stacks+1];
	for(j=0;j<stacks+1;j++){
		ctheta[j]=cos(j*dtheta);
		stheta[j]=sin(j*dtheta);
	}
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,1);
	glVertex3f(0,0,radius);
	for(i=0;i<slices+1;i++){
		x=stheta[1]*cphi[i];
		y=stheta[1]*sphi[i];
		z=ctheta[1];
		glNormal3f(x,y,z);
		glVertex3f(radius*x,radius*y,radius*z);
	}
	glEnd();
	for(i=1;i<stacks-1;i++){
		glBegin(GL_QUAD_STRIP);
		for(j=0;j<slices+1;j++){
			x=stheta[i]*cphi[j];
			y=stheta[i]*sphi[j];
			z=ctheta[i];
			glNormal3f(x,y,z);
			glVertex3f(radius*x,radius*y,radius*z);
			x=stheta[i+1]*cphi[j];
			y=stheta[i+1]*sphi[j];
			z=ctheta[i+1];
			glNormal3f(x,y,z);
			glVertex3f(radius*x,radius*y,radius*z);
		}
		glEnd();
	}
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,-1);
	glVertex3f(0,0,-radius);
	for(i=slices;i>=0;i--){
		x=stheta[stacks-1]*cphi[i];
		y=stheta[stacks-1]*sphi[i];
		z=ctheta[stacks-1];
		glNormal3f(x,y,z);
		glVertex3f(radius*x,radius*y,radius*z);
	}
	glEnd();
	delete [] cphi;
	delete [] sphi;
	delete [] ctheta;
	delete [] stheta;
  glPopMatrix();
  glDisable(GL_NORMALIZE);
}

/*draw lines according to the structure MyLines*/
void drawLines(MyLines &lines){
	GLfloat diffuseColor[4];
	int i,k;
	GLfloat matrix[16];
	
	for(k=0;k<4;k++)
		diffuseColor[k]=lines.color[k]/255.;
	glColor4fv(diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=lines.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	glDisable(GL_LIGHTING);
	glBegin(lines.close?GL_LINE_LOOP:GL_LINE_STRIP);
	for(i=0;i<lines.nbpts;i++)
		glVertex3f(lines.points[i].x,lines.points[i].y,lines.points[i].z);
	glEnd();
  glPopMatrix();
	glEnable(GL_LIGHTING);
}

/*draw a mesh according to the structure MyMesh*/
void drawMesh(MyMesh &mesh){
	GLfloat diffuseColor[4];
  PointF3D pt0,pt1,pt2;
  double   nx,ny,nz,norm;
  double   x0,x1,x2,y0,y1,y2,z0,z1,z2;
  int      i,k;
	GLfloat  matrix[16];
	
  glEnable(GL_NORMALIZE);
	for(k=0;k<4;k++)
		diffuseColor[k]=mesh.color[k]/255.;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=mesh.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	glBegin(GL_TRIANGLES);
	for(i=0;i<mesh.faces.size();i++){
    pt0=mesh.vertices[mesh.faces[i].i];
    pt1=mesh.vertices[mesh.faces[i].j];
    pt2=mesh.vertices[mesh.faces[i].k];
    x0=pt0.x; y0=pt0.y; z0=pt0.z;
    x1=pt1.x; y1=pt1.y; z1=pt1.z;
    x2=pt2.x; y2=pt2.y; z2=pt2.z;
    nx=(y1-y0)*(z2-z0)-(z1-z0)*(y2-y0);
    ny=(z1-z0)*(x2-x0)-(x1-x0)*(z2-z0);
    nz=(x1-x0)*(y2-y0)-(y1-y0)*(x2-x0);
    norm=sqrt(nx*nx+ny*ny+nz*nz);
    glNormal3f(nx/norm,ny/norm,nz/norm);
		glVertex3f(pt0.x,pt0.y,pt0.z);
		glVertex3f(pt1.x,pt1.y,pt1.z);
		glVertex3f(pt2.x,pt2.y,pt2.z);
	}
	glEnd();
  glPopMatrix();
  glDisable(GL_NORMALIZE);
}

void getMinMaxBox(MyBox &box, float *dims){
  dims[0]=-box.width/2;
  dims[1]= box.width/2;
  dims[2]=-box.length/2;
  dims[3]= box.length/2;
  dims[4]= 0;
  dims[5]= box.height;
  transformBoxDims(dims,box.transform);
}

void getMinMaxCylinder(MyCylinder &cylinder, float *dims){
  dims[0]=-cylinder.radius;
  dims[1]= cylinder.radius;
  dims[2]=-cylinder.radius;
  dims[3]= cylinder.radius;
  dims[4]= 0;
  dims[5]= cylinder.height;
  transformBoxDims(dims,cylinder.transform);
}

void getMinMaxSphere(MySphere &sphere, float *dims){
  dims[0]=dims[2]=dims[4]=-sphere.radius;
  dims[1]=dims[3]=dims[5]= sphere.radius;
  transformBoxDims(dims,sphere.transform);
}
void getMinMaxLines(MyLines &lines, float *dims){
  int i;
  float tmpx,tmpy,tmpz;
  if(lines.nbpts>0){
    dims[0]=dims[1]=lines.points[0].x;
    dims[2]=dims[3]=lines.points[0].y;
    dims[4]=dims[5]=lines.points[0].z;
    for(i=1;i<lines.nbpts;i++){
      tmpx=lines.points[i].x;
      tmpy=lines.points[i].y;
      tmpz=lines.points[i].z;
      dims[0]=tmpx<dims[0]?tmpx:dims[0];
      dims[1]=tmpx>dims[1]?tmpx:dims[1];
      dims[2]=tmpy<dims[2]?tmpy:dims[2];
      dims[3]=tmpy>dims[3]?tmpy:dims[3];
      dims[4]=tmpz<dims[4]?tmpz:dims[4];
      dims[5]=tmpz>dims[5]?tmpz:dims[5];
    }
  }
  transformBoxDims(dims,lines.transform);
}

void getMinMaxMesh(MyMesh &mesh, float*dims){
  int i;
  float tmpx,tmpy,tmpz;
  if(mesh.vertices.size()>0){
    dims[0]=dims[1]=mesh.vertices[0].x;
    dims[2]=dims[3]=mesh.vertices[0].y;
    dims[4]=dims[5]=mesh.vertices[0].z;
    for(i=1;i<mesh.vertices.size();i++){
      tmpx=mesh.vertices[i].x;
      tmpy=mesh.vertices[i].y;
      tmpz=mesh.vertices[i].z;
      dims[0]=tmpx<dims[0]?tmpx:dims[0];
      dims[1]=tmpx>dims[1]?tmpx:dims[1];
      dims[2]=tmpy<dims[2]?tmpy:dims[2];
      dims[3]=tmpy>dims[3]?tmpy:dims[3];
      dims[4]=tmpz<dims[4]?tmpz:dims[4];
      dims[5]=tmpz>dims[5]?tmpz:dims[5];
    }
  }
  transformBoxDims(dims,mesh.transform);
}

void transformBoxDims(float *dims, float trans[16]){
  float pt1[4]={0,0,0,1},pt2[4]={0,0,0,1};
  float minx,maxx,miny,maxy,minz,maxz;
  int x[8]={0,0,0,0,1,1,1,1};
  int y[8]={2,2,3,3,2,2,3,3};
  int z[8]={4,5,4,5,4,5,4,5};
  int i,j,k;
  
  minx=maxx=0;
  miny=maxy=0;
  minz=maxz=0;
  for(k=0;k<8;k++){
    pt1[0]=dims[x[k]];
    pt1[1]=dims[y[k]];
    pt1[2]=dims[z[k]];
    for(i=0;i<3;i++){
      pt2[i]=0;
      for(j=0;j<4;j++)
        pt2[i]+=trans[i+4*j]*pt1[j];
    }
    if(k==0){
      minx=maxx=pt2[0];
      miny=maxy=pt2[1];
      minz=maxz=pt2[2];
    }
    else{
      minx=pt2[0]<minx?pt2[0]:minx;
      maxx=pt2[0]>maxx?pt2[0]:maxx;
      miny=pt2[1]<miny?pt2[1]:miny;
      maxy=pt2[1]>maxy?pt2[1]:maxy;
      minz=pt2[2]<minz?pt2[2]:minz;
      maxz=pt2[2]>maxz?pt2[2]:maxz;
    }
  }
  dims[0]=minx;
  dims[1]=maxx;
  dims[2]=miny;
  dims[3]=maxy;
  dims[4]=minz;
  dims[5]=maxz;
}


/*generate a Geosphere structure containing an icosahedron*/
Geosphere* getIcosahedron(float radius){
	
	Geosphere *geo;
	double alpha,beta;
	double y,z;
	int i;
	
	geo=(Geosphere*)calloc(1,sizeof(Geosphere));
	geo->nbVerts=12;
	geo->nbFaces=20;
	geo->vertices=(float(*)[3])calloc(geo->nbVerts,sizeof(float[3]));
	geo->faces=(int(*)[3])calloc(geo->nbFaces,sizeof(int[3]));
	
	alpha=cos(2.*PI/5.);
	beta=sin(2.*PI/5.);
	y=sqrt(1.-2*alpha)/(1-alpha);
	z=alpha/(1-alpha);
	geo->vertices[0][0]=0;
	geo->vertices[0][1]=0;
	geo->vertices[0][2]=radius;
	geo->vertices[1][0]=0;
	geo->vertices[1][1]=radius*y;
	geo->vertices[1][2]=radius*z;
	for(i=0;i<4;i++){
		geo->vertices[2+i][0]=alpha*geo->vertices[1+i][0]-beta*geo->vertices[1+i][1];
		geo->vertices[2+i][1]=beta*geo->vertices[1+i][0]+alpha*geo->vertices[1+i][1];
		geo->vertices[2+i][2]=geo->vertices[1][2];
	}
	for(i=0;i<6;i++){
		geo->vertices[6+i][0]=-geo->vertices[5-i][0];
		geo->vertices[6+i][1]=-geo->vertices[5-i][1];
		geo->vertices[6+i][2]=-geo->vertices[5-i][2];
	}
	geo->faces[0][0]=0;
	geo->faces[0][1]=1;
	geo->faces[0][2]=2;
	geo->faces[1][0]=0;
	geo->faces[1][1]=2;
	geo->faces[1][2]=3;
	geo->faces[2][0]=0;
	geo->faces[2][1]=3;
	geo->faces[2][2]=4;
	geo->faces[3][0]=0;
	geo->faces[3][1]=4;
	geo->faces[3][2]=5;
	geo->faces[4][0]=0;
	geo->faces[4][1]=5;
	geo->faces[4][2]=1;
	geo->faces[5][0]=1;
	geo->faces[5][1]=7;
	geo->faces[5][2]=2;
	geo->faces[6][0]=2;
	geo->faces[6][1]=6;
	geo->faces[6][2]=3;
	geo->faces[7][0]=3;
	geo->faces[7][1]=10;
	geo->faces[7][2]=4;
	geo->faces[8][0]=4;
	geo->faces[8][1]=9;
	geo->faces[8][2]=5;
	geo->faces[9][0]=5;
	geo->faces[9][1]=8;
	geo->faces[9][2]=1;
	geo->faces[10][0]=2;
	geo->faces[10][1]=7;
	geo->faces[10][2]=6;
	geo->faces[11][0]=3;
	geo->faces[11][1]=6;
	geo->faces[11][2]=10;
	geo->faces[12][0]=4;
	geo->faces[12][1]=10;
	geo->faces[12][2]=9;
	geo->faces[13][0]=5;
	geo->faces[13][1]=9;
	geo->faces[13][2]=8;
	geo->faces[14][0]=1;
	geo->faces[14][1]=8;
	geo->faces[14][2]=7;
	geo->faces[15][0]=11;
	geo->faces[15][1]=6;
	geo->faces[15][2]=7;
	geo->faces[16][0]=11;
	geo->faces[16][1]=7;
	geo->faces[16][2]=8;
	geo->faces[17][0]=11;
	geo->faces[17][1]=8;
	geo->faces[17][2]=9;
	geo->faces[18][0]=11;
	geo->faces[18][1]=9;
	geo->faces[18][2]=10;
	geo->faces[19][0]=11;
	geo->faces[19][1]=10;
	geo->faces[19][2]=6;
	
	return geo;
}

/*relax memory allocated for a Geosphere structure.*/
/*the structure must have been created dynamically.*/
void relaxGeosphere(Geosphere *geo){
	if(geo!=NULL){
		free(geo->vertices);
		free(geo->faces);
		free(geo);
	}
}

/*creates a geosphere based on geoIni. Each segment*/
/*of geoIni will be divided in nbSegs.*/
Geosphere* getGeosphere(Geosphere *geoIni, int nbSegs){
	
	Geosphere *geoNew=NULL;
	int i,j,k,index;
	int nbNewVertsSeg,iNewVertSeg;
	int iNewVertIn;
	int iFaces;
	int nbVertsPerFace;
	int iv1,iv2,iv3;
	int n1,n2,n3,w;
	int (*segs)[3]; // array for each new vertex on triangle side
	int *faceVerts;
	double norm,radius;
	
	if(nbSegs<=0)
		return NULL;
	
	geoNew=(Geosphere*)calloc(1,sizeof(Geosphere));
	geoNew->nbVerts=geoIni->nbVerts+(geoIni->nbFaces*(nbSegs*nbSegs-1))/2;
	nbNewVertsSeg=(geoIni->nbFaces*3*(nbSegs-1))/2; // number of new vertices on triangle side
	nbVertsPerFace=((nbSegs+1)*(nbSegs+2))/2; // number of vertices per face
	geoNew->nbFaces=geoIni->nbFaces*nbSegs*nbSegs;
	geoNew->vertices=(float(*)[3])calloc(geoNew->nbVerts,sizeof(float[3]));
	geoNew->faces=(int(*)[3])calloc(geoNew->nbFaces,sizeof(int[3]));
	segs=(int(*)[3])calloc(nbNewVertsSeg,sizeof(int[3]));
	faceVerts=(int*)calloc(nbVertsPerFace,sizeof(int));
	
	iNewVertSeg=0;
	iNewVertIn=0;
	iFaces=0;
	for(i=0;i<geoIni->nbVerts;i++)
		memcpy(geoNew->vertices[i],geoIni->vertices[i],3*sizeof(float));
	for(i=0;i<geoIni->nbFaces;i++){
		index=0;
		for(n1=nbSegs;n1>=0;n1--){
			for(n2=nbSegs-n1;n2>=0;n2--){
				n3=nbSegs-n1-n2;
				iv1=geoIni->faces[i][0];
				iv2=geoIni->faces[i][1];
				iv3=geoIni->faces[i][2];
				if(n1==nbSegs)
					faceVerts[index]=iv1;
				else if(n2==nbSegs)
					faceVerts[index]=iv2;
				else if(n3==nbSegs)
					faceVerts[index]=iv3;
				else if(n1!=0 && n2!=0 && n3!=0){// vertex inside triangle
					faceVerts[index]=geoIni->nbVerts+nbNewVertsSeg+iNewVertIn;
					for(j=0;j<3;j++){
						geoNew->vertices[faceVerts[index]][j] =(geoIni->vertices[iv1][j])*n1;
						geoNew->vertices[faceVerts[index]][j]+=(geoIni->vertices[iv2][j])*n2;
						geoNew->vertices[faceVerts[index]][j]+=(geoIni->vertices[iv3][j])*n3;
						geoNew->vertices[faceVerts[index]][j]/=nbSegs;
					}
					iNewVertIn++;
				}
				else{// vertex on triangle side. can already exist.
					if(n1==0){ //set iv1 and iv2 as the 2 vertices of segment, with iv1<iv2
					  if(iv2<iv3) {iv1=iv2; iv2=iv3; w=n2;} //w=weight of 1st vertex.
					  else        {iv1=iv3; w=n3;}
					}
					else if(n2==0){
					  if(iv3<iv1) {iv2=iv1; iv1=iv3; w=n3;}
					  else        {iv2=iv3; w=n1;}
					}
					else{ //n3==0
					  if(iv1>iv2) {iv3=iv2; iv2=iv1; iv1=iv3; w=n2;}
					  else		    w=n1;
					}
					for(j=0;j<iNewVertSeg;j++)
						if(iv1==segs[j][0] && iv2==segs[j][1] && w==segs[j][2])
							break;
					faceVerts[index]=geoIni->nbVerts+j;
					if(j==iNewVertSeg){//new vertex
						segs[j][0]=iv1;
						segs[j][1]=iv2;
						segs[j][2]=w;
						for(k=0;k<3;k++){
							geoNew->vertices[faceVerts[index]][k] =(geoIni->vertices[iv1][k])*w;
							geoNew->vertices[faceVerts[index]][k]+=(geoIni->vertices[iv2][k])*(nbSegs-w);
							geoNew->vertices[faceVerts[index]][k]/=nbSegs;
						}
						iNewVertSeg++;
					}
				}
				index++;
			}
		}
		for(index=0,j=0;j<nbSegs;j++){//generate faces
			for(k=0;k<j+1;k++){
				geoNew->faces[iFaces][0]=faceVerts[index];
				geoNew->faces[iFaces][1]=faceVerts[index+j+1];
				geoNew->faces[iFaces][2]=faceVerts[index+j+2];
				iFaces++;
				if(k<j){
					geoNew->faces[iFaces][0]=faceVerts[index];
					geoNew->faces[iFaces][1]=faceVerts[index+j+2];
					geoNew->faces[iFaces][2]=faceVerts[index+1];
					iFaces++;
				}
				index++;
			}
		}
	}
	for(radius=0,i=0;i<3;i++)
		radius+=(double)(geoIni->vertices[0][i])*geoIni->vertices[0][i];
	radius=sqrt(radius);
	for(i=geoIni->nbVerts;i<geoNew->nbVerts;i++){
		for(norm=0,j=0;j<3;j++)
			norm+=(double)(geoNew->vertices[i][j])*geoNew->vertices[i][j];
		norm=sqrt(norm)/radius;
		for(j=0;j<3;j++) geoNew->vertices[i][j]/=norm;
	}
	
	free(segs);
	free(faceVerts);
	relaxGeosphere(geoIni);
	return geoNew;
}

/*draw a geosphere centered at (0,0,0)*/
/*start from an icosahedron, nbSegs>=1*/
/*geosphere=icosahedron when nbSegs=1*/
void drawGeosphere(float radius, int nbSegs){
	Geosphere *geo;
	int       i,j,index;
	float     x,y,z;
	
	geo=getIcosahedron(1);
	geo=getGeosphere(geo,nbSegs);
	for(i=0;i<geo->nbFaces;i++){
		glBegin(GL_TRIANGLES);
		for(j=0;j<3;j++){
			index=geo->faces[i][j];
			x=geo->vertices[index][0];
			y=geo->vertices[index][1];
			z=geo->vertices[index][2];
			glNormal3f(x,y,z);
			glVertex3f(radius*x,radius*y,radius*z);
		}
		glEnd();
	}
	relaxGeosphere(geo);
}

/*draw a geosphere according to the structure MySphere*/
void drawGeosphere(MySphere &sphere){
	GLfloat diffuseColor[4];
	GLfloat matrix[16];
	int     k;
	
  glEnable(GL_NORMALIZE);
	for(k=0;k<4;k++)
		diffuseColor[k]=sphere.color[k]/255.;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseColor);
	for(k=0;k<16;k++)
		matrix[k]=sphere.transform[k];
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glMultMatrixf(matrix);
	drawGeosphere(sphere.radius,sphere.segs);
  glPopMatrix();
  glDisable(GL_NORMALIZE);
}
