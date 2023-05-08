#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

#include "coordinate.h"
#include "definitions.h"
#include "physics.h"


//Feel free to change this program to facilitate parallelization.

float rand1(){
	return (float)( rand()/(float) RAND_MAX );
}

void init_collisions(pcord_t* particles, unsigned int max){
	for(unsigned int i=0;i<max;++i)
		particles[i].collided=0;
}

void check_collisions(pcord_t* us, pcord_t* them, unsigned int us_number, unsigned int them_number) {
	int p,pp;
		for (p = 0; p<us_number; ++p) {
			if(us[p].collided) continue;

			for(pp=0; pp<them_number; ++pp) {
				if(them[pp].collided) continue;

				float t=collide(&us[p], &them[pp]);
				if(t!=-1){ // collision
					us[p].collided=them[pp].collided=1;
					interact(&us[p], &them[pp], t);
					break; // only check collision of two particles
				}
			}
		}

}


int main(int argc, char** argv){


	unsigned int time_stamp = 0, time_max;
	float pressure = 0;


	// parse arguments
	if(argc != 2) {
		fprintf(stderr, "Usage: %s simulation_time\n", argv[0]);
		fprintf(stderr, "For example: %s 10\n", argv[0]);
		exit(1);
	}

	time_max = atoi(argv[1]);

	/* Initialize */
	// 1. set the walls
	cord_t wall;
	wall.y0 = wall.x0 = 0;
	wall.x1 = BOX_HORIZ_SIZE;
	wall.y1 = BOX_VERT_SIZE;

	int me_world, num_process, me, coords;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &me_world);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);

	MPI_Comm cyclic_comm;
	MPI_Cart_create(MPI_COMM_WORLD, 1, &num_process, &num_process, 0, &cyclic_comm);
	MPI_Comm_rank(cyclic_comm, &me);

	MPI_Cart_coords(cyclic_comm, me, 1, &coords);

	printf("me_world: %d, me: %d, coords: %d, num_process: %d\n", me_world, me, coords, num_process);

	int src, dst;
	MPI_Cart_shift(cyclic_comm, 0, 1, &src, &dst);

	printf("src: %d, dst: %d\n", src, dst);

	int particles_per_thread = INIT_NO_PARTICLES / num_process;
	int upper_bound = particles_per_thread + INIT_NO_PARTICLES % num_process;
	if(me == num_process - 1) particles_per_thread = upper_bound;

	// 2. allocate particle bufer and initialize the particles
	pcord_t* particles = (pcord_t*) malloc(particles_per_thread*sizeof(pcord_t));
	pcord_t* buffer = (pcord_t*) malloc(upper_bound*sizeof(pcord_t));
	//bool *collisions=(bool *)malloc(INIT_NO_PARTICLES*sizeof(bool) );

	srand( time(NULL) + 1234 );

	float r, a;
	for(int i=0; i<particles_per_thread; i++){
		// initialize random position
		particles[i].x = wall.x0 + rand1()*BOX_HORIZ_SIZE;
		particles[i].y = wall.y0 + rand1()*BOX_VERT_SIZE;

		// initialize random velocity
		r = rand1()*MAX_INITIAL_VELOCITY;
		a = rand1()*2*PI;
		particles[i].vx = r*cos(a);
		particles[i].vy = r*sin(a);
	}


	unsigned int p, pp;

	/* Main loop */
	for (time_stamp=0; time_stamp<time_max; time_stamp++) { // for each time stamp

		init_collisions(particles, particles_per_thread);

		/* Collision check for our local particles */
		check_collisions(particles, particles, particles_per_thread, particles_per_thread);

		/* Round-Robin of particles */
		MPI_Request dummy;
		MPI_Status status;

		int src_particles_number = particles_per_thread;

		pcord_t* ptr = particles;

		for (int i = 0; i < num_process; ++i)
		{
			MPI_Isend(&src_particles_number, 1, MPI_INT, dst, 1, cyclic_comm, &dummy);
			MPI_Isend(ptr, 5 * src_particles_number, MPI_FLOAT, dst, 0, cyclic_comm, &dummy);
			MPI_Recv(&src_particles_number, 1, MPI_INT, src, 1, cyclic_comm, &status);
			MPI_Recv(buffer, 5 * src_particles_number, MPI_FLOAT, src, 0, cyclic_comm, &status);
			ptr = buffer;

			if(i != num_process-1) check_collisions(particles, buffer, particles_per_thread, src_particles_number);
		}



		// move particles that has not collided with another
		for(p=0; p<particles_per_thread; p++)
			if(!particles[p].collided){
				feuler(&particles[p], 1);

				/* check for wall interaction and add the momentum */
				pressure += wall_collide(&particles[p], wall);
			}


	}


	printf("Average pressure = %f\n", pressure / (WALL_LENGTH*time_max));

	MPI_Finalize();

	free(particles);

	return 0;

}

