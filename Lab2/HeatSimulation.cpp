#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>

#define NX 100        // Number of columns
#define NY 100        // Total number of rows in the global grid
#define MAX_ITER 1000 // Maximum number of iterations
#define TOL 1e-4      // Convergence tolerance

// Macro to access a 2D grid stored in a 1D vector
#define IDX(i, j) ((i) * NX + (j))

using namespace std;

int main(int argc, char **argv) {
    int rank, size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Assume NY is divisible by the number of processes
    int local_NY = NY / size;  // Number of interior rows per process
    int local_rows = local_NY + 2; // Include ghost rows

    // Allocate memory for current and next iterations
    vector<double> current(local_rows * NX, 0.0);
    vector<double> next(local_rows * NX, 0.0);

    // Apply fixed boundary conditions
    if (rank == 0) {
        for (int j = 0; j < NX; j++) {
            current[IDX(1, j)] = 100.0;
            next[IDX(1, j)] = 100.0;
        }
    }
    if (rank == size - 1) {
        for (int j = 0; j < NX; j++) {
            current[IDX(local_rows - 2, j)] = 100.0;
            next[IDX(local_rows - 2, j)] = 100.0;
        }
    }

    // Main iterative loop
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double local_diff = 0.0;

        // Exchange ghost rows with neighboring processes
        MPI_Status status;
        if (rank > 0) {
            MPI_Sendrecv(&current[IDX(1, 0)], NX, MPI_DOUBLE, rank - 1, 0,
                         &current[IDX(0, 0)], NX, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, &status);
        }
        if (rank < size - 1) {
            MPI_Sendrecv(&current[IDX(local_rows - 2, 0)], NX, MPI_DOUBLE, rank + 1, 0,
                         &current[IDX(local_rows - 1, 0)], NX, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, &status);
        }

        // Update interior points
        for (int i = 1; i < local_rows - 1; i++) {
            for (int j = 1; j < NX - 1; j++) {
                int global_row = rank * local_NY + (i - 1);
                if ((rank == 0 && i == 1) || (rank == size - 1 && i == local_rows - 2)) {
                    next[IDX(i, j)] = current[IDX(i, j)];
                } else {
                    next[IDX(i, j)] = 0.25 * (current[IDX(i - 1, j)] + current[IDX(i + 1, j)] +
                                              current[IDX(i, j - 1)] + current[IDX(i, j + 1)]);
                    local_diff += fabs(next[IDX(i, j)] - current[IDX(i, j)]);
                }
            }
        }

        // Check for convergence
        double global_diff;
        MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        if (global_diff < TOL) {
            if (rank == 0) {
                cout << "Converged after " << iter << " iterations with global diff = " << global_diff << endl;
            }
            break;
        }

        // Swap arrays
        swap(current, next);
    }

    // Gather results
    vector<double> full_grid;
    if (rank == 0) {
        full_grid.resize(NY * NX);
    }
    MPI_Gather(&current[IDX(1, 0)], local_NY * NX, MPI_DOUBLE,
               full_grid.data(), local_NY * NX, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Print results
    if (rank == 0) {
        cout << "Final heat distribution:\n";
        for (int i = 0; i < NY; i++) {
            for (int j = 0; j < NX; j++) {
                cout << full_grid[i * NX + j] << " ";
            }
            cout << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
