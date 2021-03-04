
all:
	g++ serial.cpp -lpng  -o serial.o
	g++ openmp.cpp -lpng  -fopenmp -o omp.o
	mpiCC mpi.cpp -lpng -lm -o mpi.o 

run:
	
	@echo "======================== Serial  =========== "
	./serial.o
	@echo "======================== OpenMP  =========== "
	./omp.o
	@echo "========================  MPI (with 4 Nodes)+ OpenMP      =========== "
	mpirun -np 4 ./mpi.o
	@echo "========================  MPI (with 8 Nodes) + OpenMP     =========== "
	mpirun -np 8 ./mpi.o
	@echo "========================  MPI (with 16 Nodes)+ OpenMP      =========== "
	mpirun -np 16 ./mpi.o
clean:
	rm *.o 
