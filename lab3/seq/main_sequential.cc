#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <vector>
#include <tuple>
#include <iostream>

#include "coordinate.h"
#include "definitions.h"
#include "physics.h"

#define P 4

// Feel free to change this program to facilitate parallelization.

float rand1()
{
	return (float)(rand() / (float)RAND_MAX);
}

void check(std::vector<pcord_t> &first, std::vector<pcord_t> &second)
{
	unsigned p, pp;

	for (p = 0; p < first.size(); p++)
	{ // for all particles
		if (first[p].has_collided)
			continue;

		/* check for collisions */
		for (pp = p + 1; pp < second.size(); pp++)
		{
			if (second[pp].has_collided)
				continue;
			float t = collide(&first[p], &second[pp]);
			if (t != -1)
			{ // collision
				first[p].has_collided = second[pp].has_collided = true;
				interact(&first[p], &second[pp], t);
				break; // only check collision of two particles
			}
		}
	}
}

std::vector<std::pair<int, int>> get_neighbors(int row, int column)
{
	std::vector<std::pair<int, int>> result;
	if (row == 0)
	{
		result.push_back({row + 1, column});
	}
	else
	{
		result.push_back({row - 1, column});
	}

	if (column > 0)
	{
		result.push_back({row, column - 1});
	}
	if (column < P - 1)
	{
		result.push_back({row, column + 1});
	}
	return result;
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

	std::vector<pcord_t> particles[2][P];
	std::vector<pcord_t> new_particles[2][P];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < P; j++)
		{
			particles[i][j] = {};
			new_particles[i][j] = {};
		}
	}

	srand(time(NULL) + 1234);

	float r, a;
	for (int i = 0; i < INIT_NO_PARTICLES; i++)
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

		int row = (int)floor(temp.y / ((BOX_VERT_SIZE + 1) / 2));
		int column = (int)floor(temp.x / ((BOX_HORIZ_SIZE + 1) / P));
		particles[row][column].push_back(temp);
	}

	unsigned int p, pp;

	/* Main loop */
	for (time_stamp = 0; time_stamp < time_max; time_stamp++)
	{ // for each time stamp

		for (int row = 0; row < 2; row++)
		{
			for (int column = 0; column < P; column++)
			{
				check(particles[row][column], particles[row][column]);
				std::vector<std::pair<int, int>> neighbors = get_neighbors(row, column);
				for (auto &neighbor : neighbors)
				{
					check(particles[row][column], particles[neighbor.first][neighbor.second]);
				}
			}
		}

		for (int row = 0; row < 2; row++)
		{
			for (int column = 0; column < P; column++)
			{
				for (p = 0; p < particles[row][column].size(); p++)
				{
					if (!particles[row][column][p].has_collided)
					{
						feuler(&particles[row][column][p], 1);
						pressure += wall_collide(&particles[row][column][p], wall);
					}
					int new_row = (int)floor(particles[row][column][p].y / ((BOX_VERT_SIZE + 1) / 2));
					int new_column = (int)floor(particles[row][column][p].x / ((BOX_HORIZ_SIZE + 1) / P));
					particles[row][column][p].has_collided = false;
					new_particles[new_row][new_column].push_back(particles[row][column][p]);
				}
			}
		}

		for (int row = 0; row < 2; row++)
		{
			for (int column = 0; column < P; column++)
			{
				// particles[row][column].clear();
				// particles[row][column].insert(particles[row][column].end(), new_particles[row][column].begin(), new_particles[row][column].end());
				// new_particles[row][column].clear();
			}
		}
	}

	printf("Average pressure = %f\n", pressure / (WALL_LENGTH * time_max));

	// free(particles);

	return 0;
}
