// 2 Heat sources in boundary
// #include <mpi.h>
// #include <iostream>
// #include <vector>
// #include <cmath>
// #include <cstdlib>

// // OpenGL and windowing includes
// #include <GL/glew.h>
// #include <GLFW/glfw3.h>

// #define NX 100        // Number of columns (simulation grid width)
// #define NY 100        // Total number of rows (simulation grid height)
// #define MAX_ITER 1000 // Maximum number of iterations for the simulation
// #define TOL 1e-4      // Convergence tolerance for the simulation

// // Macro to access a 2D grid stored in a 1D vector
// #define IDX(i, j) ((i) * NX + (j))

// using namespace std;

// // Vertex shader (using OpenGL 2.1 / GLSL 1.20 syntax)
// const char* vertexShaderSource = R"(
//     #version 120
//     attribute vec2 position;
//     attribute vec2 texcoord;
//     varying vec2 vTexcoord;
//     void main() {
//         gl_Position = vec4(position, 0.0, 1.0);
//         vTexcoord = texcoord;
//     }
// )";

// // Fragment shader that maps the grayscale heat texture to a color gradient.
// const char* fragmentShaderSource = R"(
//     #version 120
//     uniform sampler2D heatTexture;
//     varying vec2 vTexcoord;
//     void main() {
//         // Get the intensity value (normalized between 0.0 and 1.0)
//         float intensity = texture2D(heatTexture, vTexcoord).r;
//         vec3 color;
//         // Use a custom gradient: blue -> cyan -> yellow -> red.
//         if (intensity < 0.33) {
//             // From blue (0.0,0.0,1.0) to cyan (0.0,1.0,1.0)
//             color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), intensity / 0.33);
//         } else if (intensity < 0.66) {
//             // From cyan (0.0,1.0,1.0) to yellow (1.0,1.0,0.0)
//             color = mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), (intensity - 0.33) / 0.33);
//         } else {
//             // From yellow (1.0,1.0,0.0) to red (1.0,0.0,0.0)
//             color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (intensity - 0.66) / 0.34);
//         }
//         gl_FragColor = vec4(color, 1.0);
//     }
// )";

// // Utility function to compile a shader and check for errors.
// GLuint compileShader(GLenum type, const char* source) {
//     GLuint shader = glCreateShader(type);
//     glShaderSource(shader, 1, &source, nullptr);
//     glCompileShader(shader);
//     GLint success;
//     glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//     if (!success) {
//         char info[512];
//         glGetShaderInfoLog(shader, 512, NULL, info);
//         cerr << "Shader compilation error: " << info << endl;
//         exit(EXIT_FAILURE);
//     }
//     return shader;
// }

// // Create and link the shader program.
// GLuint createShaderProgram() {
//     GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
//     GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
//     GLuint program = glCreateProgram();
//     glAttachShader(program, vertexShader);
//     glAttachShader(program, fragmentShader);
//     // Bind attribute locations explicitly.
//     glBindAttribLocation(program, 0, "position");
//     glBindAttribLocation(program, 1, "texcoord");
//     glLinkProgram(program);
//     GLint success;
//     glGetProgramiv(program, GL_LINK_STATUS, &success);
//     if (!success) {
//         char info[512];
//         glGetProgramInfoLog(program, 512, NULL, info);
//         cerr << "Program linking error: " << info << endl;
//         exit(EXIT_FAILURE);
//     }
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);
//     return program;
// }

// // Initialize GLFW and create an OpenGL window.
// GLFWwindow* initOpenGL(int width, int height, const char* title) {
//     if (!glfwInit()) {
//         cerr << "Failed to initialize GLFW" << endl;
//         return nullptr;
//     }
//     // Request an OpenGL 2.1 context (sufficient for GLSL 1.20)
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//     GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
//     if (!window) {
//         cerr << "Failed to create GLFW window" << endl;
//         glfwTerminate();
//         return nullptr;
//     }
//     glfwMakeContextCurrent(window);
//     glewExperimental = GL_TRUE;
//     GLenum err = glewInit();
//     if (GLEW_OK != err) {
//         cerr << "Error: " << glewGetErrorString(err) << endl;
//         return nullptr;
//     }
//     return window;
// }

// // Create an OpenGL texture from the simulation grid data.
// // The grid values (assumed to be in [0,100]) are mapped to an 8-bit intensity.
// GLuint createTextureFromGrid(const vector<double>& grid) {
//     // Prepare a single-channel image (luminance) where each pixel is [0,255].
//     vector<GLubyte> image(NX * NY);
//     for (int i = 0; i < NY; i++) {
//         for (int j = 0; j < NX; j++) {
//             double temp = grid[i * NX + j];
//             if (temp < 0.0) temp = 0.0;
//             if (temp > 100.0) temp = 100.0;
//             GLubyte intensity = static_cast<GLubyte>((temp / 100.0) * 255);
//             image[i * NX + j] = intensity;
//         }
//     }
//     GLuint texID;
//     glGenTextures(1, &texID);
//     glBindTexture(GL_TEXTURE_2D, texID);
//     // Use GL_LUMINANCE to create a one-channel texture.
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.data());
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixelated look
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     return texID;
// }

// int main(int argc, char **argv) {
//     int rank, size;

//     // ------------------------------
//     // MPI-Based Heat Distribution Simulation
//     // ------------------------------
//     MPI_Init(&argc, &argv);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     // Divide the grid rows among processes.
//     int local_NY = NY / size;  // Number of interior rows per process
//     int local_rows = local_NY + 2; // Include two ghost rows

//     // Allocate simulation arrays.
//     vector<double> current(local_rows * NX, 0.0);
//     vector<double> next(local_rows * NX, 0.0);

//     // Apply fixed boundary conditions: top row (rank 0) and bottom row (last rank) set to 100.
//     if (rank == 0) {
//         for (int j = 0; j < NX; j++) {
//             current[IDX(1, j)] = 100.0;
//             next[IDX(1, j)] = 100.0;
//         }
//     }
//     if (rank == size - 1) {
//         for (int j = 0; j < NX; j++) {
//             current[IDX(local_rows - 2, j)] = 100.0;
//             next[IDX(local_rows - 2, j)] = 100.0;
//         }
//     }

//     // Main simulation loop (Jacobi relaxation).
//     for (int iter = 0; iter < MAX_ITER; iter++) {
//         double local_diff = 0.0;
//         MPI_Status status;
//         // Exchange ghost rows with the neighboring processes.
//         if (rank > 0) {
//             MPI_Sendrecv(&current[IDX(1, 0)], NX, MPI_DOUBLE, rank - 1, 0,
//                          &current[IDX(0, 0)], NX, MPI_DOUBLE, rank - 1, 0,
//                          MPI_COMM_WORLD, &status);
//         }
//         if (rank < size - 1) {
//             MPI_Sendrecv(&current[IDX(local_rows - 2, 0)], NX, MPI_DOUBLE, rank + 1, 0,
//                          &current[IDX(local_rows - 1, 0)], NX, MPI_DOUBLE, rank + 1, 0,
//                          MPI_COMM_WORLD, &status);
//         }
//         // Update interior grid points.
//         for (int i = 1; i < local_rows - 1; i++) {
//             for (int j = 1; j < NX - 1; j++) {
//                 // Keep fixed boundaries unchanged.
//                 if ((rank == 0 && i == 1) || (rank == size - 1 && i == local_rows - 2)) {
//                     next[IDX(i, j)] = current[IDX(i, j)];
//                 } else {
//                     next[IDX(i, j)] = 0.25 * (current[IDX(i - 1, j)] +
//                                               current[IDX(i + 1, j)] +
//                                               current[IDX(i, j - 1)] +
//                                               current[IDX(i, j + 1)]);
//                     local_diff += fabs(next[IDX(i, j)] - current[IDX(i, j)]);
//                 }
//             }
//         }
//         // Convergence check (global reduction).
//         double global_diff;
//         MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
//         if (global_diff < TOL) {
//             if (rank == 0)
//                 cout << "Converged after " << iter << " iterations with global diff = " << global_diff << endl;
//             break;
//         }
//         swap(current, next);
//     }

//     // Gather the simulation results at rank 0.
//     vector<double> full_grid;
//     if (rank == 0) {
//         full_grid.resize(NY * NX);
//     }
//     MPI_Gather(&current[IDX(1, 0)], local_NY * NX, MPI_DOUBLE,
//                full_grid.data(), local_NY * NX, MPI_DOUBLE, 0, MPI_COMM_WORLD);

//     // ------------------------------
//     // Enhanced OpenGL Visualization (Only on rank 0)
//     // ------------------------------
//     if (rank == 0) {
//         // Create an OpenGL window.
//         const int winWidth = 800;
//         const int winHeight = 800;
//         GLFWwindow* window = initOpenGL(winWidth, winHeight, "Enhanced Heat Distribution Visualization");
//         if (!window) {
//             MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
//         }

//         // Create and compile the shader program.
//         GLuint shaderProgram = createShaderProgram();

//         // Define vertices and texture coordinates for a full-screen quad.
//         GLfloat vertices[] = {
//             // Positions      // Tex Coords
//             -1.0f, -1.0f,      0.0f, 0.0f,
//              1.0f, -1.0f,      1.0f, 0.0f,
//              1.0f,  1.0f,      1.0f, 1.0f,
//             -1.0f,  1.0f,      0.0f, 1.0f
//         };
//         GLuint indices[] = {
//             0, 1, 2,
//             2, 3, 0
//         };

//         // Set up VAO, VBO, and EBO.
//         GLuint VAO, VBO, EBO;
//         glGenVertexArrays(1, &VAO);
//         glGenBuffers(1, &VBO);
//         glGenBuffers(1, &EBO);

//         glBindVertexArray(VAO);

//         glBindBuffer(GL_ARRAY_BUFFER, VBO);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//         // Position attribute (location = 0)
//         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
//         glEnableVertexAttribArray(0);
//         // Texture coordinate attribute (location = 1)
//         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
//         glEnableVertexAttribArray(1);
//         glBindVertexArray(0);

//         // Create a texture from the simulation grid.
//         GLuint textureID = createTextureFromGrid(full_grid);

//         // Render loop.
//         while (!glfwWindowShouldClose(window)) {
//             glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//             glClear(GL_COLOR_BUFFER_BIT);

//             // Use our shader program.
//             glUseProgram(shaderProgram);
//             glActiveTexture(GL_TEXTURE0);
//             glBindTexture(GL_TEXTURE_2D, textureID);
//             // Set the texture uniform.
//             GLint texLoc = glGetUniformLocation(shaderProgram, "heatTexture");
//             glUniform1i(texLoc, 0);

//             // Draw the quad.
//             glBindVertexArray(VAO);
//             glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//             glBindVertexArray(0);

//             glfwSwapBuffers(window);
//             glfwPollEvents();
//         }

//         // Clean up OpenGL resources.
//         glDeleteTextures(1, &textureID);
//         glDeleteProgram(shaderProgram);
//         glDeleteBuffers(1, &VBO);
//         glDeleteBuffers(1, &EBO);
//         glDeleteVertexArrays(1, &VAO);
//         glfwDestroyWindow(window);
//         glfwTerminate();
//     }

//     MPI_Finalize();
//     return 0;
// }


// Heat source at center
#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

// OpenGL and windowing includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NX 100        // Number of columns (simulation grid width)
#define NY 100        // Number of rows (simulation grid height)
#define MAX_ITER 1000 // Maximum number of iterations for the simulation
#define TOL 1e-4      // Convergence tolerance for the simulation

// Macro to access a 2D grid stored in a 1D vector (row-major)
#define IDX(i, j) ((i) * NX + (j))

using namespace std;

// Define the center heat source region (as percentages of the grid dimensions)
const int center_i_start = NY * 0.4;
const int center_i_end   = NY * 0.6;
const int center_j_start = NX * 0.4;
const int center_j_end   = NX * 0.6;

// ------------------------------------------------------------------
// OpenGL Shader sources (GLSL 1.20 syntax)
// ------------------------------------------------------------------

// Vertex shader: Passes vertex positions and texture coordinates.
const char* vertexShaderSource = R"(
    #version 120
    attribute vec2 position;
    attribute vec2 texcoord;
    varying vec2 vTexcoord;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        vTexcoord = texcoord;
    }
)";

// Fragment shader: Maps the one-channel heat texture to a color gradient.
const char* fragmentShaderSource = R"(
    #version 120
    uniform sampler2D heatTexture;
    varying vec2 vTexcoord;
    void main() {
        float intensity = texture2D(heatTexture, vTexcoord).r;
        vec3 color;
        // Custom gradient: blue -> cyan -> yellow -> red.
        if (intensity < 0.33) {
            color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), intensity / 0.33);
        } else if (intensity < 0.66) {
            color = mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), (intensity - 0.33) / 0.33);
        } else {
            color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (intensity - 0.66) / 0.34);
        }
        gl_FragColor = vec4(color, 1.0);
    }
)";

// Utility function to compile a shader.
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        cerr << "Shader compilation error: " << info << endl;
        exit(EXIT_FAILURE);
    }
    return shader;
}

// Create and link the shader program.
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    // Bind attribute locations explicitly.
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "texcoord");
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, 512, nullptr, info);
        cerr << "Program linking error: " << info << endl;
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// Initialize GLFW and create an OpenGL window.
GLFWwindow* initOpenGL(int width, int height, const char* title) {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        return nullptr;
    }
    // Request an OpenGL 2.1 context (compatible with GLSL 1.20)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
        return nullptr;
    }
    return window;
}

// Create an OpenGL texture from the simulation grid data.
// The grid values (assumed to be in [0,100]) are mapped to an 8-bit intensity.
GLuint createTextureFromGrid(const vector<double>& grid) {
    // Create a single-channel luminance image.
    vector<GLubyte> image(NX * NY);
    for (int i = 0; i < NY; i++) {
        for (int j = 0; j < NX; j++) {
            double temp = grid[i * NX + j];
            if (temp < 0.0) temp = 0.0;
            if (temp > 100.0) temp = 100.0;
            GLubyte intensity = static_cast<GLubyte>((temp / 100.0) * 255);
            image[i * NX + j] = intensity;
        }
    }
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixelated filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texID;
}

// ------------------------------------------------------------------
// Main Simulation and Visualization
// ------------------------------------------------------------------
int main(int argc, char **argv) {
    int rank, size;

    // Initialize MPI.
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Divide the grid rows among processes.
    int local_NY = NY / size;         // Number of interior rows per process.
    int local_rows = local_NY + 2;      // Add 2 ghost rows (one on top, one on bottom).

    // Allocate simulation arrays.
    vector<double> current(local_rows * NX, 0.0);
    vector<double> next(local_rows * NX, 0.0);

    // Initialize the simulation:
    //   - Set the center heat source cells (global coordinates) to 100.
    //   - Other cells remain at 0.
    // For each local interior row, compute its global row index.
    for (int i = 1; i < local_rows - 1; i++) {
        int global_i = rank * local_NY + (i - 1);
        for (int j = 0; j < NX; j++) {
            if (global_i >= center_i_start && global_i < center_i_end &&
                j >= center_j_start && j < center_j_end) {
                current[IDX(i, j)] = 100.0;
                next[IDX(i, j)] = 100.0;
            } else {
                current[IDX(i, j)] = 0.0;
                next[IDX(i, j)] = 0.0;
            }
        }
    }

    // Main simulation loop (Jacobi relaxation).
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double local_diff = 0.0;
        MPI_Status status;

        // Exchange ghost rows with neighboring processes.
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

        // Update interior grid points.
        for (int i = 1; i < local_rows - 1; i++) {
            int global_i = rank * local_NY + (i - 1);
            for (int j = 1; j < NX - 1; j++) {
                // Check if this cell is part of the fixed center heat source.
                if (global_i >= center_i_start && global_i < center_i_end &&
                    j >= center_j_start && j < center_j_end) {
                    // Fixed heat source: do not update.
                    next[IDX(i, j)] = current[IDX(i, j)];
                } else {
                    // Update using the average of the four neighbors.
                    next[IDX(i, j)] = 0.25 * (current[IDX(i - 1, j)] +
                                              current[IDX(i + 1, j)] +
                                              current[IDX(i, j - 1)] +
                                              current[IDX(i, j + 1)]);
                    local_diff += fabs(next[IDX(i, j)] - current[IDX(i, j)]);
                }
            }
        }
        // Global convergence check.
        double global_diff;
        MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        if (global_diff < TOL) {
            if (rank == 0)
                cout << "Converged after " << iter << " iterations with global diff = " << global_diff << endl;
            break;
        }
        swap(current, next);
    }

    // Gather results at rank 0.
    vector<double> full_grid;
    if (rank == 0) {
        full_grid.resize(NY * NX);
    }
    MPI_Gather(&current[IDX(1, 0)], local_NY * NX, MPI_DOUBLE,
               full_grid.data(), local_NY * NX, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // ------------------------------------------------------------------
    // Visualization using Modern OpenGL (Only on Rank 0)
    // ------------------------------------------------------------------
    if (rank == 0) {
        const int winWidth = 800, winHeight = 800;
        GLFWwindow* window = initOpenGL(winWidth, winHeight, "Heat Source in the Center");
        if (!window) {
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        GLuint shaderProgram = createShaderProgram();

        // Define vertices and texture coordinates for a full-screen quad.
        GLfloat vertices[] = {
            // Positions    // Texture Coordinates
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f,
            -1.0f,  1.0f,   0.0f, 1.0f
        };
        GLuint indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        // Setup VAO, VBO, and EBO.
        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Position attribute (location = 0)
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            // Texture coordinate attribute (location = 1)
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        // Create a texture from the simulation grid.
        GLuint textureID = createTextureFromGrid(full_grid);

        // Render loop.
        while (!glfwWindowShouldClose(window)) {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(shaderProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            GLint texLoc = glGetUniformLocation(shaderProgram, "heatTexture");
            glUniform1i(texLoc, 0);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup OpenGL resources.
        glDeleteTextures(1, &textureID);
        glDeleteProgram(shaderProgram);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    MPI_Finalize();
    return 0;
}
