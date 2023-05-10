#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>
#include <vector>

#include "coordinate.h"
#include "definitions.h"
#include "physics.h"

// Feel free to change this program to facilitate parallelization.

float rand1()
{
	return (float)(rand() / (float)RAND_MAX);
}

pcord_t get_random_particle(cord_t wall)
{
	float r, a;
	pcord_t temp;
	// initialize random position
	temp.x = wall.x0 + rand1() * BOX_HORIZ_SIZE;
	temp.y = wall.y0 + rand1() * BOX_VERT_SIZE;

	// initialize random velocity
	r = rand1() * MAX_INITIAL_VELOCITY;
	a = rand1() * 2 * PI;
	temp.vx = r * cos(a);
	temp.vy = r * sin(a);

	return temp;
}

int main(int argc, char **argv)
{

	unsigned int time_stamp = 0, time_max;
	float pressure = 0;

	// parse arguments
	if (argc != 2)
	{
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

	srand(time(NULL) + 1234);

	int me, p;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	int sendcounts[p]{};
	int displs[p]{};
	std::vector<pcord_t> buffer{};
	if (me == 0)
	{
		std::vector<std::vector<pcord_t>> boxes(2 * p);
		for (int i = 0; i < INIT_NO_PARTICLES; i++)
		{
			pcord_t particle = get_random_particle(wall);
			boxes[particle.x / ((BOX_HORIZ_SIZE + 1) / p)].push_back(particle);
		}

		for (size_t i{}, j{}; i < p; ++i, j += 2)
		{
			displs[i] = buffer.size();
			buffer.insert(buffer.end(), boxes[j].begin(), boxes[j].end());
			buffer.insert(buffer.end(), boxes[j + 1].begin(), boxes[j + 1].end());
			sendcounts[i] = boxes[j].size() + boxes[j + 1].size();
		}
	}

	MPI_Datatype pcord_mpi;
	MPI_Type_vector(1, 5, 0, MPI_FLOAT, &pcord_mpi);
	MPI_Type_commit(&pcord_mpi);

	int particle_count;
	MPI_Scatter(sendcounts, 1, MPI_INT, &particle_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::vector<pcord_t> particles{particle_count};
	MPI_Scatterv(buffer.data(), sendcounts, displs, pcord_mpi, &particles, particle_count, pcord_mpi, 0, MPI_COMM_WORLD);

	/* Main loop */
	for (time_stamp = 0; time_stamp < time_max; time_stamp++)
	{ // for each time stamp
	}

	printf("Average pressure = %f\n", pressure / (WALL_LENGTH * time_max));

	MPI_Finalize();

	return 0;
}
