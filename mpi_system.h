#ifndef MPI_SYSTEM_H
#define MPU_SYSTEM_H


#include "mpi.h"


// include for cross-reference purposes
#include "global.h"



// MPI variables:
//	how many processors there are, and which rank current machine is...
int rank, num_processors;

// MPI Status variable (used for receive calls)
MPI_Request request;
MPI_Status status;



// MPI INIT: returns rank and size in given pointer variables
void init_mpi(int argc, char **argv);



// HEAD NODE: sends a buffer of values to all nodes as initialized data
void MPISendStatus(int buffer[], int count);

// WORKER NODES: receive the status buffer sent by head node.
void MPIRecvStatus();



/********** RUNTIME REPORT PROTOCOL ***********/
/* This section of the code declares functions used
 *	to communicate between the HEAD node and the
 *	WORKER nodes during runtime. Each update, the
 *	WORKER nodes send position information to the
 *	HEAD node for display updates, and the HEAD node
 *	replies with a message indicating whether or not
 *	the simulation should continue.
 */

// WORKER NODES: sends a position report to the head node
void MPISendPosReport(float buffer[], int count);
// OLD VERSION: void MPISendPosReport(int buffer[], int count);

// HEAD NODE: receive position report
void MPIRecvPosReport(
	float *plants, int num_plants,
	float *herbivores, int num_herbivores,
	float *predators, int num_predators);

// HEAD NODE: send whether or not to continue: 1 for yes, 0 for no.
void MPISendContinue(int TorF);

// WORKER NODES: receive the continue/discontinue package
//	returns: 1 to continue, 0 to stop.
int MPIReceiveContinue();


/**********************************************/
/************* COLLISION NODES ****************/
/**********************************************/


// send and receive collision data (the actual x and y locations)
//	to calculate PLANT-HERBIVORE collisions
void MPISendCollisionPos_PLANTS_HERBIVORES(int buffer[], int count);
void MPIRecvCollisionPos_PLANTS_HERBIVORES(
	int *plants, int num_plants,
	int *herbivores, int num_herbivores);

// send and receive collision data (the actual x and y locations)
//	to calculate HERBIVORE-PREDATOR collisions
void MPISendCollisionPos_HERBIVORES_PREDATORS(int buffer[], int count);
void MPIRecvCollisionPos_HERBIVORES_PREDATORS(
	int *herbivores, int num_herbivores,
	int *predators, int num_predators);

// send and receive death reports of each organims (dead or alive)
//	0 = dead, 1 = alive
void MPISendDeathReports(char buffer[], int count, int destination);
void MPIRecvDeathReports(char buffer[], int count, int source);

// send and receive feed reports of each organism
//	value at each position indicates how many things they ate
void MPISendFeedReports(int buffer[], int count, int destination);
void MPIRecvFeedReports(int buffer[], int count, int source);

/**********************************************/
/**********************************************/
/**********************************************/


// ALL NODES: stops MPI
void MPIDone();



#endif
