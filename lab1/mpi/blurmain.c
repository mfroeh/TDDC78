#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../ppmio.h"
#include "blurfilter.h"
#include "../gaussw.h"
#include <math.h>
#include <mpi.h>

#define MAX_RAD 1000

int main(int argc, char **argv)
{
	int me, p;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	int radius, xsize, ysize, colmax;
	pixel *src = NULL;
	double w[MAX_RAD];

	/* Take care of the arguments */
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
		exit(1);
	}

	radius = atoi(argv[1]);
	if ((radius > MAX_RAD) || (radius < 1))
	{
		fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
		exit(1);
	}

	if(me == 0) { //P0 only section

		src = (pixel *)malloc(sizeof(pixel) * MAX_PIXELS);

		/* Read file */
		if (read_ppm(argv[2], &xsize, &ysize, &colmax, (char *)src) != 0)
			exit(1);

		if (colmax > 255)
		{
			fprintf(stderr, "Too large maximum color-component value\n");
			exit(1);
		}

		printf("Has read the image, generating coefficients\n");
	}

	/* filter */
	get_gauss_weights(radius, w);

	double start_time = MPI_Wtime();

	//Broadcast ysize and xsize to all processes
	MPI_Bcast(&ysize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&xsize, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* Row-wise Section */

	int *sendcounts = (int *)malloc(p * sizeof(int));
	int *displs = (int *)malloc(p * sizeof(int));

	// Compute the send counts and their offsets
	int rowsPerThread = ysize / p;

	for (int i = 0; i < p; ++i)
	{
		displs[i] = 3 * i * rowsPerThread * xsize;
		if (i == p - 1)
			rowsPerThread += ysize % p;
		sendcounts[i] = rowsPerThread * xsize * 3;
	}

	pixel *buf = malloc(sizeof(unsigned char) * sendcounts[me]);
	MPI_Scatterv(src, sendcounts, displs, MPI_UNSIGNED_CHAR, buf, sendcounts[me], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	pixel* dst = malloc(sizeof(unsigned char) * sendcounts[me]);
	
	int endRow = sendcounts[me] / (3*xsize);

	// Compute the weighted row-wise averages for pixels of the assigned rows
	for (int y = 0; y < endRow; ++y)
		compute_row(y, xsize, radius, w, buf, dst);

	// Gather the results and scatter column-wise
	MPI_Gatherv(dst, sendcounts[me], MPI_UNSIGNED_CHAR, src, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	/* Column-wise Section */

	//Custom data type creation for columns
	MPI_Datatype col, col_type;

	if (me == 0) {
		MPI_Type_vector(ysize,    
				3,                  
				xsize*3,
				MPI_UNSIGNED_CHAR,       
				&col);     

		MPI_Type_commit(&col);
		MPI_Type_create_resized(col, 0, 3*sizeof(unsigned char), &col_type);
		MPI_Type_commit(&col_type);
	}

	int colsPerThread = xsize / p;

	// Compute the send counts and their offsets
	for (int i = 0; i < p; ++i)
	{
		displs[i] = i * colsPerThread;
		if (i == p - 1)
			colsPerThread += xsize % p;
		sendcounts[i] = colsPerThread;
	}

	int recvcount = sendcounts[me] * ysize * 3;

	free(buf);
	buf = malloc(sizeof(unsigned char) * recvcount);

	MPI_Scatterv(src, sendcounts, displs, col_type, buf, recvcount, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	free(dst);
	dst = malloc(sizeof(unsigned char) * recvcount);

	int endColumn = sendcounts[me];

	// Compute the weighted column-wise averages for pixels of the assigned columns (re-using the compute_row function)
	for (int x = 0; x < endColumn; ++x)
		compute_row(x, ysize, radius, w, buf, dst);

	MPI_Gatherv(dst, recvcount, MPI_UNSIGNED_CHAR, src, sendcounts, displs, col_type, 0, MPI_COMM_WORLD);

	double end_time = MPI_Wtime();
	printf("Process %d MPI code took %f\n", me, end_time - start_time);

	MPI_Finalize();

	if(me == 0) {

		/* Write result */
		printf("Writing output file\n");

		if (write_ppm(argv[3], xsize, ysize, (char *)src) != 0)
			exit(1);
	}
}
