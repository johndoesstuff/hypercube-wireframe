#include <stdio.h>
#include <math.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define D 4

void matrixMultiply(float m[D][D], float *vec, float *out) {
        for (int i = 0; i < D; i++) {
                out[i] = 0;
                for (int j = 0; j < D; j++) {
                        out[i] += vec[j] * m[i][j];
                }
        }
}

void rotate(float *vec, int *plane, float *out, float theta) {
        float m[D][D];
        for (int i = 0; i < D; i++) {
                for (int j = 0; j < D; j++) {
                        if (i == plane[0] && j == plane[0]) {
                                m[i][j] = cos(theta);
                        } else if (i == plane[0] && j == plane[1]) {
                                m[i][j] = -sin(theta);
                        } else if (i == plane[1] && j == plane[0]) {
                                m[i][j] = sin(theta);
                        } else if (i == plane[1] && j == plane[1]) {
                                m[i][j] = cos(theta);
                        } else if (i == j) {
                                m[i][j] = 1;
                        } else {
                                m[i][j] = 0;
                        }
                }
        }
        matrixMultiply(m, vec, out);
}

float shypot(float *a) {
        float sum = 0;
        for (int i = 0; i < D; i++) {
                sum += a[i];
        }
        return sum;
}

int max(int a, int b) {
        return (a > b) ? a : b;
}

int min(int a, int b) {
        return (a < b) ? a : b;
}

int main() {
        int f = 0;
        float dist = 2 + D;

        int rotationPlanes = D * (D-1);
        float rotationSpeeds[rotationPlanes];
        for (int i = 0; i < rotationPlanes; i++) {
                rotationSpeeds[i] = 20.0 + ((float)rand() / (float)RAND_MAX)*30.0;
        }

        while(1) {
                //get window size
                struct winsize ws;

                if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
                        perror("ioctl");
                        exit(EXIT_FAILURE);
                }

                int screenX = ws.ws_col;
                int screenY = ws.ws_row-2;

                //set display buffer
                char displayBuffer[screenY][screenX][20];
                for (int y = 0; y < screenY; y++) {
                        for (int x = 0; x < screenX; x++) {
                                strcpy(displayBuffer[y][x], " ");
                        }
                }

                //create cube/hypercube
                int cubePoints = pow(2, D);
                float cube[cubePoints][D];
                for (int i = 0; i < pow(2, D); i++) {
                        for (int j = 0; j < D; j++) {
                                cube[i][j] = 2 * ((i >> j) & 1) - 1;
                        }
                }

                //rotate

                int s = 0;
                for (int p0 = 0; p0 < D; p0++) {
                        for (int p1 = p0 + 1; p1 < D; p1++) {
                                for (int i = 0; i < cubePoints; i++) {
                                        int plane[2] = { p0, p1 };
                                        float rotatedP[D];
                                        float theta = 1.0 * f / rotationSpeeds[s];
                                        rotate(cube[i], plane, rotatedP, theta);
                                        for (int j = 0; j < D; j++) {
                                                cube[i][j] = rotatedP[j];
                                        }
                                }
                                s++;
                        }
                }

                //flatten
                float flattenedCube[cubePoints][2];
                for (int i = 0; i < cubePoints; i++) {
                        flattenedCube[i][0] = cube[i][0];
                        flattenedCube[i][1] = cube[i][1];
                }

                float unflattenedCube[cubePoints][D];
                for (int i = 0; i < cubePoints; i++) {
                        for (int j = 0; j < D; j++) {
                                if (j <= 1) {
                                        unflattenedCube[i][j] = 0;
                                } else {
                                        unflattenedCube[i][j] = cube[i][j];
                                }
                        }
                }

                float distScalar[cubePoints];
                for (int i = 0; i < cubePoints; i++) {
                        distScalar[i] = 5*dist/(dist - shypot(unflattenedCube[i])) / (D+5);
                }

                for (int i = 0; i < cubePoints; i++) {
                        int rx = (int)round(screenX/2 + screenX/8*flattenedCube[i][0]*distScalar[i]);
                        int ry = (int)round(screenY/2 + screenY/4*flattenedCube[i][1]*distScalar[i]);
                        rx = min(max(rx, 0), screenX-1);
                        ry = min(max(ry, 0), screenY-1);
                        strcpy(displayBuffer[ry][rx], "\033[47m \033[0m");

                        for (int j = 0; j < D; j++) {
                                int targetIndex = i ^ (1 << j);
                                for (int k = 1; k <= 20; k++) {
                                        float x = (1.0 - k/41.0)*flattenedCube[i][0]*distScalar[i] + (k/41.0)*flattenedCube[targetIndex][0]*distScalar[targetIndex];
                                        float y = (1.0 - k/41.0)*flattenedCube[i][1]*distScalar[i] + (k/41.0)*flattenedCube[targetIndex][1]*distScalar[targetIndex];
                                        int rx = (int)round(screenX/2 + screenX/8*x);
                                        int ry = (int)round(screenY/2 + screenY/4*y);
                                        rx = min(max(rx, 0), screenX-1);
                                        ry = min(max(ry, 0), screenY-1);
                                        strcpy(displayBuffer[ry][rx], "\033[47m \033[0m");
                                }
                        }
                }

                //print to screen
                char printBuffer[1000000];
                printBuffer[0] = '\0';
                size_t offset = 0;
                for (int y = 0; y < screenY; y++) {
                        for (int x = 0; x < screenX; x++) {
                                offset += snprintf(printBuffer + offset, sizeof(printBuffer) - offset, "%s", displayBuffer[y][x]);
                        }
                        offset += snprintf(printBuffer + offset, sizeof(printBuffer) - offset, "\n");
                }

                printf("\033[H");

                printf("\033[?25l");

                printf("%s", printBuffer);
                printf("\033[?25h");

                fflush(stdout);

                f++;
                usleep(50000);
        }
}