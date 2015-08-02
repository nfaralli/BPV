#include "benzene.h"
#include "particles.h"
#include <stdlib.h>
#include <string.h>

typedef struct _point{
    float x;
    float y;
    float z;
} Point;

/*
 * Generate the Header structure.
 */
Header* createBenzeneHeader(unsigned int nbSteps){
    Header *header;
    int i;

    header=createHeader(2, 0, 12, 12, 0, nbSteps);
    strcpy(header->types[0].name, "Carbon");
    header->types[0].radius = 40;
    header->types[0].rgb[0] = 60;
    header->types[0].rgb[1] = 60;
    header->types[0].rgb[2] = 60;
    strcpy(header->types[1].name, "Hydrogen");
    header->types[1].radius = 40;
    header->types[1].rgb[0] = 255;
    header->types[1].rgb[1] = 255;
    header->types[1].rgb[2] = 255;

    strcpy(header->xyzNames[0], "x [pm]");
    strcpy(header->xyzNames[1], "y [pm]");
    strcpy(header->xyzNames[2], "z [pm]");

    header->bondRadius = 20;
    for(i=0; i<6; i++){
        header->particlesType[i]=0;
        header->particlesType[i+6]=1;
        header->bondsIndices[i][0]=i;
        header->bondsIndices[i][1]=(i+1)%6;
        header->bondsIndices[i+6][0]=i;
        header->bondsIndices[i+6][1]=i+6;
    }

    return header;
}

/*
 * Generate the 12 atoms of the benzene molecule.
 * The first 6 elements are the carbon atoms.
 * The last 6 are the hydrogen atoms.
 */
Point* createAtoms(){
    Point *atoms;
    int i;

    atoms = (Point*)calloc(12,sizeof(Point));
    atoms[0].x=139;
    atoms[6].x=139+109;
    // just rotate the first carbon and hydrogen to get
    // the corrdinates of the remaining atoms.
    for(i=1; i<6; i++){
        atoms[i].x = 0.5*atoms[i-1].x - 0.86602540378*atoms[i-1].z;
        atoms[i].z = 0.86602540378*atoms[i-1].x + 0.5*atoms[i-1].z;
        atoms[i+6].x = 0.5*atoms[i+6-1].x - 0.86602540378*atoms[i+6-1].z;
        atoms[i+6].z = 0.86602540378*atoms[i+6-1].x + 0.5*atoms[i+6-1].z;
    }
    return atoms;
}

/*
 * Update the Step structure with the new atoms position.
 * Apply some random noise, just to create some animation.
 */
void updateStep(Step *step, float time, Point *atoms, unsigned int nbAtoms){
    unsigned int i;
    step->time=time;
    for(i=0; i<nbAtoms; i++){
        step->x[i] = atoms[i].x + rand()*(5.0/RAND_MAX);
        step->y[i] = atoms[i].y + rand()*(5.0/RAND_MAX);
        step->z[i] = atoms[i].z + rand()*(5.0/RAND_MAX);
    }
}

/*
 * Main function running the "simulation"
 * and creating the .par file.
 */
void createBenzene(char* filename){
    FILE *file;
    Header *header=NULL;
    Step *step;
    Point *atoms;
    unsigned int nbSteps = 100;
    unsigned int nbAtoms = 12;
    unsigned int i;

    file = fopen(filename, "wb");

    header = createBenzeneHeader(nbSteps);
    writeHeader(header, file);
    freeHeader(header);

    atoms = createAtoms();
    step = createStep(0, nbAtoms);
    for(i=0; i<nbSteps; i++) {
        updateStep(step, (float)i, atoms, nbAtoms);
        writeStep(step, file);
    }
    free(atoms);
    freeStep(step);
    fclose(file);
}
