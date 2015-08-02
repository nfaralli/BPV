#ifndef __PARTICLES_H__
#define __PARTICLES_H__

#include <stdlib.h>
#include <stdio.h>

#define NAME_LENGTH 32
#define FREE(x) {if (x) free(x);}

/*
 * Type structure.
 * Each type has its own name, particle radius and color.
 */
typedef struct _type {
    char name[NAME_LENGTH];
    float radius;
    unsigned char rgb[3];
} Type;

/*
 * Mesh structure.
 * name: name of the mesh.
 * dim: number of triangle defining the mesh.
 * indices: array of 'dim' unsigned int[3]
 * each unsigned int[3] contains the indices of the
 * three particles which define one triangle.
 * The order of the vertices is important as it
 * defines which side of the triangle is visible
 * (basically, if when looking at the triangle the
 * vertices v0, v1, and v2 are ordered in a counter
 * clockwise way, then this face is visible).
 */
typedef struct _mesh {
    char name[NAME_LENGTH];
    unsigned int dim;
    unsigned int (*indices)[3];
} Mesh;

/*
 * Header structure.
 * Contains all the data needed to generate the
 * header of the .par file.
 * It also contains the total number of steps in the file.
 */
typedef struct _header {
    unsigned char nbTypes;
    Type *types;                     // array of 'nbType' Type.
    unsigned char nbVariables;
    char (*variables)[NAME_LENGTH];  // array of 'nbVariables' char[NAME_LENGTH].
    char xyzNames[3][NAME_LENGTH];
    unsigned int nbParticles;
    unsigned char *particlesType;    // array of 'nbParticles' unsigned char.
    unsigned int nbBonds;
    float bondRadius;
    unsigned int (*bondsIndices)[2]; // array of 'nbBonds' unsigned int[2].
    unsigned int nbMeshes;
    Mesh *meshes;                    // array of 'nbMeshes' Mesh.
    unsigned int nbSteps;
} Header;

/*
 * Step structure.
 * contains the position and variable values of each particle at a given time.
 */
typedef struct _step {
    float time;
    unsigned int nbParticles;
    unsigned char nbVariables;
    float *x;  // array of 'nbParticles' float.
    float *y;  // idem
    float *z;  // idem
    float **variables;  // array of 'nbVariables'*'nbParticles' float.
} Step;

/*
 * Helper function used to create a Header structue.
 * Memory is allocated and set to 0 for each array in
 * the Header structure.
 * The caller owns the new Header pointer.
 */
Header* createHeader(unsigned char nbTypes,
                     unsigned char nbVariables,
                     unsigned int nbParticles,
                     unsigned int nbBonds,
                     unsigned int nbMeshes,
                     unsigned int nbSteps);
/*
 * Release any memory allocated for header (including header itself).
 */
void freeHeader(Header *header);
/*
 * Write the header of the .par file.
 */
void writeHeader(Header *header, FILE *file);

/*
 * Helper function used to create a Step structure.
 * Memory is allocated and set to 0 for each array in
 * the Step structure.
 * The calloer owns the new Step pointer.
 */
Step* createStep(unsigned char nbVariables,
                 unsigned int nbParticles);
/*
 * Release any memory allocated for step (including step itself).
 */
void freeStep(Step *step);
/*
 * Dump a new step in the .par file.
 */
void writeStep(Step *step, FILE *file);

#endif
