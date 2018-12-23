// Robot Simulation Program: by Anway De, Rob Berger, Linh Tran. December 2018.

#include <iostream>
#include "robot.h"

using namespace std;

int main(int argc, char** argv)
{
	int my_rank;
	int nprocs;
	MPI_Status status;


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	if (my_rank == 0) {
		Server server;
		server.run();
	}
	else {
		Robot robot(0);
		robot.start();
	}

	MPI_Finalize();

	return 0;
}
