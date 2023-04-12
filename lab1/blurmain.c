#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"
#include "math.h"

#define MAX_RAD 1000

int main(int argc, char **argv)
{
	int radius, xsize, ysize, colmax;
	pixel *src = (pixel *)malloc(sizeof(pixel) * MAX_PIXELS);
	struct timespec stime, etime;
	double w[MAX_RAD];

	/* Take care of the arguments */
	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s radius threads infile outfile\n", argv[0]);
		exit(1);
	}

	radius = atoi(argv[1]);
	if ((radius > MAX_RAD) || (radius < 1))
	{
		fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
		exit(1);
	}

	int threads = atoi(argv[2]);
	int exponent = log2f(threads);
	if ((threads > 64 || threads < 1 || exponent != ceil(exponent)))
	{
		fprintf(stderr, "Threads (%d) must be an element of the 2^n series and <= 64", threads);
		exit(1);
	}

	/* Read file */
	if (read_ppm(argv[3], &xsize, &ysize, &colmax, (char *)src) != 0)
		exit(1);

	if (colmax > 255)
	{
		fprintf(stderr, "Too large maximum color-component value\n");
		exit(1);
	}

	printf("Has read the image, generating coefficients\n");

	/* filter */
	get_gauss_weights(radius, w);

	printf("Calling filter\n");

	clock_gettime(CLOCK_REALTIME, &stime);
	blurfilter(xsize, ysize, src, radius, w, threads);
	clock_gettime(CLOCK_REALTIME, &etime);

	printf("Filtering took: %g secs\n", (etime.tv_sec - stime.tv_sec) +
											1e-9 * (etime.tv_nsec - stime.tv_nsec));

	/* Write result */
	printf("Writing output file\n");

	if (write_ppm(argv[4], xsize, ysize, (char *)src) != 0)
		exit(1);
}
