#include "thresfilter.h"
#include <mpi.h>
#include <stdio.h>

typedef unsigned int uint;

void thresfilter(pixel *buf, int const count, int const N)
{
	uint sum = 0;
	// Compute average over all pixels
	for (int i = 0; i < count; ++i)
		sum += (uint)buf[i].r + (uint)buf[i].g + (uint)buf[i].b;
	sum /= N;

	uint avg;
	MPI_Allreduce(&sum, &avg, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	// Set values for all my pixels
	for (int i = 0; i < count; ++i)
	{
		uint psum = (uint)buf[i].r + (uint)buf[i].g + (uint)buf[i].b;
		if (avg > psum)
			buf[i].r = buf[i].g = buf[i].b = 0;
		else
			buf[i].r = buf[i].g = buf[i].b = 255;
	}
}
