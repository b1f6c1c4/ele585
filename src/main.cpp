#include "main.h"

#include <iostream>

int main(int argc, char *argv[])
{
    std::cerr << "This is " << PACKAGE << " version " << VERSION << std::endl;
    std::cerr << "Please report bug to " << PACKAGE_BUGREPORT << std::endl;

    int number_nodes, myid;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_nodes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    std::cerr << "MPI_Init: I'm " << myid << " / " << number_nodes << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    std::cerr << "MPI_Finalize: I'm " << myid << " / " << number_nodes << std::endl;
    MPI_Finalize();
    return 0;
}
