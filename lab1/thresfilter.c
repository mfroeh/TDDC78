#include "thresfilter.h"
#include <mpi.h>
#include "stdio.h"

// TODO: Remove all the prints
void thresfilter(pixel *buf, int const count, int const N)
{
	printf("%d, %d\n", count, N);
	int sum = 0;
	// Compute average over all pixels
	for (int i = 0; i < count; ++i)
		sum += (int)buf[i].r + (int)buf[i].g + (int)buf[i].b;
	sum /= N;

	int avg;
	MPI_Allreduce(&sum, &avg, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	printf("Hey, my local avg was %d and the received global avg is %d\n", sum, avg);

	// Set values for all my pixels
	for (int i = 0; i < count; ++i)
	{
		int psum = (int)buf[i].r + (int)buf[i].g + (int)buf[i].b;
		if (avg > psum)
			buf[i].r = buf[i].g = buf[i].b = 0;
		else
			buf[i].r = buf[i].g = buf[i].b = 255;
	}
}