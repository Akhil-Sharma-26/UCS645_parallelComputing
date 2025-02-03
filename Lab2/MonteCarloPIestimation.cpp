#include <mpi.h>
#include <iostream>
#include <random>
#include <cmath>
#include <stdexcept>
#include <chrono>
int main(int argc, char** argv) {
    auto start = std::chrono::steady_clock::now();

    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Validate input
    if (rank == 0 && argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <total_points>" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Broadcast total points to all processes
    long total_points = 0;
    if (rank == 0) total_points = std::stol(argv[1]);
    MPI_Bcast(&total_points, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    // Divide work with load balancing
    long local_points = total_points / num_procs;
    if (rank < total_points % num_procs) local_points++;

    // Initialize random number generator with unique seed per process
    std::random_device rd;
    std::mt19937_64 gen(rd() ^ (rank + 1));  // Combine device entropy with rank
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Local computation
    long local_hits = 0;
    for (long i = 0; i < local_points; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        if (x*x + y*y <= 1.0) local_hits++;
    }

    // Reduce all hits to root process
    long global_hits;
    MPI_Reduce(&local_hits, &global_hits, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // Final calculation and output
    if (rank == 0) {
        double pi_estimate = 4.0 * static_cast<double>(global_hits) / total_points;
        std::cout.precision(10);
        std::cout << "Estimated π: " << pi_estimate 
                  << "\nTotal points: " << total_points
                  << "\nProcesses used: " << num_procs << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    MPI_Finalize();
    auto diff = end - start;

    std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;
    return 0;
}