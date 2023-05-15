#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <vector>

#include "coordinate.h"
#include "definitions.h"
#include "physics.h"

#define P 128

// Feel free to change this program to facilitate parallelization.

float rand1()
{
	return (float)(rand() / (float)RAND_MAX);
}

void check(std::vector<pcord_t> &first, std::vector<pcord_t> &second)
{
	for (size_t p = 0; p < first.size(); p++)
	{
		if (first[p].has_collided)
			continue;

		/* check for collisions */
		for (size_t pp = p + 1; pp < second.size(); pp++)
		{
			if (second[pp].has_collided)
				continue;

			float t = collide(&first[p], &second[pp]);
			if (t != -1)
			{
				first[p].has_collided = second[pp].has_collided = true;
				interact(&first[p], &second[pp], t);
				break; // only check collision of two particles
			}
		}
	}
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

	struct timespec stime, etime;
	clock_gettime(CLOCK_REALTIME, &stime);

	time_max = atoi(argv[1]);

	/* Initialize */
	// 1. set the walls
	cord_t wall;
	wall.y0 = wall.x0 = 0;
	wall.x1 = BOX_HORIZ_SIZE;
	wall.y1 = BOX_VERT_SIZE;

	struct timespec stime, etime;
	clock_gettime(CLOCK_REALTIME, &stime);

	std::vector<pcord_t> particles[2 * P]{};
	std::vector<pcord_t> new_particles[2 * P]{};

	srand(time(NULL) + 1234);

	float r, a;
	for (size_t i = 0; i < INIT_NO_PARTICLES; i++)
	{
		// initialize random position
		pcord_t temp;
		temp.x = wall.x0 + rand1() * BOX_HORIZ_SIZE;
		temp.y = wall.y0 + rand1() * BOX_VERT_SIZE;
		temp.has_collided = false;

		// initialize random velocity
		r = rand1() * MAX_INITIAL_VELOCITY;
		a = rand1() * 2 * PI;
		temp.vx = r * cos(a);
		temp.vy = r * sin(a);

		size_t box{static_cast<size_t>(floor(temp.x / ((BOX_HORIZ_SIZE + 1) / (2 * P))))};
		particles[box].push_back(temp);
	}

	/* Main loop */
	for (time_stamp = 0; time_stamp < time_max; ++time_stamp)
	{ // for each time stamp

		for (size_t box = 0; box < 2 * P; box += 2)
		{
			// A - A
			check(particles[box], particles[box]);
			// B - B
			check(particles[box + 1], particles[box + 1]);
			// A - B
			check(particles[box], particles[box + 1]);
			// B - C
			if (box != 2 * P - 2)
				check(particles[box + 1], particles[box + 2]);
		}

		for (size_t box = 0; box < 2 * P; ++box)
		{
			for (size_t p = 0; p < particles[box].size(); ++p)
			{
				auto &particle = particles[box][p];
				if (!particle.has_collided)
				{
					feuler(&particles[box][p], 1);
					pressure += wall_collide(&particles[box][p], wall);
				}

				size_t new_box{static_cast<size_t>(floor(particle.x / ((BOX_HORIZ_SIZE + 1) / (2 * P))))};
				particle.has_collided = false;
				new_particles[new_box].push_back(particle);
			}
		}

		for (size_t box = 0; box < 2 * P; ++box)
		{
			particles[box].clear();
			particles[box].insert(particles[box].begin(), new_particles[box].begin(), new_particles[box].end());
			new_particles[box].clear();
		}
	}

	clock_gettime(CLOCK_REALTIME, &etime);
	printf("Time taken: %f\n", (etime.tv_sec - stime.tv_sec) + 1e-9 * (etime.tv_nsec - stime.tv_nsec));

	printf("Average pressure = %f\n", pressure / (WALL_LENGTH * time_max));

	// free(particles);

	return 0;
}
