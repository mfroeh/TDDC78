all: ours given

ours: main_ours.cc physics.cc
	icc -std=c++17 -g -o $@ $^ -lrt -lm

given: main_given.cc physics.cc
	icc -std=c++17 -g -o $@ $^ -lrt -lm

clean:
	-rm *.o sequential
