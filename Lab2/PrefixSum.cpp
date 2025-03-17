#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    const int local_size = 2;
    std::vector<int> data(local_size, rank + 1);
    std::vector<int> prefix(local_size);

    prefix[0] = data[0];
    for (int i = 1; i < local_size; ++i)
        prefix[i] = prefix[i-1] + data[i];

    int local_sum = prefix.back();
    int carry;
    MPI_Scan(&local_sum, &carry, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    carry -= local_sum;

    for (int& val : prefix)
        val += carry;

    std::cout << "Process " << rank << ": ";
    for (int val : prefix) std::cout << val << " ";
    std::cout << std::endl;

    MPI_Finalize();
    return 0;
}