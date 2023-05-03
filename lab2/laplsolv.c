
//-----------------------------------------------------------------------
// Serial program for solving the heat conduction problem
// on a square using the Jacobi method.
// Written by August Ernstsson 2015-2019
//-----------------------------------------------------------------------

#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <omp.h>

double timediff(struct timespec *begin, struct timespec *end)
{
	double sec = 0.0, nsec = 0.0;
	if ((end->tv_nsec - begin->tv_nsec) < 0)
	{
		sec = (double)(end->tv_sec - begin->tv_sec - 1);
		nsec = (double)(end->tv_nsec - begin->tv_nsec + 1000000000);
	}
	else
	{
		sec = (double)(end->tv_sec - begin->tv_sec);
		nsec = (double)(end->tv_nsec - begin->tv_nsec);
	}
	return sec + nsec / 1E9;
}

void printm(int n, double *M)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			printf("%f\t", *(M + n * i + j));
		}
		printf("\n");
	}
	printf("\n");
}

void arrcpy(double *dst, double *src, int len)
{
	for (int it = 0; it < len; it++)
		dst[it] = src[it];
}

void laplsolv(int n, int maxiter, double tol)
{
	double T[n + 2][n + 2];
	double tmp1[n], tmp2[n], tmp3[n];
	int k;

	struct timespec starttime, endtime;

	// Set boundary conditions and initial values for the unknowns
	for (int i = 0; i <= n + 1; ++i)
	{
		for (int j = 0; j <= n + 1; ++j)
		{
			if (i == n + 1)
				T[i][j] = 2;
			else if (j == 0 || j == n + 1)
				T[i][j] = 1;
			else
				T[i][j] = 0;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &starttime);

	// Solve the linear system of equations using the Jacobi method
	for (k = 0; k < maxiter; ++k)
	{
		double error = -INFINITY;

		// Loop for each of this thread's rows
#pragma omp parallel private(tmp1, tmp2)
		{

			int p = omp_get_num_threads();
			int me = omp_get_thread_num();

			int chunk_size = n / p;
			int start = chunk_size * me + 1;
			if (me == p - 1)
				chunk_size += n % p;
			int end = start + chunk_size + 1;

			arrcpy(tmp1, &T[start - 1][1], n);

			double row_after_chunk[n];
			arrcpy(row_after_chunk, &T[end][1], n);

#pragma omp barrier

			for (int i = start; i < end; ++i)
			{
				arrcpy(tmp2, &T[i][1], n);
				if (i == end - 1)
				{
					for (int j = 1; j <= n; ++j)
					{
						double previous_value = T[i][j];
						T[i][j] = (T[i][j - 1] + T[i][j + 1] + row_after_chunk[j - 1] + tmp1[j - 1]) / 4.0;
						error = fmax(error, fabs(previous_value - T[i][j]));
					}
				}
				else
				{
					for (int j = 1; j <= n; ++j)
					{
						double previous_value = T[i][j];
						T[i][j] = (T[i][j - 1] + T[i][j + 1] + T[i + 1][j - 1] + tmp1[j - 1]) / 4.0;
						error = fmax(error, fabs(previous_value - T[i][j]));
					}
				}
				arrcpy(tmp1, tmp2, n);
			}
		}
		if (error < tol)
			break;
	}

	clock_gettime(CLOCK_MONOTONIC, &endtime);

	printf("Time: %f\n", timediff(&starttime, &endtime));
	printf("Number of iterations: %d\n", k);
	printf("Temperature of element T(1,1): %.17f\n", T[1][1]);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Usage: %s [size] [maxiter] [tolerance] \n", argv[0]);
		exit(1);
	}

	int size = atoi(argv[1]);
	int maxiter = atoi(argv[2]);
	double tol = atof(argv[3]);

	printf("Size %d, max iter %d and tolerance %f.\n", size, maxiter, tol);
	laplsolv(size, maxiter, tol);
	return 0;
}
