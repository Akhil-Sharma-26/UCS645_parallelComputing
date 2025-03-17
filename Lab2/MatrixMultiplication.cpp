#include <mpi.h>
#include <iostream>
#include <omp.h>
#include <cstdlib>

void fill_matrix(double* matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            matrix[i*cols + j] = rand() % 100;
}

void multiply(double* A, double* B, double* C, int rows, int size) {
    for (int i = 0; i < rows; ++i)
        for (int k = 0; k < size; ++k)
            for (int j = 0; j < size; ++j)
                C[i*size + j] += A[i*size + k] * B[k*size + j];
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    const int matrix_size = 70;
    const int rows_per_proc = matrix_size / num_procs;
    double *A = nullptr, *B = nullptr, *C = nullptr;
    double *local_A = new double[rows_per_proc * matrix_size];
    double *local_C = new double[rows_per_proc * matrix_size]();

    if (rank == 0) {
        A = new double[matrix_size * matrix_size];
        B = new double[matrix_size * matrix_size];
        C = new double[matrix_size * matrix_size]();
        fill_matrix(A, matrix_size, matrix_size);
        fill_matrix(B, matrix_size, matrix_size);
    }

    MPI_Scatter(A, rows_per_proc * matrix_size, MPI_DOUBLE, local_A,
                rows_per_proc * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, matrix_size * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = omp_get_wtime();
    multiply(local_A, B, local_C, rows_per_proc, matrix_size);
    double end = omp_get_wtime();
    double runtime = end - start;

    MPI_Gather(local_C, rows_per_proc * matrix_size, MPI_DOUBLE,
               C, rows_per_proc * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "Parallel time: " << runtime << " seconds\n";
        delete[] A;
        delete[] B;
        delete[] C;
    }

    delete[] local_A;
    delete[] local_C;
    MPI_Finalize();
    return 0;
}