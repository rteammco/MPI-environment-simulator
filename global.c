#include "global.h"


/* Initialize all subsystems:
 *	MPI System: initialize and setup cluster system
 *	If rank = 0 (head):
 *		Initialize DISPLAY and render environment
 *	Else:
 *		Await instructions from head!
 */
void start_sim(int plants, int herbivores, int predators,
		int argc, char **argv){
		
	num_plants = plants;
	num_herbivores = herbivores;
	num_predators = predators;
	
	init_mpi(argc, argv);
	
	if(rank == 0){
		// print the initial starting values for organisms
		printf("Head node started... plants=%d, herbs=%d, preds=%d.\n",
			num_plants, num_herbivores, num_predators);
		printf("Simulating a total of %d organisms.\n",
			num_plants + num_herbivores + num_predators);
			
		// send startup data:
		// #plants, #herbivores, #predators, scrnWidth, scrnHeight
		int init_data[NUMBER_OF_ORGANISMS];
		init_data[PLANTS] = num_plants; // PLANTS = 0
		init_data[HERBIVORES] = num_herbivores; // HERBIVORES = 1
		init_data[PREDATORS] = num_predators; // PREDATORS = 2
		MPISendStatus(init_data, NUMBER_OF_ORGANISMS);
		
		
		// note: display idle_func handles all simulation polling events
		//	all buffer receiving activity handled from this point forward
		//	in DISPLAY subsystem (display.c)
		init_display(argc, argv, num_plants, num_herbivores, num_predators);
		terminate();
	}
	
	// if node > 5, this is no good. DO nothing!
	else if(rank > 5){
		// loop aimlessly doing nothing until the head
		//	terminates all operations
		while(MPIReceiveContinue()){
			continue;
		}
		printf("Unused node %d is done.\n", rank);
		MPIDone();
	}
	
	
	
	/*******************************************************/
	/*******************************************************/
	/*** WORKER NODES CODE: HERE IS WHAT WORKER NODES DO ***/
	// run the individual MPI processor and await instructions:
	/* Node 1: handle positioning all PLANTS
	 * Node 2: handle positioning all herbivores
	 * Node 3: handle positioning all predators
	 * Node 4: handle collisions between predators and herbivores
	 * Node 5: handle collisions between herbivores and plants
	*/
	
	
	// NODE 4: herbivore-plant collisions
	else if(rank == COLL_PLANTS_HERBIVORES){ // rank 4
	
		int max_plants = num_plants;
		int max_herbivores = num_herbivores;
		
		// create initial position buffers (defaults to 0)
		int plant_positions[num_plants*2];
		int herbivore_positions[num_herbivores*2];
		
		// create initial death buffer (defaults to (char)0 = alive)
		char plant_deaths[num_plants];
		
		// create initial feed buffer (defaults to 0);
		int herbivore_feed[num_herbivores];
			
		// collision processing loop:
		do{
			// send feed and death data
			MPISendDeathReports(plant_deaths, num_plants, PLANTS+1);
			MPISendFeedReports(herbivore_feed, num_herbivores, HERBIVORES+1);
			
			// receive position data for both (predators and herbivores)
			MPIRecvCollisionPos_PLANTS_HERBIVORES(
						(int*)&plant_positions, max_plants*2,
						(int*)&herbivore_positions, max_herbivores*2);

			// clear out the arrays
			memset(plant_deaths, 0, num_plants*sizeof(char));
			memset(herbivore_feed, 0, num_herbivores*sizeof(int));

			// processes collisions and apply feed and death data
			int i; // index variable
			int j; // index variable
			for(i=0; i<num_plants; i++){
				for(j=0; j<num_herbivores; j++){
					int plantX = plant_positions[i*2];
					int plantY = plant_positions[i*2+1];
					int herbX = herbivore_positions[j*2];
					int herbY = herbivore_positions[j*2+1];
					
					// check for collision in alive herbivores
					if(	plant_deaths[i] == 0 &&
						plantX <= herbX+2 && plantX >= herbX-2 &&
						plantY <= herbY+2 && plantY >= herbY-2){
							plant_deaths[i] = 1;
							//num_deaths++;
							herbivore_feed[j]++;
					}
				}
			}
		}
		while(MPIReceiveContinue());
		
		printf("Plant-Herbivore collision node (%d) done.\n", rank);
		MPIDone();
	}
	
	// NODE 5: predator-herbivore collisions
	else if(rank == COLL_HERBIVORES_PREDATORS){ // rank 5
	
		int max_herbivores = num_herbivores;
		int max_predators = num_predators;
		
		// create initial position buffers (defaults to 0)
		int herbivore_positions[num_herbivores*2];
		int predator_positions[num_predators*2];
		
		// create initial death buffer (defaults to (char)0 = alive)
		char herbivore_deaths[num_herbivores];
		
		// create initial feed buffer (defaults to 0);
		int predator_feed[num_predators];
		
		int num_deaths = 0;
		// collision processing loop:
		do{
			// send feed and death data
			//void MPISendDeathReports(char buffer[], int count, int destination)
			//	PLANTS+1 = the actual nodes (offset from 0, nodes start at 1)
			MPISendDeathReports(herbivore_deaths, num_herbivores, HERBIVORES+1);
			MPISendFeedReports(predator_feed, num_predators, PREDATORS+1);
			// update number of herbivores since last death cycle
			//num_herbivores = num_herbivores - num_deaths;
			//num_deaths = 0;
			
			// receive position data for both (predators and herbivores)
			MPIRecvCollisionPos_HERBIVORES_PREDATORS(
						(int*)&herbivore_positions, max_herbivores*2,
						(int*)&predator_positions, max_predators*2);

			// clear out the arrays
			memset(herbivore_deaths, 0, num_herbivores*sizeof(char));
			memset(predator_feed, 0, num_predators*sizeof(int));

			// processes collisions and apply feed and death data
			int i; // index variable
			int j; // index variable
			for(i=0; i<num_herbivores; i++){
				for(j=0; j<num_predators; j++){
					int herbX = herbivore_positions[i*2];
					int herbY = herbivore_positions[i*2+1];
					int predX = predator_positions[j*2];
					int predY = predator_positions[j*2+1];
					
					// check for collision in alive herbivores
					if(	herbivore_deaths[i] == 0 &&
						herbX <= predX+1 && herbX >= predX-1 &&
						herbY <= predY+1 && herbY >= predY-1){
							herbivore_deaths[i] = 1;
							//num_deaths++;
							predator_feed[j]++;
					}
				}
			}
			
			//do float while int true if for 5 double "times"
			
		}
		while(MPIReceiveContinue());
		
		printf("Herbivore-Predator collision node (%d) done.\n", rank);
		MPIDone();
	}
	
	
	// NODE 1-3 (moving herbivores, plants, predators)
	else {
		// establish organism type:
		// 0 = plant
		// 1 = herbivore
		// 2 = predator
		organism_type = rank - 1;
		
		// collect status from head node
		//	this will tell how many organisms this node will have
		//	have to deal with.
		MPIRecvStatus();
		
		// statistics data
		int num_eaten = 0;
		int num_starved = 0;
		int num_reproductions = 0;
		
		// create position and movement arrays for each organism
		//	each organism is an index in all four arrays
		int positions[num_organisms*2]; // absolute positions
		float posF[num_organisms*2]; // relative GL positions
		int x_velocity[num_organisms]; // x velocities
		int y_velocity[num_organisms]; // y velocities
		
		// set values for minumum and maximum X, Y positions
		int x_min = 15;
		int y_min = 15;
		int x_max = WINDOW_WIDTH - 30;
		int y_max = WINDOW_HEIGHT - 30;
		
		// create random x and y positions, and random x and y
		//	velocities for each organism
		unsigned int randSeed = (unsigned int)time(NULL);
		srand(randSeed);
		// insert randomly generated positions/velocities into arrays
		int i;
		for(i=0; i<num_organisms; i++){
			// random works as follows:
			//	rand() % x		: generates a pseudonumber from 0 to (x-1)
			//	then add offsets
			positions[2*i] = (rand() % x_max + 15);
			positions[2*i+1] = (rand() % y_max + 15);
			if(organism_type == PLANTS){
				x_velocity[i] = 0;
				y_velocity[i] = 0;
			}
			else{
				x_velocity[i] = (rand() % 10 + 1);
				y_velocity[i] = (rand() % 10 + 1);
				int dir = (rand() % 2);
				if(dir == 0)
					x_velocity[i] *= -1;
				dir = (rand() % 2);
				if(dir == 0)
					y_velocity[i] *= -1;
			}
		}
		
		// feed buffer
		int total_feeds[num_organisms];
		memset(total_feeds, 0, num_organisms*sizeof(int));
		
		// loop until all ogranisms are dead
		while(num_organisms > 0){
				
			// update all positions, and check for possible reversal
			//	of velocity (if out of bounds!)
			for(i=0; i<num_organisms; i++){
				// update x-position
				positions[2*i] += x_velocity[i];
				// if x position is out of bounds, reverse velocity
				if(positions[2*i] < x_min || positions[2*i] > x_max){
					x_velocity[i] *= -1;
					positions[2*i] += x_velocity[i];
				}
				
				// update y-position
				positions[2*i+1] += y_velocity[i];
				// if y position is out of bounds, reverse velocity
				if(positions[2*i+1] < y_min || positions[2*i+1] > y_max){
					y_velocity[i] *= -1;
					positions[2*i+1] += y_velocity[i];
				}
			}
			
			// death and feed buffers
			char deaths[num_organisms];
			int feeds[num_organisms];
			
			// send collision detecting nodes all data
			if(organism_type == PLANTS){
				// get death data
				MPIRecvDeathReports((char*)&deaths, num_organisms,
					COLL_PLANTS_HERBIVORES);
				// update deaths
				for(i=0; i<num_organisms; i++){
					if(deaths[i] == (char)1){
						// move last organism to this position
						//	and decrease number of organisms
						positions[2*i] = positions[2*num_organisms-2];
						positions[2*i+1] = positions[2*num_organisms-1];
						deaths[i] = deaths[num_organisms-1];
						total_feeds[i] = total_feeds[num_organisms-1];
						num_organisms--;
						i--;
						num_eaten++;
						//printf("killed a plant! Now we have %d\n", num_organisms);
					}
				}
				
				// regrow a new plant if under limit
				if(num_organisms < num_plants && num_organisms != 0){
					// generate 1 new plant for every 100 alive
					int added = 0;
					i=0;
					while((num_organisms+added) < num_plants && i < num_organisms){
						positions[2*num_organisms-2] = (rand() % x_max + 15);
						positions[2*num_organisms-1] = (rand() % y_max + 15);
						total_feeds[num_organisms-1] = 0;
						added++;
						i+=30;
					}
					num_organisms += added;
					num_reproductions++;
				}
				
				// send to node COLL_PLANTS_HERBIVORES
				MPISendCollisionPos_PLANTS_HERBIVORES(
					positions, num_organisms*2);
			}
			
			else if(organism_type == HERBIVORES){
				
				// receive feed reports from COLL_PLANTS_HERBIVORES
				MPIRecvFeedReports((int*)&feeds, num_organisms,
					COLL_PLANTS_HERBIVORES);
				
				// receive death reports from COLL_HERBIVORES_PREDATORS
				MPIRecvDeathReports((char*)&deaths, num_organisms,
					COLL_HERBIVORES_PREDATORS);
				
				// check for reproductions
				for(i=0; i<num_organisms; i++){
					total_feeds[i]--;
					//printf("After: %d\n", total_feeds[i]);
					total_feeds[i] += 10*feeds[i];
					// organism starves if it hasn't fed
					if(total_feeds[i] < -100){
						// move last organism to this position
						//	and decrease number of organisms
						positions[2*i] = positions[2*num_organisms-2];
						positions[2*i+1] = positions[2*num_organisms-1];
						feeds[i] = feeds[num_organisms-1];
						total_feeds[i] = total_feeds[num_organisms-1];
						num_organisms--;
						i--;
						num_starved++;
					}
					// organism reproduces
					if(total_feeds[i] >= 10 && num_organisms < num_herbivores){
						positions[2*num_organisms-2] = (rand() % x_max + 15);
						positions[2*num_organisms-1] = (rand() % y_max + 15);
						x_velocity[num_organisms] = (rand() % 10 + 1);
						y_velocity[num_organisms] = (rand() % 10 + 1);
						int dir = (rand() % 2);
						if(dir == 0)
							x_velocity[num_organisms] *= -1;
						dir = (rand() % 2);
						if(dir == 0)
							y_velocity[num_organisms] *= -1;
						total_feeds[num_organisms] = 0;
						// create a new herbivore at a random position
						num_organisms++;
						num_reproductions++;
					}
				}
				
				// check for deaths
				for(i=0; i<num_organisms; i++){
					if(deaths[i] == (char)1){
						// move last organism to this position
						//	and decrease number of organisms
						positions[2*i] = positions[2*num_organisms-2];
						positions[2*i+1] = positions[2*num_organisms-1];
						deaths[i] = deaths[num_organisms-1];
						total_feeds[i] = total_feeds[num_organisms-1];
						num_organisms--;
						i--;
						num_eaten++;
					}
				}
			
			
				// send to node COLL_PLANTS_HERBIVORES
				MPISendCollisionPos_PLANTS_HERBIVORES(
					positions, num_organisms*2);
				
				//	and COLL_HERBIVORES_PREDATORS
				MPISendCollisionPos_HERBIVORES_PREDATORS(
					positions, num_organisms*2);
				
			}
			
			else if(organism_type == PREDATORS){
				
				// receive feed reports from COLL_HERBIVORES_PREDATORS
				MPIRecvFeedReports((int*)&feeds, num_organisms,
					COLL_HERBIVORES_PREDATORS);
				
				// check feed data:
				for(i=0; i<num_organisms; i++){
					//printf("Before: %d\n", total_feeds[i]);
					total_feeds[i]--;
					//printf("After: %d\n", total_feeds[i]);
					total_feeds[i] += 20*feeds[i];
					// organism starves if it hasn't fed
					if(total_feeds[i] < -1000){
						// move last organism to this position
						//	and decrease number of organisms
						positions[2*i] = positions[2*num_organisms-2];
						positions[2*i+1] = positions[2*num_organisms-1];
						feeds[i] = feeds[num_organisms-1];
						total_feeds[i] = total_feeds[num_organisms-1];
						num_organisms--;
						i--;
						num_starved++;
					}
					// organism reproduces
					if(total_feeds[i] >= 10 && num_organisms < num_predators){
						positions[2*num_organisms] = (rand() % x_max + 15);
						positions[2*num_organisms+1] = (rand() % y_max + 15);
						x_velocity[num_organisms] = (rand() % 10 + 1);
						y_velocity[num_organisms] = x_velocity[num_organisms];
						total_feeds[num_organisms] = 0;
						// create a new predator at a random position
						num_organisms++;
						num_reproductions++;
					}
				}
				
				// send to node COLL_HERBIVORES_PREDATORS
				MPISendCollisionPos_HERBIVORES_PREDATORS(
					positions, num_organisms*2);
			}
			
			
			// ERROR CHECK:
			//	make sure number of organisms is not negative
			if(num_organisms < 0)
				num_organisms = 0;
				
			
			// update all positions in the OpenGL float format
			//	to display in the head node
			for(i=0; i<num_organisms; i++){
				// scale the x position to relative (-1, 1) scale
				//	for OpenGL to render
				float xpos = (float)positions[2*i] / WINDOW_WIDTH;
				xpos = xpos * 2 - 1.0;
				posF[2*i] = xpos;
				
				// scale the y position to relative (-1, 1) scale
				//	for OpenGL to render
				float ypos = (float)positions[2*i+1] / WINDOW_HEIGHT;
				ypos = ypos * 2 - 1.0;
				posF[2*i+1] = ypos;
			}
			
			
			// send new positions (posF is the relative float positions
			//	for OpenGL to display)
			MPISendPosReport(posF, num_organisms*2);
			
			// receive acknowledgement / report
			if(!MPIReceiveContinue()){
				// if ack is not received (Head node called for stop
				//	of operation), break the loop
				break;
			}
		}
		
		// once loop is broken, finish node
		//	loop is broken if:
		//		1) Head node terminates parallel operation
		//	 or 2) This node's organism goes extinct
		printf("Organism location node (%d) is done.\n", rank);
		printf("(%d) ### STATISTICS:\n", rank);
		printf("(%d) #### Got eaten: %d\n", rank, num_eaten);
		printf("(%d) #### Starved to death: %d\n", rank, num_starved);
		printf("(%d) #### Times reproduced: %d\n", rank, num_reproductions);
		MPIDone();
	}
	/****************** WORKER NODES END *******************/
	/*******************************************************/
	/*******************************************************/
}


/* FOR HEAD NODE:
 *	Stops MPI, and sends a "do not continue" message to all
 *	worker nodes.
 */
void terminate(){
	printf("----------------------------------------\n");
	printf("Head node (0) terminated all operations.\n");
	printf("----------------------------------------\n");
	MPISendContinue(0); // 0 = false
	MPIDone(); // stop MPI
	exit(0); // quit program
}
