#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wave.h"
#include "benzene.h"
#include "box.h"

/*
 * Generate several .par files to see how to
 * use particles, bonds, meshes, etc.
 * WARNING: the createBox function generates almost 290MB of data.
 * You can reduce this amount by modifying the hard coded parameters
 * of createBox in box.c.
 */
int main(void)
{
    createWave("wave.par");
    createBenzene("benzene.par");
    createBox("box.par");
    return 0;
}

