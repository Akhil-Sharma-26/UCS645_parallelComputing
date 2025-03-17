#include <mpi.h>
#include <iostream>
#include <cstdlib>

void fill_matrix(double* matrix, int size) {
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            matrix[i*size + j] = rand() % 100;
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
    double *A = nullptr, *C = nullptr;
    
    // Allocate B on ALL processes
    double *B = new double[matrix_size * matrix_size];
    double *local_A = new double[rows_per_proc * matrix_size];
    double *local_C = new double[rows_per_proc * matrix_size]();

    if (rank == 0) {
        A = new double[matrix_size * matrix_size];
        C = new double[matrix_size * matrix_size]();
        fill_matrix(A, matrix_size);
        fill_matrix(B, matrix_size);  // Fill B only on root
    }

    // Root broadcasts B to all processes
    MPI_Bcast(B, matrix_size * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Scatter A from root to all processes
    MPI_Scatter(A, rows_per_proc * matrix_size, MPI_DOUBLE, 
               local_A, rows_per_proc * matrix_size, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);

    // Local computation
    multiply(local_A, B, local_C, rows_per_proc, matrix_size);

    // Gather results to root
    MPI_Gather(local_C, rows_per_proc * matrix_size, MPI_DOUBLE,
               C, rows_per_proc * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        delete[] A;
        delete[] C;
    }

    delete[] B;          // All processes delete B
    delete[] local_A;
    delete[] local_C;
    MPI_Finalize();
    return 0;
}