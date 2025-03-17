#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    const int total_size = 70;
    const int local_size = total_size / nprocs;
    std::vector<double> a(local_size), b(local_size);

    std::vector<double> global_a, global_b;
    if (rank == 0) {
        global_a.resize(total_size);
        global_b.resize(total_size);
        for (int i = 0; i < total_size; ++i) {
            global_a[i] = rand() % 10;
            global_b[i] = rand() % 10;
        }
    }

    MPI_Scatter(global_a.data(), local_size, MPI_DOUBLE, a.data(), local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(global_b.data(), local_size, MPI_DOUBLE, b.data(), local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local_dot = 0.0;
    for (int i = 0; i < local_size; ++i)
        local_dot += a[i] * b[i];

    double global_dot;
    MPI_Reduce(&local_dot, &global_dot, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        std::cout << "Dot Product: " << global_dot << std::endl;

    MPI_Finalize();
    return 0;
}