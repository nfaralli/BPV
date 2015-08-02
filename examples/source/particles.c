#include "particles.h"

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
                     unsigned int nbSteps) {
    Header *header;
    header = (Header*)calloc(1, sizeof(Header));
    header->nbTypes = nbTypes;
    header->types = (Type*)calloc(nbTypes, sizeof(Type));
    header->nbVariables = nbVariables;
    header->variables = (char(*)[NAME_LENGTH])calloc(nbVariables, sizeof(char[NAME_LENGTH]));
    header->nbParticles = nbParticles;
    header->particlesType = (unsigned char*)calloc(nbParticles, sizeof(unsigned char));
    header->nbBonds = nbBonds;
    header->bondsIndices = (unsigned int(*)[2])calloc(nbBonds, sizeof(unsigned int[2]));
    header->nbMeshes = nbMeshes;
    header->meshes = (Mesh*)calloc(nbMeshes, sizeof(Mesh));
    header->nbSteps = nbSteps;
    return header;
}

/*
 * Release any memory allocated for header (including header itself).
 */
void freeHeader(Header *header) {
    unsigned int i;
    if(header == NULL) {
        return;
    }
    FREE(header->types);
    FREE(header->variables);
    FREE(header->particlesType);
    FREE(header->bondsIndices);
    for(i=0; i<header->nbMeshes; i++) {
        FREE(header->meshes[i].indices);
    }
    FREE(header->meshes);
    FREE(header);
}

/*
 * Helper function used to create a Step structure.
 * Memory is allocated and set to 0 for each array in
 * the Step structure.
 * The calloer owns the new Step pointer.
 */
Step* createStep(unsigned char nbVariables,
                 unsigned int nbParticles) {
    Step *step;
    int i;
    step = (Step*)calloc(1, sizeof(Step));
    step->nbVariables = nbVariables;
    step->nbParticles = nbParticles;
    step->x = (float*)calloc(nbParticles, sizeof(float));
    step->y = (float*)calloc(nbParticles, sizeof(float));
    step->z = (float*)calloc(nbParticles, sizeof(float));
    step->variables = (float**)calloc(nbVariables, sizeof(float*));
    for(i=0; i<nbVariables; i++) {
        step->variables[i] = (float*)calloc(nbParticles, sizeof(float));
    }
    return step;
}

/*
 * Release any memory allocated for step (including step itself).
 */
void freeStep(Step *step) {
    int i;
    if(step == NULL) {
        return;
    }
    FREE(step->x);
    FREE(step->y);
    FREE(step->z);
    for(i=0; i<step->nbVariables; i++) {
        FREE(step->variables[i]);
    }
    FREE(step->variables);
    FREE(step);
}


void writeTypesSection(Header *header, FILE *file) {
    Type *type;
    fwrite(&(header->nbTypes), sizeof(unsigned char), 1, file);
    for(type = header->types; type<header->types + header->nbTypes; type++){
        fwrite(type->name, sizeof(char), NAME_LENGTH, file);
        fwrite(&(type->radius), sizeof(float), 1, file);
        fwrite(type->rgb, sizeof(unsigned char), 3, file);
    }
}

void writeVariablesSection(Header *header, FILE *file) {
    unsigned int i;
    fwrite(&(header->nbVariables), sizeof(unsigned char), 1, file);
    for(i=0; i<3; i++){
        fwrite(header->xyzNames[i], sizeof(char), NAME_LENGTH, file);
    }
    for(i=0; i<header->nbVariables; i++){
        fwrite(header->variables[i], sizeof(char), NAME_LENGTH, file);
    }
}

void writeParticlesTypeSection(Header *header, FILE *file) {
    fwrite(&(header->nbParticles), sizeof(unsigned int), 1, file);
    fwrite(header->particlesType, sizeof(unsigned char), header->nbParticles, file);
}

void writeBondsSection(Header *header, FILE *file) {
    unsigned int i;
    fwrite(&(header->nbBonds), sizeof(unsigned int), 1, file);
    fwrite(&(header->bondRadius), sizeof(float), 1, file);
    for(i=0; i<header->nbBonds; i++){
        fwrite(&(header->bondsIndices[i][0]), sizeof(unsigned int), 1, file);
        fwrite(&(header->bondsIndices[i][1]), sizeof(unsigned int), 1, file);
    }
}

void writeMeshesSection(Header *header, FILE *file) {
    Mesh *mesh;
    fwrite(&(header->nbMeshes), sizeof(unsigned int), 1, file);
    for(mesh = header->meshes; mesh<header->meshes + header->nbMeshes; mesh++) {
        fwrite(mesh->name, sizeof(char), NAME_LENGTH, file);
        fwrite(&(mesh->dim), sizeof(unsigned int), 1, file);
        fwrite(mesh->indices, sizeof(unsigned int[3]), mesh->dim, file);
    }
}

/*
 * Write the header of the .par file.
 */
void writeHeader(Header *header, FILE *file) {
    fwrite("PAR", 3, 1, file);
    writeTypesSection(header, file);
    writeVariablesSection(header, file);
    writeParticlesTypeSection(header, file);
    writeBondsSection(header, file);
    writeMeshesSection(header, file);
    fwrite(&(header->nbSteps), sizeof(unsigned int), 1, file);
}

/*
 * Dump a new step in the .par file.
 */
void writeStep(Step *step, FILE *file) {
    unsigned int i, j;
    fwrite(&(step->time), sizeof(float), 1, file);
    for(i=0; i<step->nbParticles; i++) {
        fwrite(step->x+i, sizeof(float), 1, file);
        fwrite(step->y+i, sizeof(float), 1, file);
        fwrite(step->z+i, sizeof(float), 1, file);
        for(j=0; j<step->nbVariables; j++) {
            fwrite(step->variables[j]+i, sizeof(float), 1, file);
        }
    }
}
