#include<mpi.h>
#include<iostream>

int main(){
    // initialize MPI:
    MPI_Init(NULL,NULL);

    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int number;
    if (world_rank == 0) {
        number = 500;
        MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        std::cout<<"something recieved from process 0:"<<number<<std::endl;
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}

