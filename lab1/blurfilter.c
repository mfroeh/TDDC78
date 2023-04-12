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
} task_args;


void *compute_row(void* buf) {
	task_args args = *(task_args*)(buf);
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

	free(buf);
}

void *compute_col(void* buf) {
	task_args args = *(task_args*)(buf);
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

	free(buf);
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

	pthread_t *threads = malloc(sizeof(pthread_t) * ysize);
	for (int y=0; y<ysize; y++)
	{
		task_args *args = malloc(sizeof(task_args));
		args->global = global;
		args->index = y;
		pthread_create(&threads[y], 0, compute_row, args);
	}

	for (int y=0; y < ysize; ++y) {
		pthread_join(threads[y], NULL);
	}
	free(threads);

	threads = malloc(sizeof(pthread_t) * xsize);

	global->src = dst;
	global->dst = src;
	for (int x=0; x < xsize; ++x) {
		task_args *args = malloc(sizeof(task_args));
		args->global = global;
		args->index = x;
		pthread_create(&threads[x], 0, compute_col, args);
	}

	for (int x=0; x < xsize; ++x) {
		pthread_join(threads[x], NULL);
	}
	free(threads);

	free(dst);
}
