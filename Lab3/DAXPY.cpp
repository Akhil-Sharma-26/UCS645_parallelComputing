#include <mpi.h>
#include <iostream>
#include <cmath>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = pow(2, 16);
    double a = 2.0;
    double *X = nullptr, *Y = nullptr;
    double start, end;

    int local_size = N / size;
    double *local_X = new double[local_size];
    double *local_Y = new double[local_size];

    // Initialize vectors on root
    if (rank == 0) {
        X = new double[N];
        Y = new double[N];
        for (int i = 0; i < N; i++) {
            X[i] = i;
            Y[i] = N - i;
        }
        start = MPI_Wtime();
    }

    // Scatter vectors
    MPI_Scatter(X, local_size, MPI_DOUBLE, local_X, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(Y, local_size, MPI_DOUBLE, local_Y, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Compute local DAXPY
    for (int i = 0; i < local_size; i++) {
        local_X[i] = a * local_X[i] + local_Y[i];
    }

    // Gather results
    MPI_Gather(local_X, local_size, MPI_DOUBLE, X, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end = MPI_Wtime();
        std::cout << "Parallel Time: " << end - start << " seconds\n";
        delete[] X;
        delete[] Y;
    }

    delete[] local_X;
    delete[] local_Y;
    MPI_Finalize();
    return 0;
}