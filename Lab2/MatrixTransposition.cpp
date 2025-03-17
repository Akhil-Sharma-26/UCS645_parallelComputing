#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // Ensure number of processes is a perfect square
    int sqrt_nprocs = static_cast<int>(std::sqrt(nprocs));
    if (sqrt_nprocs * sqrt_nprocs != nprocs) {
        if (rank == 0) {
            std::cerr << "Number of processes must be a perfect square!\n";
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    int dims[2] = {sqrt_nprocs, sqrt_nprocs}; // Force square grid
    int periods[2] = {0, 0};
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    const int matrix_size = 70;
    const int block_rows = matrix_size / dims[0];
    const int block_cols = matrix_size / dims[1];
    std::vector<double> block(block_rows * block_cols);

    // Initialize local block
    for (int i = 0; i < block_rows; ++i)
        for (int j = 0; j < block_cols; ++j)
            block[i*block_cols + j] = coords[0] * block_rows + i + (coords[1] * block_cols + j) * matrix_size;

    // Get transposed rank in original communicator
    int transposed_coords[2] = {coords[1], coords[0]};
    int transposed_rank;
    MPI_Cart_rank(cart_comm, transposed_coords, &transposed_rank);

    // Exchange data with transposed rank
    std::vector<double> recv_block(block.size());
    MPI_Sendrecv(block.data(), block.size(), MPI_DOUBLE, transposed_rank, 0,
                 recv_block.data(), recv_block.size(), MPI_DOUBLE, transposed_rank, 0,
                 cart_comm, MPI_STATUS_IGNORE);

    // Transpose local block
    std::vector<double> transposed(block_rows * block_cols);
    for (int i = 0; i < block_cols; ++i)
        for (int j = 0; j < block_rows; ++j)
            transposed[j*block_cols + i] = recv_block[i*block_rows + j];

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}