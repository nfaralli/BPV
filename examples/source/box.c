#include "box.h"
#include "particles.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

/*
 * structure defining a particle
 */
typedef struct _Particle{
  float pos[3];   // position [m]
  float vel[3];   // velocity [m/s]
  float mass;     // mass [kg]
  float radius;   // radius [m]
  float cor;      // coefficient of restitution. Used when particle collides with something
  float f[3];     // force applied to particle [N]
  struct _Cell  *cell;  // cell in which the particle lies
  struct _Particle* nextPart;  // used for linked list in Cell structure.
} Particle;

/*
 * Structure defining a wall=border or obstacle.
 * A wall is represented by a triangle, and one
 * side is transparent (particles can go through)
 * while the other side is opaque (particles
 * bounce on it). Normal is a vector defining
 * which side is transparent and which one is
 * opaque (same idea as in openGL for example).
 */
typedef struct _Wall{
  float v1[3];      //1st vertex
  float v2[3];      //2nd vertex
  float v3[3];      //3rd vertex
  float normal[3];  //unit vector=v12xv13/norm

  //vector definition for collision detection
  float v12[3];     //v2-v1
  float v13[3];     //v3-v1
  float norm;       //norm of v12xv13
} Wall;

typedef struct _Cell{
  float pos[3]; // coordinates of the center of the cell.
  float dim[3]; // size along x, y, and z
  struct _Cell* nghb[14]; // neighboring cells used to detect p-p collisions.
  struct _Particle* part0; // linked list of particles in this cell.
} Cell;

typedef struct _Grid{
  Cell  *cells;
  int   ncells; // = dim[0]*dim[1]*dim[2]
  int   dim[3];
  float boundaries[3][2];
} Grid;

/*
 * structure defining the system containing
 * particles, walls and grid.
 */
typedef struct _System{
  float    time;              //time [s]
  float    prevDt;            //previous dt used
  Particle *parts;            //array of particles
  int      nbParts;           //number of particles
  Wall     *walls;            //array of 'walls'
  int      nbWalls;           //number of walls
  float    boundaries[3][2];  //system boundaries ([3] for x/y/z, [2] for min/max)
  Grid     *grid;
  unsigned char *prevStepCollisionTable; //used to check previous collision between particles
} System;

static float myrand();
static System* createSystem(int nbParts, float minMax[3][2], float cellSize);
static int deleteSystem(System *sys);
static int setGrid(System *sys, float cellSize);
static int setParticles(System *sys, int first, int last, float radius, float mass);
static Wall createWall(float *v1, float *v2, float *v3);
static int addWall(System *sys, Wall wall);
static int updateSys(System *sys, float dt);
static int freeFlight(Particle *part, float dt, Wall *walls, int nbWalls);
static int setParticlesGrid(Grid *grid, Particle *part0, int nbParts);
static int checkPPCollision(System *sys, Cell *cell0, int nCells);
static int setForces(Particle *parts, int nbParts, float time);
static void addWalls(System *sys);
static void createBoxGeometry(System *sys, char *filename);

/*
 * returns a float between 0 and 1 (inclusive)
 */
static float myrand(){
  return (float)(((double)rand())/RAND_MAX);
}

/*
 * Create a structure System containing nbParts particles.
 */
static System* createSystem(int nbParts, float minMax[3][2], float cellSize){
  System *sys;
  int i,j;
  float v1[3],v2[3],v3[3];

  if(nbParts<=0) return NULL;
  for(i=0;i<3;i++)
    if(minMax[i][0]>=minMax[i][1])
      return NULL;
  if((sys=(System*)calloc(1,sizeof(System)))==NULL)
    return NULL;
  sys->time=0;
  sys->nbWalls=12;
  sys->walls=(Wall*)calloc(sys->nbWalls,sizeof(Wall));
  for(i=0;i<3;i++){
    for(j=0;j<2;j++)
      sys->boundaries[i][j]=minMax[i][j];
    for(j=0;j<4;j++){
      v1[i]=      minMax[i][(j==0||j==3)?0:1];
      v1[(i+1)%3]=minMax[(i+1)%3][(j==0||j==3)?0:1];
      v1[(i+2)%3]=minMax[(i+2)%3][1-(j>>1)];
      v2[i]=      minMax[i][j&1];
      v2[(i+1)%3]=minMax[(i+1)%3][1-(j&1)];
      v2[(i+2)%3]=minMax[(i+2)%3][1-(j>>1)];
      v3[i]=      minMax[i][(j==0||j==3)?1:0];
      v3[(i+1)%3]=minMax[(i+1)%3][(j==0||j==3)?1:0];
      v3[(i+2)%3]=minMax[(i+2)%3][1-(j>>1)];
      sys->walls[4*i+j]=createWall(v1,v2,v3);
    }
  }
  sys->nbParts=nbParts;
  if((sys->parts=(Particle*)calloc(nbParts,sizeof(Particle)))==NULL)
      return NULL;
  sys->prevStepCollisionTable=(unsigned char*)calloc(nbParts*(nbParts-1)/2,sizeof(unsigned char));
  setGrid(sys, cellSize);
  return sys;
}

/*
 * Release the memory dynamically allocated by sys
 * (sys will also be deleted)
 */
static int deleteSystem(System *sys){
  free(sys->parts);
  free(sys->grid->cells);
  free(sys->grid);
  free(sys);
  return 0;
}

/*
 * sets the grid used to simplify the computation
 * of particle-particle collisions.
 * cellSize should be at least twice the radius of the
 * biggest particle.
 */
static int setGrid(System *sys, float cellSize){
  Grid *grid;
  Cell *cell, **nghb;
  int alpha,ixyz[3],index;
  int ix,iy,iz,ixyzNghb[3];
  int gridDim[3];

  grid=(Grid*)calloc(1,sizeof(Grid));
  grid->ncells=1;
  for(alpha=0; alpha<3; alpha++){
    gridDim[alpha]=(int)((sys->boundaries[alpha][1]-sys->boundaries[alpha][0])/cellSize)+1;
    grid->dim[alpha]=gridDim[alpha];
    grid->ncells*=grid->dim[alpha];
    grid->boundaries[alpha][0]=sys->boundaries[alpha][0]-(grid->dim[alpha]*cellSize-(sys->boundaries[alpha][1]-sys->boundaries[alpha][0]))/2;
    grid->boundaries[alpha][1]=grid->boundaries[alpha][0]+grid->dim[alpha]*cellSize;
  }
  grid->cells=(Cell*)calloc(grid->ncells,sizeof(Cell));
  for(index=0, cell=grid->cells; index<grid->ncells; index++, cell++){
    ixyz[0]=index%gridDim[0];
    ixyz[1]=(index/gridDim[0])%gridDim[1];
    ixyz[2]=index/(gridDim[0]*gridDim[1]);
    for(alpha=0; alpha<3; alpha++){
      cell->pos[alpha]=cellSize*(ixyz[alpha]+0.5)+grid->boundaries[alpha][0];
      cell->dim[alpha]=cellSize;
    }
    // set the 14 neighbors that will be used to check collisions. (actually only 13 neighbors plus cell itself)
    nghb=cell->nghb;
    for(iz=0; iz<=1; iz++){
      ixyzNghb[2]=ixyz[2]+iz;
      for(iy=(iz==1?-1:0); iy<=1; iy++){
        ixyzNghb[1]=ixyz[1]+iy;
        for(ix=(iz==1?-1:(iy==1?-1:0)); ix<=1; ix++, nghb++){
          ixyzNghb[0]=ixyz[0]+ix;
          for(alpha=0; alpha<3; alpha++)
            if(ixyzNghb[alpha]<0 || ixyzNghb[alpha]>=gridDim[alpha])
              break;
          if(alpha<3)
            *nghb=NULL;
          else
            *nghb=grid->cells+(ixyzNghb[0]+ixyzNghb[1]*gridDim[0]+ixyzNghb[2]*gridDim[0]*gridDim[1]);
        }
      }
    }
    cell->part0=NULL;
  }
  sys->grid=grid;
  printf("setGrid: done\n"); fflush(stdout);
  return 0;
}

/*
 * set the particles in sys.
 * first and last are the indices of the particles which will be set.
 * particles are initialized with zero velocity, and randomly
 * distributed within the system.
 * mass is the particle's mass.
 * returns 0 if particles set correctly
 */
static int setParticles(System *sys, int first, int last, float radius, float mass){
  Particle *part,*part0,*part1;
  int      alpha;

  if(first<0 || first>last ||last>=sys->nbParts)
    return 1;
  part0=sys->parts+first;
  part1=sys->parts+last+1;
  for(part=part0;part<part1;part++){
    part->mass=mass;
    part->radius=radius;
    part->cor=0.8;
    for(alpha=0;alpha<3;alpha++){
      part->vel[alpha]=0;
      part->pos[alpha]=myrand()*(sys->boundaries[alpha][1]-sys->boundaries[alpha][0]-2.01*part->radius)+sys->boundaries[alpha][0]+1.005*part->radius;
    }
    part->f[0]=0;
    part->f[1]=0;
    part->f[2]=0;
  }
  setParticlesGrid(sys->grid, sys->parts, sys->nbParts);
  return 0;
}

/*
 * from the 3 vertices v1, v2, and v3, generate a structure Wall
 */
static Wall createWall(float *v1, float *v2, float *v3){
  Wall  wall;
  float norm,small;
  int   i;

  for(small=i=0;i<3;i++){
    wall.v1[i]=v1[i];
    wall.v2[i]=v2[i];
    wall.v3[i]=v3[i];
    wall.v12[i]=v2[i]-v1[i];
    wall.v13[i]=v3[i]-v1[i];
    small=small<fabs(wall.v12[i])?fabs(wall.v12[i]):small;
    small=small<fabs(wall.v13[i])?fabs(wall.v13[i]):small;
  }
  small*=1e-5;
  for(norm=i=0;i<3;i++){
    wall.normal[i]=wall.v12[(i+1)%3]*wall.v13[(i+2)%3]-wall.v12[(i+2)%3]*wall.v13[(i+1)%3];
    norm+=wall.normal[i]*wall.normal[i];
  }
  norm=sqrt(norm);
  wall.norm=norm;
  if(norm<small){
    for(i=0;i<3;i++)
      wall.normal[i]=0;
  }
  else{
    for(i=0;i<3;i++){
      wall.normal[i]/=norm;
    }
  }
  return wall;
}

/*
 * Add wall to sys.
 */
static int addWall(System *sys, Wall wall){
  sys->walls=(Wall*)realloc(sys->walls,(sys->nbWalls+1)*sizeof(Wall));
  sys->walls[sys->nbWalls++]=wall;
  return 0;
}

/*
 * Perform the following:
 * - Free flight (velocity is constant during the free flight)
 * - Particle tracking: find in which cell each particle ended
 *   up after the free flight.
 * - Check for particle-particle collision and update particle
 *   velocities if a collisions occur.
 * - Set the force based on the new position and time.
 * - Update the velocity based on force.
 */
static int updateSys(System *sys, float time){
  int      alpha;     //x,y,z index
  Particle *part;
  static clock_t timeFreeFlight=0;
  static clock_t timeTracking=0;
  static clock_t timePPCollision=0;
  float dt;
  clock_t time1, time2;

  // specific case used to dump some timers.
  if(sys==NULL){
      printf("free flight: %f sec; tracking: %f sec; collision: %f sec\n",
          (float)timeFreeFlight/CLOCKS_PER_SEC, (float)timeTracking/CLOCKS_PER_SEC, (float)timePPCollision/CLOCKS_PER_SEC);
    return 0;
  }

  dt=time-sys->time;
  time1=clock();
  for(part=sys->parts; part<sys->parts+sys->nbParts; part++){
    freeFlight(part,dt,sys->walls,sys->nbWalls);
  }
  time2=clock();
  timeFreeFlight+=time2-time1;
  time1=time2;

  setParticlesGrid(sys->grid, sys->parts, sys->nbParts);
  time2=clock();
  timeTracking+=time2-time1;
  time1=time2;

  checkPPCollision(sys, sys->grid->cells, sys->grid->ncells);
  time2=clock();
  timePPCollision+=time2-time1;
  time1=time2;

  setForces(sys->parts, sys->nbParts, time);
  for(part=sys->parts; part<sys->parts+sys->nbParts; part++){
    for(alpha=0;alpha<3;alpha++){
      part->vel[alpha]+=dt*part->f[alpha]/part->mass;
    }
  }

  sys->time=time;
  sys->prevDt=dt;
  return 0;
}

/*
 * Check if a particle hits a wall during the "free flight".
 * If a collision occurs, update the position and the velocity.
 * Collision Detection is done by computing the intersection
 * of the plane defined by the wall, and the line defined by part->pos
 * and part->vel.
 */
static int freeFlight(Particle *part, float dt, Wall *walls, int nbWalls){
  Wall  *wall,*wall1,*wallC;
  float r[3],rr[3],dr[3];
  float cp[3];
  float omega, t=0, beta, gamma, tmin, coef;
  int alpha;

  wall1=walls+nbWalls;
  for(alpha=0; alpha<3; alpha++)
    r[alpha]=part->pos[alpha];
  do{
    wallC=NULL;
    tmin=1;
    for(alpha=0; alpha<3; alpha++)
      dr[alpha]=part->vel[alpha]*dt;
    for(wall=walls;wall<wall1;wall++){
      omega=wall->normal[0]*dr[0]+wall->normal[1]*dr[1]+wall->normal[2]*dr[2];
      if(omega>=0) //particle not going in the good direction
        continue;
      for(alpha=0; alpha<3; alpha++)
        rr[alpha]=r[alpha]-part->radius*wall->normal[alpha]; //rr is the point on the spherical particle that may touch the wall
      t=(wall->normal[0]*(wall->v1[0]-rr[0])+wall->normal[1]*(wall->v1[1]-rr[1])+wall->normal[2]*(wall->v1[2]-rr[2]))/omega;
      if(t>1 || t>=tmin) //t>1 means that the collision will happen in the future
        continue;
      if(t<0){ //t<0 means that the collision happened in the past
        if(t<0.2*part->radius/omega) //due to numerical precision, sometime particles crosses wall. solution: go back in time!
          continue; // here if the particle is less than 10% inside the wall (20% of radius), we go back in time
      }
      // rr+t*dr is on the plane containing wall. have to check now if it is on wall
      omega*=wall->norm;
      cp[0] = (rr[1]-wall->v1[1])*dr[2] - (rr[2]-wall->v1[2])*dr[1];
      cp[1] = (rr[2]-wall->v1[2])*dr[0] - (rr[0]-wall->v1[0])*dr[2];
      cp[2] = (rr[0]-wall->v1[0])*dr[1] - (rr[1]-wall->v1[1])*dr[0];
      beta=-(cp[0]*wall->v13[0]+cp[1]*wall->v13[1]+cp[2]*wall->v13[2])/omega;
      if(beta<-0.001 || beta>1.001)
        continue;
      gamma=(cp[0]*wall->v12[0]+cp[1]*wall->v12[1]+cp[2]*wall->v12[2])/omega;
      if(gamma<-0.001 || gamma>1.001 || (beta+gamma)<-0.001 || (beta+gamma)>1.001)
        continue;
      //collision!
      wallC=wall;
      tmin=t;
    }
    if(wallC!=NULL){//collision detected. update position and velocity
      coef=-(1+part->cor)*(part->vel[0]*wallC->normal[0]+part->vel[1]*wallC->normal[1]+part->vel[2]*wallC->normal[2]);
      for(alpha=0; alpha<3; alpha++){
        r[alpha]+=tmin*dr[alpha]; //update position
        part->vel[alpha]+=coef*wallC->normal[alpha];//reflect velocity
      }
      dt*=1-tmin; //update remaining time
    }
  }while(wallC!=NULL);
  for(alpha=0;alpha<3;alpha++)
    part->pos[alpha]=r[alpha]+dr[alpha];
  return 0;
}

/*
 * Sets the particles in the grid, setting up the linked list in each Cell structure.
 * This function is supposed to be called after updating the particle's position.
 */
static int setParticlesGrid(Grid *grid, Particle *part0, int nbParts){
  float (*boundaries)[2];
  float cellDim[3];
  int   gridDim[3];
  int alpha;
  float *pos;
  int   ixyz[3];
  Particle *part, *part1, **cellPart;
  Cell *initCell, *newCell;

  boundaries=grid->boundaries;
  for(alpha=0; alpha<3; alpha++){
    cellDim[alpha]=grid->cells->dim[alpha];
    gridDim[alpha]=grid->dim[alpha];
  }
  part1=part0+nbParts;
  for(part=part0; part<part1; part++){
    //get index in grid
    pos=part->pos;
    for(alpha=0; alpha<3; alpha++){
      if(pos[alpha]<boundaries[alpha][0] || pos[alpha]>boundaries[alpha][1])
        break;
      ixyz[alpha]=(int)((pos[alpha]-boundaries[alpha][0])/cellDim[alpha]);
      if(ixyz[alpha]<0)               ixyz[alpha]=0;
      if(ixyz[alpha]>=gridDim[alpha]) ixyz[alpha]=gridDim[alpha]-1;
    }
    //check if the particle moved outside of its initial cell
    newCell = alpha<3 ? NULL : grid->cells + (ixyz[0] + (ixyz[1] + ixyz[2]*gridDim[1])*gridDim[0]);
    if(part->cell != newCell){
      //update both initial and final cell
      initCell=part->cell;
      if(initCell){
        cellPart=&(initCell->part0);
        while(*cellPart){
          if(*cellPart==part){
            *cellPart=part->nextPart;
            break;
          }
          cellPart=&((*cellPart)->nextPart);
        }
      }
      if(newCell){
        part->nextPart=newCell->part0;
        newCell->part0=part;
      }
      part->cell=newCell;
    }
  }
  return 0;
}

/*
 * check Particle-Particle collision, and update velocities if needed
 */
int checkPPCollision(System *sys, Cell *cell0, int nCells){
  Particle *part1, *part2;
  float n[3],norm,dotp;
  float coef1,coef2;
  int alpha, inghb;
  int i, i1, i2;
  unsigned char *prevStepCollision;
  Cell *cell, *cell1, *nghbCell;
  float k=500000; //stiffness coef

  cell1=cell0+nCells;
  for(cell=cell0; cell<cell1; cell++){
    part1=cell->part0;
    while(part1){
      for(inghb=0; inghb<14; inghb++){
        nghbCell=cell->nghb[inghb];
        if(!nghbCell)
          continue;
        if(nghbCell==cell)
          part2=part1->nextPart;
        else
          part2=nghbCell->part0;
        while(part2){
          for(alpha=norm=0; alpha<3; alpha++){
            n[alpha]=part1->pos[alpha]-part2->pos[alpha];
            norm+=n[alpha]*n[alpha];
          }
          norm=sqrt(norm);
          i1=part1-sys->parts;
          i2=part2-sys->parts;
          if(i1<i2){
            i=i1;
            i1=i2;
            i2=i;
          }
          i=i1*(2*sys->nbParts-3-i1)/2+i2-1;
          prevStepCollision=sys->prevStepCollisionTable+i;
          if(norm<(part1->radius+part2->radius) && norm>0){ //collision detected. norm=0 should never happen...
            if(*prevStepCollision==0){
              for(alpha=dotp=0; alpha<3; alpha++){
                n[alpha]/=norm;
                dotp+=(part2->vel[alpha]-part1->vel[alpha])*n[alpha];
              }
              coef1=dotp*(1+part1->cor)*(part2->mass/(part1->mass+part2->mass));
              coef2=-dotp*(1+part2->cor)*(part1->mass/(part1->mass+part2->mass));
              for(alpha=0; alpha<3; alpha++){
                part1->vel[alpha]+=coef1*n[alpha];
                part2->vel[alpha]+=coef2*n[alpha];
              }
              *prevStepCollision=1;
            }
            else{ //implement some spring force to repulse the two particles, otherwise they form some clusters
              coef1=k*(part1->radius-part1->radius*norm/(part1->radius+part2->radius))*sys->prevDt/part1->mass;
              coef2=-k*(part2->radius-part2->radius*norm/(part1->radius+part2->radius))*sys->prevDt/part1->mass;
              for(alpha=0; alpha<3; alpha++){
                part1->vel[alpha]+=coef1*n[alpha];
                part2->vel[alpha]+=coef2*n[alpha];
              }
            }
          }
          else //no collision. clear flag
            *prevStepCollision=0;
          part2=part2->nextPart;
        }
      }
      part1=part1->nextPart;
    }
  }
  return 0;
}

/*
 * Set the force applied to each particles.
 * The forces vary over time for more interesting trajectories.
 */
static int setForces(Particle *parts, int nbParts, float time){
  Particle *part;
  const float g=9.80665;  //g in m/s^2
  int alpha;
  float norm;
  for(part=parts; part<parts+nbParts; part++){
    for(alpha=0;alpha<3;alpha++){
      if(time<2){  // Gravity along -z
        part->f[0]=0;
        part->f[1]=0;
        part->f[2]=-part->mass*g;
      } else if(time<4){  // Gravity along +x
        part->f[0]=part->mass*g;
        part->f[1]=0;
        part->f[2]=0;
      } else if(time<6){  // Gravity along +z
        part->f[0]=0;
        part->f[1]=0;
        part->f[2]=part->mass*g;
      } else if(time<8){  // Gravity along -z
        part->f[0]=0;
        part->f[1]=0;
        part->f[2]=-part->mass*g;
      } else if(time<11){  // Rotational force field
        part->f[0]=part->f[1]=part->f[2]=0;
        norm=sqrt(part->pos[0]*part->pos[0]+part->pos[2]*part->pos[2]);
        if(norm>0.01){
          part->f[0]=(part->pos[2]/norm)*part->mass*g;
          part->f[2]=-(part->pos[0]/norm)*part->mass*g;
        }
      } else if(time<16){  // Central force
        part->f[0]=part->f[1]=part->f[2]=0;
        norm=sqrt(part->pos[0]*part->pos[0]+part->pos[2]*part->pos[2]);
        if(norm>0.01){
          for(alpha=0; alpha<3; alpha++)
            part->f[alpha]=-(part->pos[alpha]/norm)*part->mass*g;
        }
      } else {  // Central force
          part->f[0]=part->f[1]=part->f[2]=0;
          norm=sqrt(part->pos[0]*part->pos[0]+part->pos[2]*part->pos[2]);
          if(norm>0.01){
            for(alpha=0; alpha<3; alpha++)
              part->f[alpha]=(part->pos[alpha]/norm)*part->mass*g;
          }
      }
    }
  }
  return 0;
}

/*
 * Add obstacles in the system.
 */
static void addWalls(System *sys){
  float v1[3],v2[3],v3[3],v4[3];
  v1[0]=-0.5;  v1[1]=1;   v1[2]=-0.5;
  v2[0]=-0.5;  v2[1]=-1;  v2[2]=-0.5;
  v3[0]=1;     v3[1]=-1;  v3[2]=0.5;
  v4[0]=1;     v4[1]=1;   v4[2]=0.5;
  addWall(sys,createWall(v1,v2,v3));
  addWall(sys,createWall(v1,v3,v4));

  v1[0]=-0.5;  v1[1]=1;   v1[2]=-0.51;
  v2[0]=-0.5;  v2[1]=-1;  v2[2]=-0.51;
  v3[0]=1;     v3[1]=-1;  v3[2]=0.49;
  v4[0]=1;     v4[1]=1;   v4[2]=0.49;
  addWall(sys,createWall(v1,v3,v2));
  addWall(sys,createWall(v1,v4,v3));
}

/*
 * Generate the header of the .par file.
 * The particles are split in two groups.
 * An additional variable is use (velocity) and can be used
 * e.g. to set the color of the particles in BPV.
 */
static Header* createBoxHeader(float radius, unsigned int nbParts, unsigned int nbSteps){
    Header *header;
    unsigned int i;

    header=createHeader(2, 1, nbParts, 0, 0, nbSteps);
    strcpy(header->types[0].name, "balls 1");
    header->types[0].radius = radius;
    header->types[0].rgb[0] = 0;
    header->types[0].rgb[1] = 0;
    header->types[0].rgb[2] = 255;
    strcpy(header->types[1].name, "balls 2");
    header->types[1].radius = radius;
    header->types[1].rgb[0] = 255;
    header->types[1].rgb[1] = 0;
    header->types[1].rgb[2] = 0;

    strcpy(header->xyzNames[0], "x [m]");
    strcpy(header->xyzNames[1], "y [m]");
    strcpy(header->xyzNames[2], "z [m]");
    strcpy(header->variables[0], "velocity [m/s]");

    for(i=0; i<nbParts; i++){
        header->particlesType[i]=i&1;
    }
    return header;
}

/*
 * Update the Step structure with the particles position, time, etc.
 */
static void updateStep(Step *step, float time, Particle *parts, unsigned int nbParts){
    unsigned int i;
    float velocity;
    step->time=time;
    for(i=0; i<nbParts; i++){
        step->x[i] = parts[i].pos[0];
        step->y[i] = parts[i].pos[1];
        step->z[i] = parts[i].pos[2];
        velocity = sqrt(parts[i].vel[0]*parts[i].vel[0] +
                        parts[i].vel[1]*parts[i].vel[1] +
                        parts[i].vel[2]*parts[i].vel[2]);
        step->variables[0][i] = velocity;
    }
}

/*
 * Write a mesh primitive in file (must be a .geo file).
 */
void writeMeshGeometry(FILE *file, Wall *wall0, unsigned int nbWalls, unsigned char rgba[4], char *name, int showBackface){
    Wall *wall;
    unsigned int i;
    fprintf(file, "mesh name:\"%s\" wirecolor:(color %d %d %d %d) vertices:#(", name, rgba[0], rgba[1], rgba[2], rgba[3]);
    for(i=0; i<nbWalls; i++){
        wall=wall0+i;
        fprintf(file, "[%f, %f, %f],", wall->v1[0], wall->v1[1], wall->v1[2]);
        fprintf(file, "[%f, %f, %f],", wall->v2[0], wall->v2[1], wall->v2[2]);
        fprintf(file, "[%f, %f, %f]" , wall->v3[0], wall->v3[1], wall->v3[2]);
        if(i<nbWalls-1){
            fprintf(file, ",");
        }
    }
    fprintf(file, ") faces:#(");
    for(i=0; i<nbWalls; i++){
        fprintf(file, "[%d, %d, %d]", 3*i+1, 3*i+2, 3*i+3);
        if(showBackface){
            fprintf(file, ",[%d, %d, %d]", 3*i+1, 3*i+3, 3*i+2);
        }
        if(i<nbWalls-1){
            fprintf(file, ",");
        }
    }
    fprintf(file, ")\n");
}

/*
 * Utility function generating a .geo file based on the
 * system geometry.
 * At least one mesh ('Box') is created with the boundary
 * of the system.
 * If sys contains additional obstacles they are dumped in
 * another mesh ('Obstacles')
 */
void createBoxGeometry(System *sys, char *filename){
    FILE *file;
    unsigned char rgba[4];

    rgba[0]=255;
    rgba[1]=255;
    rgba[2]=0;
    rgba[3]=50;
    file = fopen(filename, "wb");
    writeMeshGeometry(file, sys->walls, 12, rgba, "Box", 1);
    if(sys->nbWalls>12){
      rgba[3]=150;
      writeMeshGeometry(file, sys->walls+12, sys->nbWalls-12, rgba, "Obstacles", 0);
    }
    fclose(file);
}

/*
 * Main function generating the .par and .geo file.
 * filename should end with '.par'.
 */
void createBox(char* filename){
  int    nbParts;
  float  dt,dtFrame,simTime;
  float  minMax[3][2]={{-1,1},{-1,1},{-1,1}};
  unsigned int nbSteps,nbDtPerFrame,i,j;
  float  radius,mass;
  float  cellSize;
  System *sys;
  clock_t time;
  FILE *file;
  Header *header;
  Step *step;
  char *geometryFilename;

  // Main simulation parameters
  nbParts=20000;  // Number of particles.
  dt=0.002;       // Free flight time step [s].
  dtFrame=0.02;   // 0.02s <=> 50 frames per seconds in .par file.
  simTime=18;     // Simulation duration [s]
  radius=0.02;    // particles radius [m]
  mass=0.1;       // mass [kg]

  // Initialize the system.
  nbSteps=(unsigned int)(simTime/dtFrame+0.5)+1;
  nbDtPerFrame=(unsigned int)(dtFrame/dt+0.5);
  cellSize=2*radius*1.001;
  sys=createSystem(nbParts,minMax,cellSize);
  addWalls(sys);
  setParticles(sys,0,nbParts-1,radius,mass);

  // Create and write the .par file header.
  file = fopen(filename, "wb");
  header = createBoxHeader(radius, nbParts, nbSteps);
  writeHeader(header, file);
  freeHeader(header);

  // Start the simulation and write a new Step in
  // the .par file every dtFrame seconds.
  step = createStep(1, nbParts);
  time=clock();
  updateStep(step, 0, sys->parts, nbParts);
  writeStep(step, file);
  for(i=1; i<nbSteps; i++){
    for(j=0; j<nbDtPerFrame; j++){
      updateSys(sys,(j+1+(i-1)*nbDtPerFrame)*dt);
      printf("time = %e (%.2f%%)\n",sys->time,100*(float)(j+1+(i-1)*nbDtPerFrame)/((nbSteps-1)*nbDtPerFrame));
      fflush(stdout);
    }
    updateStep(step, sys->time, sys->parts, nbParts);
    writeStep(step, file);
  }
  time=clock()-time;
  freeStep(step);
  fclose(file);

  // Simulation done. Generate the .geo file and free allocated memory.
  geometryFilename = (char*)calloc(strlen(filename)+1, sizeof(char));
  strcpy(geometryFilename, filename);
  strcpy(geometryFilename+strlen(filename)-3, "geo");
  createBoxGeometry(sys, geometryFilename);
  free(geometryFilename);
  deleteSystem(sys);
  updateSys(NULL,0);
  printf("done in %f sec!\n", (double)(time)/CLOCKS_PER_SEC);
}
