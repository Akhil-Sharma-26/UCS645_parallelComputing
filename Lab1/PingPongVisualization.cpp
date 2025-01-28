// ! doing non blocking send and recieve, but not working as I though it would

#include <mpi.h>
#include <GL/glut.h>
#include <iostream>
#include <cmath>

float ballX = -0.5f;
float ballY = 0.0f;
int currentCount = 0;
bool isMovingRight = true;
float animationSpeed = 0.02f;
const float PROCESS_RADIUS = 0.15f;

int myRank, worldSize;
const int PING_PONG_LIMIT = 10;
const int TAG = 0;
bool isMyTurn = false;

// Add message state tracking
bool messageSent = false;
bool messageReceived = false;
MPI_Request request = MPI_REQUEST_NULL;
MPI_Status status;

void drawCircle(float cx, float cy, float radius, bool highlighted) {
    const int segments = 100;
    if (highlighted) {
        glColor3f(1.0f, 0.0f, 0.0f);
    } else {
        glColor3f(0.0f, 0.0f, 1.0f);
    }

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    drawCircle(-0.5f, 0.0f, PROCESS_RADIUS, myRank == 0 && isMyTurn);
    drawCircle(0.5f, 0.0f, PROCESS_RADIUS, myRank == 1 && isMyTurn);
    
    glColor3f(0.0f, 1.0f, 0.0f);
    drawCircle(ballX, ballY, 0.05f, true);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.55f, -0.3f);
    std::string label0 = "Process 0";
    for (char c : label0) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
    
    glRasterPos2f(0.45f, -0.3f);
    std::string label1 = "Process 1";
    for (char c : label1) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
    
    glutSwapBuffers();
}

void update(int value) {
    if (currentCount < PING_PONG_LIMIT) {
        // Check if it's this process's turn to send
        if (myRank == currentCount % 2 && !messageSent) {
            int sendValue = currentCount;
            MPI_Isend(&sendValue, 1, MPI_INT, (myRank + 1) % 2, TAG, MPI_COMM_WORLD, &request);
            std::cout << "Process " << myRank << " sent value " << sendValue << std::endl;
            messageSent = true;
            isMyTurn = false;
        }
        // Check if this process should receive
        else if (myRank != currentCount % 2 && !messageReceived) {
            int flag = 0;
            MPI_Iprobe((myRank + 1) % 2, TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                int recvValue;
                MPI_Recv(&recvValue, 1, MPI_INT, (myRank + 1) % 2, TAG, MPI_COMM_WORLD, &status);
                std::cout << "Process " << myRank << " received value " << recvValue << std::endl;
                messageReceived = true;
                currentCount++;
                messageSent = false;
                messageReceived = false;
                isMyTurn = true;
            }
        }

        // Update ball position
        if (isMovingRight) {
            ballX += animationSpeed;
            if (ballX >= 0.5f) {
                isMovingRight = false;
            }
        } else {
            ballX -= animationSpeed;
            if (ballX <= -0.5f) {
                isMovingRight = true;
            }
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    
    if (worldSize != 2) {
        if (myRank == 0) {
            std::cout << "This program requires exactly 2 processes!" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(myRank * 420, 100);
    std::string title = "Process " + std::to_string(myRank);
    glutCreateWindow(title.c_str());
    
    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    
    // Set initial turn for Process 0
    isMyTurn = (myRank == 0);
    
    glutMainLoop();
    
    MPI_Finalize();
    return 0;
}