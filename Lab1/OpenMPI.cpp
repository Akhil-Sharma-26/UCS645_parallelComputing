#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
  // Initialize the MPI environment. The two arguments to MPI Init are not
  // currently used by MPI implementations, but are there in case future
  // implementations might need the arguments.
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
         processor_name, world_rank, world_size);

  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}


// Have to load the openMPI modules to the kernel first
//  source /etc/profile.d/modules.sh
//  module load mpi/openmpi-x86_64


// akhil@aks-fedora:~/dev/GPU Computing$ source /etc/profile.d/modules.sh
// akhil@aks-fedora:~/dev/GPU Computing$ module load mpi/openmpi-x86_64
// akhil@aks-fedora:~/dev/GPU Computing$ mpic++ -o OpenMPI OpenMPI.cpp
// akhil@aks-fedora:~/dev/GPU Computing$ mpirun -np 2 ./OpenMPI

// OR
// Do the following: (but it will only run in one core somehow. idk why)
// ! g++ -I/usr/include/openmpi-x86_64 -L/usr/lib64/openmpi/lib -Wl,-rpath -Wl,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags -lmpi