#ifndef GLOBAL_H
#define GLOBAL_H


// for debugging
#include <stdio.h>

// for standard library applications (e.g. random numbers)
#include <stdlib.h>
#include <time.h>


// include all subsystem files
#include "mpi_system.h"
#include "display.h"


// Number of organisms static (may be adjusted with more added organisms)
#define NUMBER_OF_ORGANISMS 3

// organism type values
#define PLANTS 0
#define HERBIVORES 1
#define PREDATORS 2

// collision processor nodes
#define COLL_PLANTS_HERBIVORES 4
#define COLL_HERBIVORES_PREDATORS 5


/* WORKER NODE VARIABLES:
 * Each worker node will reference these for its individual task.
 *	Head node will ignore these completely.
 */
int organism_type;
int num_organisms;

int num_plants;
int num_herbivores;
int num_predators;


// number of each type of active organisms located on the screen
//	(used by calculation processors, including display and collisions)
int plant_loc_count;
int herbivore_loc_count;
int predator_loc_count;


/* starts and sorts out all subsystems, and initializes display on head node */
void start_sim(int plants, int herbivores, int predators, int argc, char **argv);

/* stop all subsystems, and quit the main program */
void terminate();


#endif
