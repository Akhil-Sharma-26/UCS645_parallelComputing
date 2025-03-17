#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int dims[2] = {0, 0};
    MPI_Dims_create(nprocs, 2, dims);
    int periods[2] = {0, 0};
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    const int matrix_size = 70;
    const int block_rows = matrix_size / dims[0];
    const int block_cols = matrix_size / dims[1];
    std::vector<double> block(block_rows * block_cols);

    for (int i = 0; i < block_rows; ++i)
        for (int j = 0; j < block_cols; ++j)
            block[i*block_cols + j] = coords[0] * block_rows + i + (coords[1] * block_cols + j) * matrix_size;

    int transposed_coords[2] = {coords[1], coords[0]};
    int transposed_rank;
    MPI_Cart_rank(cart_comm, transposed_coords, &transposed_rank);

    std::vector<double> recv_block(block_cols * block_rows);
    MPI_Sendrecv(block.data(), block.size(), MPI_DOUBLE, transposed_rank, 0,
                 recv_block.data(), recv_block.size(), MPI_DOUBLE, transposed_rank, 0,
                 cart_comm, MPI_STATUS_IGNORE);

    std::vector<double> transposed(block_rows * block_cols);
    for (int i = 0; i < block_cols; ++i)
        for (int j = 0; j < block_rows; ++j)
            transposed[j*block_cols + i] = recv_block[i*block_rows + j];

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}