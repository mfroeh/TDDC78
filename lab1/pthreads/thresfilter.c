#include "thresfilter.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned int uint;

typedef struct
{
	pixel *src;
	int N, begin, end;
	uint *sum;
} thread_args;

pthread_mutex_t sum_lock;
pthread_barrier_t barrier;

void *work(void *arg)
{
	thread_args args = *(thread_args *)arg;

	// Sum over all my pixels
	uint local_sum = 0;
	for (int i = args.begin; i < args.end; ++i)
		local_sum += args.src[i].r + args.src[i].g + args.src[i].b;

	pthread_mutex_lock(&sum_lock);
	*args.sum += local_sum;
	pthread_mutex_unlock(&sum_lock);

	pthread_barrier_wait(&barrier);
	uint avg = *args.sum / args.N;

	// Set values for all my pixels
	for (int i = args.begin; i < args.end; ++i)
	{
		uint psum = args.src[i].r + args.src[i].g + args.src[i].b;
		if (avg > psum)
			args.src[i].r = args.src[i].g = args.src[i].b = 0;
		else
			args.src[i].r = args.src[i].g = args.src[i].b = 255;
	}
	free(arg);
}

void thresfilter(const int xsize, const int ysize, pixel *src, int thread_count)
{
	pthread_barrier_init(&barrier, NULL, thread_count);
	pthread_mutex_init(&sum_lock, NULL);

	int N = xsize * ysize;
	int chunksize = N / thread_count;
	unsigned int sum = 0;

	pthread_t *threads = malloc(thread_count * sizeof(thread_args));
	for (int i = 0; i < thread_count; ++i)
	{
		thread_args *args = malloc(sizeof(thread_args));
		args->src = src;
		args->begin = i * chunksize;
		args->end = args->begin + chunksize;
		if (i == thread_count - 1)
			args->end += N % thread_count;
		args->sum = &sum;
		args->N = N;
		pthread_create(threads + i, NULL, work, args);
	}

	for (int i = 0; i < thread_count; ++i)
		pthread_join(threads[i], NULL);
	free(threads);

	pthread_barrier_destroy(&barrier);
	pthread_mutex_destroy(&sum_lock);
}
