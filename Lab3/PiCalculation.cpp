#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    static long num_steps = 100000;
    double step, pi, sum = 0.0;
    double x, local_sum = 0.0;

    if (rank == 0) {
        step = 1.0 / (double)num_steps;
    }

    // Broadcast step value to all processes
    MPI_Bcast(&step, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Divide work among processes
    long chunk = num_steps / size;
    long start = rank * chunk;
    long end = (rank == size-1) ? num_steps : start + chunk;

    for (long i = start; i < end; i++) {
        x = (i + 0.5) * step;
        local_sum += 4.0 / (1.0 + x * x);
    }

    // Reduce all partial sums to root
    MPI_Reduce(&local_sum, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi = step * sum;
        std::cout << "Calculated π: " << pi << std::endl;
    }

    MPI_Finalize();
    return 0;
}