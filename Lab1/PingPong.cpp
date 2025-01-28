#include <mpi.h>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get process rank (id) and total number of processes
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check if we have exactly 2 processes
    if (size != 2) {
        if (rank == 0) {
            std::cout << "This program requires exactly 2 processes!" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    // Define constants
    const int PING_PONG_LIMIT = 5;  // Number of times to ping pong
    const int TAG = 0;               // Message tag
    
    // Variables to track game
    int count = 0;      // Current iteration
    int value = 0;      // Value being passed back and forth
    
    // Main ping-pong loop
    while (count < PING_PONG_LIMIT) {
        // Process 0 starts with even counts
        // Process 1 starts with odd counts
        if (rank == count % 2) {
            // Increment value before sending
            value = count;
            
            // Send to other process
            // If we're process 0, send to 1; if we're process 1, send to 0
            MPI_Send(&value,                    // Buffer to send
                    1,                          // Number of elements
                    MPI_INT,                    // Data type
                    (rank + 1) % 2,            // Destination process
                    TAG,                        // Message tag
                    MPI_COMM_WORLD);           // Communicator
            
            std::cout << "Process " << rank << " sent value " << value 
                      << " to Process " << ((rank + 1) % 2) << std::endl;
            
        } else {
            // Receive from other process
            MPI_Status status;
            MPI_Recv(&value,                    // Buffer to receive into
                    1,                          // Number of elements
                    MPI_INT,                    // Data type
                    (rank + 1) % 2,            // Source process
                    TAG,                        // Message tag
                    MPI_COMM_WORLD,            // Communicator
                    &status);                   // Status object
            
            std::cout << "Process " << rank << " received value " << value 
                      << " from Process " << ((rank + 1) % 2) << std::endl;
        }
        
        // Both processes increment the counter
        count++;
        
        // Optional: Add a small delay to make output more readable
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Clean up MPI environment
    MPI_Finalize();
    return 0;
}