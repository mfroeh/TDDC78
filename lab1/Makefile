CC = gcc
CFLAGS = -g
LFLAGS = -lpthread -lrt -lm -g

all: mpi pthreads
mpi: blurc_mpi thresc_mpi
pthreads: blurc_pthreads thresc_pthreads

clean:
	-$(RM) **/*.o  blurc_* thresc_*

blurc_pthreads: ppmio.o gaussw.o pthreads/blurfilter.o pthreads/blurmain.o
	$(CC) -o $@ ppmio.o gaussw.o pthreads/blurfilter.o pthreads/blurmain.o $(LFLAGS)

thresc_pthreads: pthreads/thresmain.o ppmio.o pthreads/thresfilter.o
	$(CC) -o $@ pthreads/thresmain.o ppmio.o pthreads/thresfilter.o $(LFLAGS)

blurc_mpi: ppmio.o gaussw.o mpi/blurfilter.o mpi/blurmain.o
	mpicc -o $@ ppmio.o gaussw.o mpi/blurfilter.o mpi/blurfilter.o -g -lrt -lm

thresc_mpi: mpi/thresmain.o ppmio.o mpi/thresfilter.o
	mpicc -o $@ mpi/thresmain.o ppmio.o mpi/thresfilter.o -g -lrt -lm

arc:
	tar cf - *.c *.cc *.h Makefile data/* | gzip - > filters.tar.gz
