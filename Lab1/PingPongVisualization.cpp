#include <mpi.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>  // For strlen
#include <unistd.h> // For usleep

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

const int WIDTH = 800, HEIGHT = 600;

void drawProcessInfo(int rank, int count, int value, int partner) {
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH/2, HEIGHT/2, 0, -1, 1);  // Double the size
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Background color based on role
    if (rank == count % 2) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // Sending
    } else {
        glClearColor(0.3f, 0.2f, 0.2f, 1.0f);  // Receiving
    }
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Prepare text
    std::stringstream ss;
    ss << "Process: " << rank << "\n"
       << "Current Count: " << count << "\n"
       << "Current Value: " << value << "\n"
       << "Partner: " << partner;
    
    // Draw text using stb_easy_font
    const char* text = ss.str().c_str();
    float x = 20.0f, y = 30.0f;
    
    // Create vertex buffer with explicit white color
    static unsigned char buffer[1024 * 1024];
    unsigned char white[] = {255, 255, 255, 255};
    int num_quads = stb_easy_font_print(x, y, (char*)text, white, buffer, sizeof(buffer));
    
    // Draw the text quads
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, 4 * num_quads);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Draw communication arrow
    glColor3f(1.0f, 1.0f, 0.0f);
    if (rank == count % 2) {
        glBegin(GL_TRIANGLES);
        glVertex2f(600, 150);
        glVertex2f(700, 150);
        glVertex2f(650, 200);
        glEnd();
    } else {
        glBegin(GL_TRIANGLES);
        glVertex2f(200, 150);
        glVertex2f(100, 150);
        glVertex2f(150, 200);
        glEnd();
    }

    glDisable(GL_BLEND);
}



int main(int argc, char** argv) {
    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get process rank and size
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check for exactly 2 processes
    if (size != 2) {
        if (rank == 0) {
            std::cout << "This program requires exactly 2 processes!" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window and OpenGL context
    GLFWwindow* window;
    if (rank == 0) {
        window = glfwCreateWindow(WIDTH, HEIGHT, "MPI Process 0", NULL, NULL);
    } else {
        window = glfwCreateWindow(WIDTH, HEIGHT, "MPI Process 1", NULL, NULL);
    }

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // Add these OpenGL configurations
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Main loop variables
    int count = 0;
    int value = 0;
    const int PING_PONG_LIMIT = 5;
    const int TAG = 0;
    bool running = true;
    bool done = false;  // New flag to track completion

    // Main rendering/communication loop
    while (running && !done) {
        glfwPollEvents();

        // Only handle communication for the first 5 steps
        if (count < PING_PONG_LIMIT) {
            if (rank == count % 2) {
                value = count;
                MPI_Send(&value, 1, MPI_INT, (rank + 1) % 2, TAG, MPI_COMM_WORLD);
            } else {
                MPI_Status status;
                MPI_Recv(&value, 1, MPI_INT, (rank + 1) % 2, TAG, MPI_COMM_WORLD, &status);
            }
            count++;
        } else {
            done = true;  // Communication complete
        }

        // Render regardless of communication state
        glClear(GL_COLOR_BUFFER_BIT);
        drawProcessInfo(rank, count < PING_PONG_LIMIT ? count : PING_PONG_LIMIT, value, (rank + 1) % 2);
        glfwSwapBuffers(window);

        // Check if window should close
        if (glfwWindowShouldClose(window)) {
            running = false;
        }

        // Add small delay to make window responsive
        usleep(10000);  // 10ms delay
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Keep window open after communication finishes
    while (running) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        drawProcessInfo(rank, PING_PONG_LIMIT, value, (rank + 1) % 2);
        glfwSwapBuffers(window);
        
        if (glfwWindowShouldClose(window)) {
            running = false;
        }
        usleep(10000);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    MPI_Finalize();
    return 0;
}

