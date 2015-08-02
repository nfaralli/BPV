#include "wave.h"
#include "particles.h"
#include <math.h>
#include <string.h>

typedef struct _point{
    int i;
    int j;
} Point;

typedef struct _triangle{
    Point *v0;
    Point *v1;
    Point *v2;
} Triangle;

/*
 * Generate the points of the mesh which are inside the disk
 * of radius 'radius'.
 * nbPoints will contain the number of points generated.
 */
static Point* getPoints(float radius, unsigned int *nbPoints){
    Point *points;
    float v0[2],v1[2];
    int max;
    float x, y;
    int i, j, index;
    float sqnorm, sqradius;

    v0[0]=1;
    v0[1]=0;
    v1[0]=0.5;
    v1[1]=sqrt(3)/2;

    sqradius=radius*radius;
    max=radius*2/sqrt(3);
    if(max<radius*2/sqrt(3)){
        max++;
    }
    points=(Point*)calloc((2*max+1)*(2*max+1), sizeof(Point));
    index=0;
    for(j=-max; j<=max; j++){
        for(i=-max; i<=max; i++){
            x=i*v0[0]+j*v1[0];
            y=i*v0[1]+j*v1[1];
            sqnorm=x*x+y*y;
            if(sqnorm<=sqradius){
                points[index].i=i;
                points[index].j=j;
                index++;
            }
        }
    }
    *nbPoints=index;
    points=(Point*)realloc(points, (*nbPoints)*sizeof(Point));
    return points;
}

/*
 * From an array of Point structures, find the one with
 * coordinates (i,j).
 * Returns a pointer to the corresponding point, or NULL
 * if the array does not contain such a point.
 * The array should be sorted.
 */
static Point* getPoint(Point *points, unsigned int nbPoints, int i, int j){
    Point *point;
    for(point=points; point<points+nbPoints; point++){
        if(point->j<j){
            continue;
        }
        if(point->i==i && point->j==j){
            return point;
        }
        if(point->j>j){
            return NULL;
        }
    }
    return NULL;
}

/*
 * From an array of Point structures, generate all the triangles
 * generating the mesh.
 */
static Triangle* getTriangles(Point *points, unsigned int nbPoints, unsigned int *nbTriangles){
    Triangle *triangles;
    Point *v0, *v1, *v2;
    int index;

    triangles=(Triangle*)calloc(2*nbPoints, sizeof(Triangle));
    index=0;
    for(v0=points; v0<points+nbPoints; v0++){
        //get v1=(i-1,j) v2=(i,j-1)
        v1=getPoint(points, nbPoints, v0->i-1, v0->j);
        v2=getPoint(points, nbPoints, v0->i, v0->j-1);
        if(v1!=NULL && v2!=NULL){
            triangles[index].v0=v0;
            triangles[index].v1=v1;
            triangles[index].v2=v2;
            index++;
        }

        //get v1=(i+1,j) v2=(i,j+1)
        v1=getPoint(points, nbPoints, v0->i+1, v0->j);
        v2=getPoint(points, nbPoints, v0->i, v0->j+1);
        if(v1!=NULL && v2!=NULL){
            triangles[index].v0=v0;
            triangles[index].v1=v1;
            triangles[index].v2=v2;
            index++;
        }
    }
    *nbTriangles=index;
    triangles=(Triangle*)realloc(triangles, (*nbTriangles)*sizeof(Triangle));

    return triangles;
}

/*
 * Generate the Header structure.
 */
static Header* createWaveHeader(float radius, unsigned int nbSteps, Point **points, unsigned int *nbPoints){
    Header *header;
    Triangle *triangles;
    unsigned int nbTriangles;
    unsigned int i;

    *points = getPoints(radius, nbPoints);
    triangles = getTriangles(*points, *nbPoints, &nbTriangles);

    header=createHeader(1, 0, *nbPoints, 0, 1, nbSteps);
    strcpy(header->types[0].name, "wave");
    header->types[0].radius = 0.5;
    header->types[0].rgb[0] = 255;
    header->types[0].rgb[1] = 0;
    header->types[0].rgb[2] = 0;

    strcpy(header->xyzNames[0], "x");
    strcpy(header->xyzNames[1], "y");
    strcpy(header->xyzNames[2], "z");

    for(i=0; i<*nbPoints; i++){
        header->particlesType[i]=0;
    }

    // The createHeader function does not allocated memory for the mesh triangles.
    // Need to do that here.
    strcpy(header->meshes[0].name, "wave");
    header->meshes[0].dim = nbTriangles;
    header->meshes[0].indices = (unsigned int(*)[3])calloc(nbTriangles, sizeof(unsigned int[3]));
    for(i=0; i<nbTriangles; i++){
        header->meshes[0].indices[i][0] = (unsigned int)(triangles[i].v0-*points);
        header->meshes[0].indices[i][1] = (unsigned int)(triangles[i].v1-*points);
        header->meshes[0].indices[i][2] = (unsigned int)(triangles[i].v2-*points);
    }
    free(triangles);

    return header;
}

/*
 * Update the Step structure with the coordinates of each point.
 * The wave is generated using a simple cos function with a varying
 * phase.
 */
static void updateStep(Step *step, float time, Point *points, unsigned int nbPoints){
    unsigned int index;
    float v0[2],v1[2];
    float x, y, dist;

    v0[0]=1;
    v0[1]=0;
    v1[0]=0.5;
    v1[1]=sqrt(3)/2;

    step->time=time;
    for(index=0; index<nbPoints; index++){
        step->x[index]=x=points[index].i*v0[0] + points[index].j*v1[0];
        step->y[index]=y=points[index].i*v0[1] + points[index].j*v1[1];
        dist=sqrt(x*x+y*y);
        step->z[index]=10*cos((2*M_PI/10)*dist-(2*M_PI/30)*time)/(dist+1);
    }
}

/*
 * Main function generating the wave simulation.
 */
void createWave(char* filename){
    FILE *file;
    Header *header=NULL;
    Step *step;
    Point *points;
    float radius = 50;
    unsigned int nbSteps = 301;
    unsigned int nbPoints;
    unsigned int i;

    file = fopen(filename, "wb");

    header = createWaveHeader(radius, nbSteps, &points, &nbPoints);
    writeHeader(header, file);
    freeHeader(header);

    step = createStep(0, nbPoints);
    for(i=0; i<nbSteps; i++) {
        updateStep(step, (float)i, points, nbPoints);
        writeStep(step, file);
    }
    freeStep(step);
    fclose(file);
}
