#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../ppmio.h"
#include "thresfilter.h"
#include <math.h>

int main(int argc, char **argv)
{
	struct timespec stime, etime;
	int xsize, ysize, N;
	pixel *src = (pixel *)malloc(sizeof(pixel) * MAX_PIXELS);
	printf("HI");

	/* Take care of the arguments */
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s threads infile outfile\n", argv[0]);
		exit(1);
	}
	printf("HI");

	int threads = atoi(argv[1]);
	int exponent = log2f(threads);
	if ((threads > 64 || threads < 1 || exponent != ceil(exponent)))
	{
		fprintf(stderr, "Threads (%d) must be an element of the 2^n series and <= 64", threads);
		exit(1);
	}
	printf("HI");

	int colmax;
	/* Read file */
	if (read_ppm(argv[2], &xsize, &ysize, &colmax, (char *)src) != 0)
		exit(1);
	N = xsize * ysize;

	if (colmax > 255)
	{
		fprintf(stderr, "Too large maximum color-component value\n");
		exit(1);
	}

	printf("HI");
	clock_gettime(CLOCK_REALTIME, &stime);
	thresfilter(xsize, ysize, src, threads);
	clock_gettime(CLOCK_REALTIME, &etime);
	printf("Filtering took: %g secs\n", (etime.tv_sec - stime.tv_sec) + 1e-9 * (etime.tv_nsec - stime.tv_nsec));

	// Write result
	printf("Writing output file\n");
	if (write_ppm(argv[3], xsize, ysize, (char *)src) != 0)
		exit(1);
}
