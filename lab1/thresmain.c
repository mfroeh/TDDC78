#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#include <assert.h>
#include <mpi.h>

void assert_any_nonzero(unsigned char *buf, int count)
{
	int any = 0;
	for (int i = 0; i < count; ++i)
	{
		if (buf[i] != 0)
		{
			any = 1;
			break;
		}
	}
	assert(any);
}

int main(int argc, char **argv)
{
	int me, p;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	// The whole image (non-null only for P0)
	pixel *src = NULL;
	int xsize, ysize, N;
	if (me == 0)
	{
		src = (pixel *)malloc(sizeof(pixel) * MAX_PIXELS);

		/* Take care of the arguments */
		if (argc != 3)
		{
			fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
			exit(1);
		}

		int colmax;
		/* Read file */
		if (read_ppm(argv[1], &xsize, &ysize, &colmax, (char *)src) != 0)
			exit(1);

		if (colmax > 255)
		{
			fprintf(stderr, "Too large maximum color-component value\n");
			exit(1);
		}
		N = xsize * ysize;
	}

	// Start MPI code
	double start_time = MPI_Wtime();

	// Broadcast the pixel count
	int status;
	status = MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

	int *sendcounts = (int *)malloc(p * sizeof(int));
	int *displs = (int *)malloc(p * sizeof(int));

	// Compute the send counts and their offsets
	int chunksize = N / p;
	for (int i = 0; i < p; ++i)
	{
		sendcounts[i] = chunksize;
		if (i == p - 1)
			sendcounts[i] += N % p;
		sendcounts[i] *= 3;
		displs[i] = 3 * i * chunksize;
	}

	// TODO: Remove
	printf("Hi im %d and I have sendcount %d (%d)\n", me, sendcounts[me], sendcounts[me] / 3);
	printf("Hi im %d and I have offset %d (%d)\n", me, displs[me], displs[me] / 3);

	// Distribute chunks of the image accross the processes
	pixel *buf = malloc(sizeof(unsigned char) * sendcounts[me]);
	status = MPI_Scatterv(src, sendcounts, displs, MPI_UNSIGNED_CHAR, buf, sendcounts[me], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	// TODO; Remove
	assert_any_nonzero((unsigned char *)buf, sendcounts[me]);

	// Apply the filter on our part of the image
	thresfilter(buf, sendcounts[me] / 3, N);

	// Reassemble the image from the filtered parts
	status = MPI_Gatherv(buf, sendcounts[me], MPI_UNSIGNED_CHAR, src, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	double end_time = MPI_Wtime();
	printf("Process %d MPI code took %f\n", me, end_time - start_time);

	MPI_Finalize();

	if (me == 0)
	{
		printf("Writing output file\n");
		if (write_ppm(argv[2], xsize, ysize, (char *)src) != 0)
		{
			printf("Failed to write!\n");
			exit(1);
		}
	}
}
