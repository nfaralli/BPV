#ifndef __BOX_H__
#define __BOX_H__

/*
 * Generate a .par file containing the animation
 * of 20,000 balls bouncing in a cube.
 * This example is much more complex than the others
 * due to the computation of particle trajectories,
 * forces, collisions, etc. However the generation of
 * the .par file is still quite simple.
 * The variable 'velocity' can be used to color the
 * balls based on their speed, to render their
 * trajectories, to blank specific balls, etc.
 * The geometry file (.geo) of the corresponding system is
 * also generated.
 *
 * The generated .par file is 288.3MB
 * The generated .geo file is 2KB
 */
void createBox(char* filename);

#endif

