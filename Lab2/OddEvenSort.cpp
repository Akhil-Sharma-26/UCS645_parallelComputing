#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    const int total_size = 70;
    const int local_size = total_size / nprocs;
    std::vector<int> local_data(local_size);

    std::vector<int> all_data;
    if (rank == 0) {
        all_data.resize(total_size);
        for (int i = 0; i < total_size; ++i)
            all_data[i] = rand() % 1000;
    }

    MPI_Scatter(all_data.data(), local_size, MPI_INT,
                local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    std::sort(local_data.begin(), local_data.end());

    for (int phase = 0; phase < nprocs; ++phase) {
        if (phase % 2 == 0) {
            if (rank % 2 == 0 && rank + 1 < nprocs) {
                std::vector<int> recv(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, rank + 1, 0,
                             recv.data(), local_size, MPI_INT, rank + 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<int> merged(2 * local_size);
                std::merge(local_data.begin(), local_data.end(), recv.begin(), recv.end(), merged.begin());
                std::copy(merged.begin(), merged.begin() + local_size, local_data.begin());
            } else if (rank % 2 == 1 && rank - 1 >= 0) {
                std::vector<int> recv(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, rank - 1, 0,
                             recv.data(), local_size, MPI_INT, rank - 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<int> merged(2 * local_size);
                std::merge(recv.begin(), recv.end(), local_data.begin(), local_data.end(), merged.begin());
                std::copy(merged.begin() + local_size, merged.end(), local_data.begin());
            }
        } else {
            if (rank % 2 == 1 && rank + 1 < nprocs) {
                std::vector<int> recv(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, rank + 1, 0,
                             recv.data(), local_size, MPI_INT, rank + 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<int> merged(2 * local_size);
                std::merge(local_data.begin(), local_data.end(), recv.begin(), recv.end(), merged.begin());
                std::copy(merged.begin(), merged.begin() + local_size, local_data.begin());
            } else if (rank % 2 == 0 && rank - 1 >= 0) {
                std::vector<int> recv(local_size);
                MPI_Sendrecv(local_data.data(), local_size, MPI_INT, rank - 1, 0,
                             recv.data(), local_size, MPI_INT, rank - 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<int> merged(2 * local_size);
                std::merge(recv.begin(), recv.end(), local_data.begin(), local_data.end(), merged.begin());
                std::copy(merged.begin() + local_size, merged.end(), local_data.begin());
            }
        }
    }

    MPI_Gather(local_data.data(), local_size, MPI_INT,
               all_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        bool sorted = true;
        for (int i = 0; i < total_size - 1; ++i)
            if (all_data[i] > all_data[i + 1]) sorted = false;
        std::cout << "Sorted: " << (sorted ? "Yes" : "No") << std::endl;
    }

    MPI_Finalize();
    return 0;
}