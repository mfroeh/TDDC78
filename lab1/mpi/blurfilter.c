#include <stdio.h>
#include <stdlib.h>
#include "blurfilter.h"
#include "../ppmio.h"
#include <pthread.h>

pthread_barrier_t barrier;

typedef struct
{
	int xsize, ysize;
	int radius;
	pixel *src, *dst;
	double const *weights;
	int rank, num_threads;
} thread_args;

pixel *pix(pixel *image, const int xx, const int yy, const int xsize)
{
	int off = xsize * yy + xx;
	return (image + off);
}

void compute_row(int y, thread_args *args)
{
	for (int x = 0; x < args->xsize; ++x)
	{
		double r = 0, g = 0, b = 0, n = 0;
		for (int wi = -args->radius; wi <= args->radius; wi++)
		{
			double wc = args->weights[abs(wi)];
			int x2 = x + wi;
			if (x2 >= 0 && x2 < args->xsize)
			{
				r += wc * pix(args->src, x2, y, args->xsize)->r;
				g += wc * pix(args->src, x2, y, args->xsize)->g;
				b += wc * pix(args->src, x2, y, args->xsize)->b;
				n += wc;
			}
		}

		pix(args->dst, x, y, args->xsize)->r = r / n;
		pix(args->dst, x, y, args->xsize)->g = g / n;
		pix(args->dst, x, y, args->xsize)->b = b / n;
	}
}

void compute_col(int x, thread_args *args)
{
	for (int y = 0; y < args->ysize; ++y)
	{

		double r = 0, g = 0, b = 0, n = 0;
		for (int wi = -args->radius; wi <= args->radius; wi++)
		{
			double wc = args->weights[abs(wi)];
			int y2 = y + wi;
			if (y2 >= 0 && y2 < args->ysize)
			{
				r += wc * pix(args->dst, x, y2, args->xsize)->r;
				g += wc * pix(args->dst, x, y2, args->xsize)->g;
				b += wc * pix(args->dst, x, y2, args->xsize)->b;
				n += wc;
			}
		}

		pix(args->src, x, y, args->xsize)->r = r / n;
		pix(args->src, x, y, args->xsize)->g = g / n;
		pix(args->src, x, y, args->xsize)->b = b / n;
	}
}

void *work(void *arg)
{
	thread_args args = *(thread_args *)arg;

	int thread_rows = args.ysize / args.num_threads;
	int thread_cols = args.xsize / args.num_threads;

	int start_row = args.rank * thread_rows;
	int start_col = args.rank * thread_cols;

	int end_row = start_row + thread_rows;
	int end_col = start_col + thread_cols;

	// Last thread does the remaining work
	if (args.rank == args.num_threads - 1)
	{
		end_row += args.ysize % args.num_threads;
		end_col += args.xsize % args.num_threads;
	}

	// Compute the weighted row-wise averages for pixels of the assigned rows
	for (int y = start_row; y < end_row; ++y)
		compute_row(y, &args);

	// Wait for all the row averages to be computed
	pthread_barrier_wait(&barrier);

	// Compute the weighted column-wise averages for pixels of the assigned columns
	for (int x = start_col; x < end_col; ++x)
		compute_col(x, &args);
}

void blurfilter(const int xsize, const int ysize, pixel *src, const int radius, const double *w, const int thread_count)
{
	pthread_barrier_init(&barrier, NULL, thread_count);

	pixel *dst = (pixel *)malloc(sizeof(pixel) * MAX_PIXELS);

	pthread_t *threads = malloc(sizeof(pthread_t) * thread_count);
	for (int t = 0; t < thread_count; ++t)
	{
		thread_args *args = malloc(sizeof(thread_args));
		args->xsize = xsize;
		args->ysize = ysize;
		args->radius = radius;
		args->weights = w;
		args->src = src;
		args->dst = dst;
		args->rank = t;
		args->num_threads = thread_count;
		pthread_create(&threads[t], NULL, work, args);
	}

	for (int t = 0; t < thread_count; ++t)
		pthread_join(threads[t], NULL);

	pthread_barrier_destroy(&barrier);

	free(dst);
}