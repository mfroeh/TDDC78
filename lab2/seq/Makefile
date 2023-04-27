laplsolv-f: laplsolv.f90
	ifort -fopenmp $^ -o $@

laplsolv: laplsolv.c
	icc -std=c11 -fopenmp $^ -o $@
	
clean:
	-$(RM) laplsolv laplsolv-f