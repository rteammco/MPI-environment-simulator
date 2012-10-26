#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* ASSOCIATED SIMULATOR PROGRAM LINKS */
#include "global.h"


/* GLOBAL SIMULATION VARIABLES */
int num_plants = 100000;
int num_herbivores = 2000;
int num_predators = 450;




/*************************************************************/
/***** OLD CODE: DOES NOT WORK WITH "make run" AS OF NOW *****/
/*************************************************************/
/* Prints a list of commands that can be used and how... */
void print_help(){
		printf("***EnvSim arguments***\n");
		printf("Note: the \"#\" means a natural number digit ( # > 0 )\n");
		printf("   [none]  :: run program with default variables.\n");
		printf("   -h      :: toggle this help list.\n");
		printf("   -plnt # :: number of plants to initialize.\n");
		printf("   -herb # :: number of herbivores to initialize.\n");
		printf("   -pred # :: number of predators to initialize.\n");
		printf("You may use multiple initialization paramters simultaneously:\n");
		printf("   (e.g.) $ ./envsim -plnt 50 -herb 100 -pred 20\n");
}


/* Processes arguments, and returns the following
 * on the type of argument:
 * 0  :: terminate program (no further action needed)
 * 1  :: continue program merrily */
int process_args(int argc, char **argv){
	if(argc < 1){
		return 1;
	}
	
	// check for invalid argument formatting
	if(argv[0][0] != '-' || strlen(argv[0]) < 2){
		printf("Error: illegal argument: %s\n", argv[0][0]);
		printf("Please use argument -h for help: $ ./envsim -h\n");
		return 0;
	}
	
	// print help statement
	else if(strlen(argv[0]) == 2 && argv[0][1] == 'h'){
		print_help();
		return 0;
	}
	
	// match all valid arguments paired with valid digits to parameters
	else{
		int required_args = 2;
		while(argc >= required_args){
			char *arg1 = argv[required_args-2];
			char *arg2 = argv[required_args-1];
			if(arg1[0] == '-'){
				// get int from text (0 or less is invalid)
				int count = atoi(arg2);
				if(strlen(arg1) == 5 && (count > 0 || (strcmp(arg2, "0") == 0))){
					if(strcmp(arg1, "-plnt") == 0){
						// set plants
						num_plants = count;
						printf("Initialized plants to: %d\n", count);
					}
					else if(strcmp(arg1, "-herb") == 0){
						// set herbivores
						num_herbivores = count;
						printf("Initialized herbivores to: %d\n", count);
					}
					else if(strcmp(arg1, "-pred") == 0){
						// set predators
						num_predators = count;
						printf("Initialized predators to: %d\n", count);
					}
				}
			}
			
			required_args += 2;
		}
	}
	
	return 1;
}
/*************************************************************/
/*************************************************************/
/*************************************************************/


/* MAIN: Program starts here */
int main(int argc, char **argv){
	// if arguments are given, process them
	int arg_result;
	if(argc > 1){
		arg_result = process_args(argc-1, &argv[1]);
	}
	
	// if argument result is null, arguments failed, exit program
	if(!arg_result){
		exit(0);
	}
	
	// call initialize simulation function, and run accordingly
	start_sim(num_plants, num_herbivores, num_predators, argc, argv);
}
