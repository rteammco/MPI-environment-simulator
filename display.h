#ifndef DISPLAY_H
#define DISPLAY_H


/* Contains functions for global operations */
#include "global.h"


/* Include necessary display utilities (i.e. OpenGL) */
#include <GL/glut.h>


// window size static values
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


/* DISPLAY BUFFERS:
 *	This is where the buffers that store location data for
 *	each organism are located.
 */
float *plant_locs;
float *herbivore_locs;
float *predator_locs;

// 1 if true, 0 if false (stop the simulation)
int simulating;


/* Display methods */
void init_display(int argc, char **argv,
	int num_plants, int num_herbavores, int num_predators);
void init_graphics();

/* GLUT window functions */
void display_func();
void idle_func();
void keyboard_func(unsigned char key, int x, int y);
void mouse_func();


#endif
