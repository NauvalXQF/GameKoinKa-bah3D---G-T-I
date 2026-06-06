#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int w, h;
extern float MAP_LIMIT;

extern float playerX, playerY, playerZ;
extern float playerAngle;
extern float lx, lz;
extern bool isTPS;

extern bool isJumping;
extern bool canJump;
extern float yVelocity;
extern float gravity;
extern float jumpForce;
extern float playerPitch;

extern bool keys[256];
extern bool specialKeys[256];

#define MAX_COINS 30
struct Coin {
    float x, z;
    bool active;
    float rotAngle;
};
extern Coin coins[MAX_COINS];
extern int collectedCoins;

extern float kaabaSize;
extern GLfloat light_position[];
extern float walkCycle;

extern int gameState;
extern float timeLeft;
extern int gameStartTime;
extern int lastMouseX, lastMouseY;
extern bool mouseCaptured;

#define MAX_GUARDS 6
struct Guard {
    float x, z;
    float angle;
    float radius;
    float speed;
    bool isChaser;
};
extern Guard guards[MAX_GUARDS];

#endif
