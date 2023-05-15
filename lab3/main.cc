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

	temp.has_collided = 0;

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
	MPI_Status status;
	MPI_Request request;

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
			boxes[particle.x / ((BOX_HORIZ_SIZE + 1) / (2 * p))].push_back(particle);
		}

		for (size_t i{}, j{}; i < p; ++i, j += 2)
		{
			displs[i] = buffer.size();
			buffer.insert(buffer.end(), boxes[j].begin(), boxes[j].end());
			buffer.insert(buffer.end(), boxes[j + 1].begin(), boxes[j + 1].end());
			sendcounts[i] = boxes[j].size() + boxes[j + 1].size();
		}

		for (size_t i{}; i < p; ++i)
			std::cout << i << ": " << sendcounts[i] << ", " << displs[i] << std::endl;
	}

	// pcord_t MPI type
	MPI_Datatype pcord_mpi;
	MPI_Type_vector(1, 5, 0, MPI_FLOAT, &pcord_mpi);
	MPI_Type_commit(&pcord_mpi);

	// Scatter the particle counts
	int particle_count;
	MPI_Scatter(sendcounts, 1, MPI_INT, &particle_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Scatter the particles themselves
	std::vector<pcord_t> particles{particle_count};
	MPI_Scatterv(buffer.data(), sendcounts, displs, pcord_mpi, particles.data(), particle_count, pcord_mpi, 0, MPI_COMM_WORLD);

	// Split up our particles in two sets
	// A = [lower, boundary) and B = [boundary, upper)
	std::vector<pcord_t> A{}, B{};
	float lower{me * (BOX_HORIZ_SIZE / p)};
	float upper{(me + 1) * (BOX_HORIZ_SIZE / p)};
	float boundary{(lower + upper) / 2};
	for (auto &p : particles)
	{
		if (p.x < boundary)
			A.push_back(p);
		else
			B.push_back(p);
	}

	unsigned recv_count, send_count;
	std::vector<pcord_t> C{};
	/* Main loop */
	for (time_stamp = 0; time_stamp < time_max; ++time_stamp)
	{
		for (auto &p : A)
			p.has_collided = false;
		for (auto &p : B)
			p.has_collided = false;
		C.clear();

		// Check AA, BB, AB
		check(A, A);
		check(B, B);
		check(A, B);

		// First one only receives
		if (me == 0)
		{
			MPI_Recv(&recv_count, 1, MPI_UNSIGNED, me + 1, 0, MPI_COMM_WORLD, &status);
			C.reserve(recv_count);
			MPI_Recv(C.data(), recv_count, pcord_mpi, me + 1, 1, MPI_COMM_WORLD, &status);
			check(B, C);
			// TODO:
		}
		// Last one only sends
		else if (me == p - 1)
		{
			send_count = A.size();
			MPI_Isend(&send_count, 1, MPI_UNSIGNED, me - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(A.data(), A.size(), pcord_mpi, me - 1, 1, MPI_COMM_WORLD, &request);
			// TODO:
		}
		// Everyone else sends and receives
		else
		{
			// Send
			send_count = A.size();
			MPI_Isend(&send_count, 1, MPI_UNSIGNED, me - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(A.data(), A.size(), pcord_mpi, me - 1, 1, MPI_COMM_WORLD, &request);
			std::cout << me << ": Sent out: " << send_count << std::endl;

			// Receive
			MPI_Recv(&recv_count, 1, MPI_UNSIGNED, me + 1, 0, MPI_COMM_WORLD, &status);
			C.reserve(recv_count);
			MPI_Recv(C.data(), recv_count, pcord_mpi, me + 1, 1, MPI_COMM_WORLD, &status);

			std::cout << me << ": Received: " << recv_count << std::endl;
			check(B, C);

			MPI_Isend(C.data(), C.size(), pcord_mpi, me + 1, 2, MPI_COMM_WORLD, &request);
			MPI_Recv(A.data(), A.size(), pcord_mpi, me - 1, 2, MPI_COMM_WORLD, &status);
			// TODO:
		}

		MPI_Barrier(MPI_COMM_WORLD);

		// Check which particles collided in BC
		// Check which particles should move for
		// A -> B, B -> A, B -> C, C -> B
		// TODO: And more ...
	}

	printf("Average pressure = %f\n", pressure / (WALL_LENGTH * time_max));

	MPI_Finalize();

	return 0;
}
