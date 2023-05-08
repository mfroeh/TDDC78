

all:main.c  physics.c
	mpicc -std=c99 -g -o $@ $^ -lrt -lm

clean:
	-rm *.o main
