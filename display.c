#include "display.h"

clock_t start_time;

/* Initialize LOCATION ARRAYS: use the number of organisms provided
 *	to setup properly-sized display buffers.
 * Initialize GLUT: (setup display functions and all necessary
 *	Windowing utilities
 */
void init_display(int argc, char **argv,
		int num_plants, int num_herbivores, int num_predators){
	
	// start timer
	start_time = clock();
	
	// true
	simulating = 1;
	
	// definie number of organisms in global size variables
	plant_loc_count = num_plants;
	herbivore_loc_count = num_herbivores;
	predator_loc_count = num_predators;
	
	
	// allocate array memory:
	//	number of that organism, times 2 (one for each coordinate:
	//		that is, 1 for x, 1 for y...
	//	then times sizeof(int), since int has (typically) 4 bytes,
	//		we need to allocate 4 bytes per single coordinate int.
	plant_locs = (float*)(malloc(plant_loc_count * 2 * sizeof(float)));
	herbivore_locs = (float*)(malloc(herbivore_loc_count * 2 * sizeof(float)));
	predator_locs = (float*)(malloc(predator_loc_count * 2 * sizeof(float)));
		
	// fill arrays up!
	MPIRecvPosReport(
		plant_locs, plant_loc_count * 2,
		herbivore_locs, herbivore_loc_count * 2,
		predator_locs, predator_loc_count * 2);
		
	// initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("EnvSim Display UI");
	
	// initialize GLUT functions (display, idle, keyboard, close)
	glutDisplayFunc(display_func);
	glutIdleFunc(idle_func);
	glutKeyboardFunc(keyboard_func);
	glutCloseFunc(terminate);
	
	// start main loop
	glutMainLoop();
}

/* Initialize GL: set up OpenGL properties */
void init_graphics(){
	glMatrixMode(GL_PROJECTION);
	glPointSize(10.0f);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}


/* GLUT DISPLAY FUNCTION: what happens for every frame
 *	is updated for OpenGL graphics here.
 */
void display_func(){
	// clear and ready for display
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// set size of each rendered point!
	glPointSize(4.0);
	
	// DISPLAY ORGANISMS!
	glBegin(GL_POINTS); // start creating POINT vertices
	
		// display plants:
		glColor3f(0.0, 1.0, 0.0); // green
		int i;
		for(i = 0; i<plant_loc_count; i++){
			glVertex2f(plant_locs[i*2], plant_locs[i*2+1]);
		}
		
		// display herbivores
		glColor3f(0.0, 0.0, 1.0); // blue
		for(i = 0; i<herbivore_loc_count; i++){
			glVertex2f(herbivore_locs[i*2], herbivore_locs[i*2+1]);
		}
		
		// display predators
		glColor3f(1.0, 0.0, 0.0); // red
		for(i = 0; i<predator_loc_count; i++){
			glVertex2f(predator_locs[i*2], predator_locs[i*2+1]);
		}
		
	glEnd(); // finish making vertices

	// display the updated buffer
	glutSwapBuffers();
}


/* GLUT IDLE FUNCTION: collects updates from every node, and stores
 *	them in proper display buffers, later used to render with OpenGL's
 *	display function.
 */
void idle_func(){
	// respond positively to all nodes
	MPISendContinue(simulating); // 1 = true
	
	if(simulating == 0){
		terminate();
	}

	// fill arrays up!
	MPIRecvPosReport(
		plant_locs, plant_loc_count * 2,
		herbivore_locs, herbivore_loc_count * 2,
		predator_locs, predator_loc_count * 2);
	
	if(plant_loc_count == 0){
		printf("----------------------------------------------\n");
		printf("::::: Simulation over: Plant extinction. :::::\n");
		printf("::::: Herbivores: %d     Predators: %d\n",
			herbivore_loc_count, predator_loc_count);
		printf("----------------------------------------------\n");
		simulating = 0;
	}
	else if(herbivore_loc_count == 0){
		printf("--------------------------------------------------\n");
		printf("::::: Simulation over: Herbivore extinction. :::::\n");
		printf("::::: Plants: %d     Predators: %d\n",
			plant_loc_count, predator_loc_count);
		printf("--------------------------------------------------\n");
		simulating = 0;
	}
	else if(predator_loc_count == 0){
		printf("-------------------------------------------------\n");
		printf("::::: Simulation over: Predator extinction. :::::\n");
		printf("::::: Plants: %d     Herbivores: %d\n",
			plant_loc_count, herbivore_loc_count);
		printf("-------------------------------------------------\n");
		simulating = 0;
	}


	if(simulating == 0){ // report final runtime
		clock_t now_time = clock();
		double time_diff =  (double)(now_time - start_time) / CLOCKS_PER_SEC;;
		printf("Simulation runtime (in seconds): %f\n", time_diff);
	}
	// tell OpenGL to refresh (call display function again)
	glutPostRedisplay();
}


/* GLUT KEYBOARD FUNCTION: listens to keyboard input, and acts
 *	appropriately if a key is pressed that is defined below.
 */
void keyboard_func(unsigned char key, int x, int y){
	// if key is (q - lowercase) or (escape),
	//	call terminate(): exit the program.
	if(key == 'q' || key == 27){
		terminate();
	}
}

/* GLUT MOUSE FUNCTION: listens to mouse input, and acts
 *	appropriately if the mouse is clicked within the GLUT window.
 */
void mouse_func(){
	// does nothing at the moment.
	//	later, can be used for user interfacing with the simulation
}
