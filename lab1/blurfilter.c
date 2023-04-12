/*
File: blurfilter.c
Implementation of blurfilter function.
*/

#include <stdio.h>
#include <stdlib.h>
#include "blurfilter.h"
#include "ppmio.h"
#include <pthread.h>
#include <stdlib.h>

#define PTHREAD_COUNT 64 
pthread_barrier_t barrier;

pixel* pix(pixel* image, const int xx, const int yy, const int xsize)
{
	int off = xsize*yy + xx;
	return (image + off);
}

typedef struct {
	double const *w;
	int xsize;
	int ysize;
	int radius;
	pixel* src;
	pixel* dst;
} global_args;

typedef struct {
	int index;
	global_args *global;
} dim_args;

typedef struct {
	int thread_index;
	dim_args dim;
} task_args;

void *compute_row(void* buf) {
	dim_args args = *(dim_args*)(buf);
	global_args *global = args.global;
	int y = args.index;

	for (int x=0; x < global->xsize; ++x) {
		double r = 0,g = 0 ,b = 0,n = 0;

		for (int wi=-global->radius; wi <= global->radius; wi++)
		{
			double wc = global->w[abs(wi)];
			int x2 = x + wi;
			if (x2 >= 0 && x2 < global->xsize)
			{
				r += wc * pix(global->src, x2, y, global->xsize)->r;
				g += wc * pix(global->src, x2, y, global->xsize)->g;
				b += wc * pix(global->src, x2, y, global->xsize)->b;
				n += wc;
			}
		}
		pix(global->dst,x,y, global->xsize)->r = r/n;
		pix(global->dst,x,y, global->xsize)->g = g/n;
		pix(global->dst,x,y, global->xsize)->b = b/n;
	}

}

void* compute_rows(void* t_args) {
	
	task_args args = *(task_args*)t_args;	

	int rows_per_thread = args.dim.global->ysize / PTHREAD_COUNT;
	int starting_row = args.thread_index*rows_per_thread;
	int last_row = starting_row + rows_per_thread;
	if(args.thread_index == PTHREAD_COUNT-1) {
		int remaining_rows = args.dim.global->ysize % PTHREAD_COUNT;
		last_row += remaining_rows;
	}
	
	for (int y=starting_row; y < last_row; ++y) {
		args.dim.index = y;
		compute_row(&args.dim);
	}
	
	pthread_barrier_wait(&barrier);
	args.dim.global->src = dst;
	args.dim.global->dst = src;
	compute_cols(
}

void *compute_col(void* buf) {
	dim_args args = *(dim_args*)(buf);
	global_args *global = args.global;
	int x = args.index;

	for (int y=0; y < global->ysize; ++y) {
		double r = 0,g = 0 ,b = 0,n = 0;
		for (int wi=-global->radius; wi <= global->radius; wi++)
		{
			double wc = global->w[abs(wi)];

			int y2 = y + wi;
			if (y2 >= 0 && y2 < global->ysize)
			{
				r += wc * pix(global->src, x, y2, global->xsize)->r;
				g += wc * pix(global->src, x, y2, global->xsize)->g;
				b += wc * pix(global->src, x, y2, global->xsize)->b;
				n += wc;
			}
		}
		pix(global->dst,x,y, global->xsize)->r = r/n;
		pix(global->dst,x,y, global->xsize)->g = g/n;
		pix(global->dst,x,y, global->xsize)->b = b/n;
	}

}

void* compute_cols(void* t_args) {
	
	task_args args = *(task_args*)t_args;	

	int cols_per_thread = args.dim.global->xsize / PTHREAD_COUNT;
	int starting_col = args.thread_index*cols_per_thread;
	int last_col = starting_col + cols_per_thread;
	if(args.thread_index == PTHREAD_COUNT-1) {
		int remaining_cols = args.dim.global->xsize % PTHREAD_COUNT;
		last_col += remaining_cols;
	}
	
	for (int x=starting_col; x < last_col; ++x) {
		args.dim.index = x;
		compute_col(&args.dim);
	}
	
	free(t_args);
}

void blurfilter(const int xsize, const int ysize, pixel* src, const int radius, const double *w)
{
	double r, g, b, n, wc;
	pixel *dst = (pixel*) malloc(sizeof(pixel) * MAX_PIXELS);

	global_args *global = malloc(sizeof(global_args));
	global->xsize = xsize;
	global->ysize = ysize;
	global->src = src;
	global->dst = dst;
	global->radius = radius;
	global->w = w;

	pthread_t threads[PTHREAD_COUNT];

	for (int thread=0; thread<PTHREAD_COUNT; thread++)
	{
		task_args *args = malloc(sizeof(task_args));
		args->dim.global = global;
		args->thread_index = thread;
		pthread_create(&threads[thread], 0, compute_rows, args);
	}

	for (int thread=0; thread < PTHREAD_COUNT; ++thread) {
		pthread_join(threads[thread], NULL);
	}

	global->src = dst;
	global->dst = src;

	for (int thread=0; thread<PTHREAD_COUNT; thread++)
	{
		task_args *args = malloc(sizeof(task_args));
		args->dim.global = global;
		args->thread_index = thread;
		pthread_create(&threads[thread], 0, compute_cols, args);
	}


	for (int thread=0; thread < PTHREAD_COUNT; ++thread) {
		pthread_join(threads[thread], NULL);
	}

	free(dst);
}
