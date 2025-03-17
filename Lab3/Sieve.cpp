#include <mpi.h>
#include <iostream>
#include <cmath>
#include <vector>

bool is_prime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0) return false;
    return true;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int MAX = 1000;
    std::vector<int> primes;

    if (rank == 0) {  // Master process
        int current = 2;
        int received;
        MPI_Status status;

        while (current <= MAX) {
            MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            
            if (received == 0) {  // Worker request
                MPI_Send(&current, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                current++;
            }
            else if (received > 0) {  // Prime result
                primes.push_back(received);
            }
        }

        // Send termination signal
        int terminate = -1;
        for (int i = 1; i < size; i++) {
            MPI_Send(&terminate, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        std::cout << "Primes found: ";
        for (int p : primes) std::cout << p << " ";
        std::cout << std::endl;
    }
    else {  // Worker processes
        int number;
        MPI_Status status;
        
        while (true) {
            // Request work
            int request = 0;
            MPI_Send(&request, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            
            MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            
            if (number == -1) break;  // Termination
            
            int result = is_prime(number) ? number : -number;
            MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}