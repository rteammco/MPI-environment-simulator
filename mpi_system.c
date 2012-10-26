#include "mpi_system.h"


/* INIT MPI SYSTEM
 *	Initializes the MPI library, and returns the rank
 *	of the current node.
 */
void init_mpi(int argc, char **argv){
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("Initialize MPI complete for node %d\n", rank);
}


/* FOR HEAD NODE:
 * send initial data to all nodes not including head,
 *	giving information for all nodes to start working.
 */
void MPISendStatus(int buffer[], int count){
	// loop to all processors aside from 0 (head) and send
	int i;
	for(i = 1; i<=3; i++){
		MPI_Send(buffer, count, MPI_INT, i, 1, MPI_COMM_WORLD);
	}
}


/* FOR WORKER NODES:
 * receive the status buffer sent by head node, and use the data to
 *	establish correct values associated with this specific organism.
 */
void MPIRecvStatus(){
	int init_buffer[NUMBER_OF_ORGANISMS];
	MPI_Recv(init_buffer, NUMBER_OF_ORGANISMS, MPI_INT, 0, 1,
		MPI_COMM_WORLD, &status);

	// collect number of organisms from init type
	num_organisms = init_buffer[organism_type];
}


/* FOR WORKER NODES:
 * send positional reports to the head node, following:
 *	count = twice the number of organisms associated with this node
 *	buffer[] containing x followed by y position for each organism.
 */
void MPISendPosReport(float buffer[], int count){
	MPI_Send(buffer, count, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
}


/* FOR HEAD NODE:
 * receive positional reports from each worker node and return
 *	the buffer as needed to the display system to use.
 */
void MPIRecvPosReport(
				float *plants, int plant_count,
				float *herbivores, int herbivore_count,
				float *predators, int predator_count
					){
	
	int cur_count = 0; // termporary count variable
	
	int i; // receive display data from each processor
	for(i = 1; i<=3; i++){
	
		// receive locations from plants node
		if(i == 1 && plant_loc_count > 0){
			MPI_Recv(plants, num_plants*2,
				MPI_FLOAT, i, 1, MPI_COMM_WORLD, &status);
			// adjust number of plants
			MPI_Get_count(&status,  MPI_FLOAT, &cur_count);
			plant_loc_count = cur_count / 2;
		}
		
		// receive locations from herbivores node
		else if(i == 2 && herbivore_loc_count > 0){
			MPI_Recv(herbivores, num_herbivores*2,
				MPI_FLOAT, i, 1, MPI_COMM_WORLD, &status);
			// adjust number of herbivores
			MPI_Get_count(&status,  MPI_FLOAT, &cur_count);
			herbivore_loc_count = cur_count / 2;
		}
		
		// receive locations from predators node
		else if(i == 3 && predator_loc_count > 0){
			MPI_Recv(predators, num_predators*2,
				MPI_FLOAT, i, 1, MPI_COMM_WORLD, &status);
			// adjust number of predators
			MPI_Get_count(&status,  MPI_FLOAT, &cur_count);
			predator_loc_count = cur_count / 2;
		}
	}
}




// COLLISION NODES:
// send collision data (the actual x and y locations)
//	to get the COLL_PLANTS_HERBIVORES node to calculate collisions
void MPISendCollisionPos_PLANTS_HERBIVORES(int buffer[], int count){
	MPI_Send(buffer, count, MPI_INT, COLL_PLANTS_HERBIVORES, 1, MPI_COMM_WORLD);
}

// receive collision data (the actual x and y locations)
//	to calculate PLANT-HERBIVORE collisions
void MPIRecvCollisionPos_PLANTS_HERBIVORES(
				int *plants, int plant_count,
				int *herbivores, int herb_count){
	
	// get plant location list
	MPI_Recv(plants, plant_count, MPI_INT, PLANTS+1, 1,
		MPI_COMM_WORLD, &status);
	int cur_count;
	MPI_Get_count(&status,  MPI_INT, &cur_count);
	num_plants = cur_count/2;
	
	// get herbivore location list
	MPI_Recv(herbivores, herb_count, MPI_INT, HERBIVORES+1, 1,
		MPI_COMM_WORLD, &status);
	MPI_Get_count(&status,  MPI_INT, &cur_count);
	num_herbivores = cur_count/2;
}

// send collision data (the actual x and y locations)
//	to get the COLL_HERBIVORES_PREDATORS node to calculate collisions
void MPISendCollisionPos_HERBIVORES_PREDATORS(int buffer[], int count){
	MPI_Send(buffer, count, MPI_INT, COLL_HERBIVORES_PREDATORS, 1, MPI_COMM_WORLD);
}

// receive collision data (the actual x and y locations)
//	to calculate HERBIVORE-PREDATOR collisions
void MPIRecvCollisionPos_HERBIVORES_PREDATORS(
				int *herbivores, int herb_count,
				int *predators, int pred_count){
	
	// get herbivore location list
	MPI_Recv(herbivores, herb_count, MPI_INT, HERBIVORES+1, 1,
		MPI_COMM_WORLD, &status);
	int cur_count;
	MPI_Get_count(&status,  MPI_INT, &cur_count);
	num_herbivores = cur_count/2;
	
	// get predator location list
	MPI_Recv(predators, pred_count, MPI_INT, PREDATORS+1, 1,
		MPI_COMM_WORLD, &status);
	MPI_Get_count(&status,  MPI_INT, &cur_count);
	num_predators = cur_count/2;
}


// send death reports of each organims (dead or alive) 0 = dead, 1 = alive
void MPISendDeathReports(char buffer[], int count, int destination){
	MPI_Send(buffer, count, MPI_CHAR, destination, 1, MPI_COMM_WORLD);
}

// receive death reports of each organims (dead or alive) 0 = dead, 1 = alive
void MPIRecvDeathReports(char *buffer, int count, int source){
	MPI_Recv(buffer, count, MPI_CHAR, source, 1, MPI_COMM_WORLD, &status);
}

// send feed reports of each organism
//	value at each position indicates how many things they ate
void MPISendFeedReports(int buffer[], int count, int destination){
	MPI_Send(buffer, count, MPI_INT, destination, 1, MPI_COMM_WORLD);
}

// receive feed reports of each organism
//	value at each position indicates how many things they ate
void MPIRecvFeedReports(int *buffer, int count, int source){
	MPI_Recv(buffer, count, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
}





// HEAD NODE: send whether or not to continue: 1 for yes, 0 for no.
void MPISendContinue(int TorF){
	int i;
	for(i = 1; i<=(num_processors-1); i++){
		MPI_Send(&TorF, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
	}
}

// ALL OTHER NODES: receive the continue/discontinue package
//	returns: 1 to continue, 0 to stop.
int MPIReceiveContinue(){
	int TorF;
	MPI_Recv(&TorF, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
	return TorF;
}


/* ALL NODES:
 * Stops the current node and disassociates it from MPI.
 *	Use this on all nodes to clean up before node's task is done.
 */
void MPIDone(){
	MPI_Finalize();
}
