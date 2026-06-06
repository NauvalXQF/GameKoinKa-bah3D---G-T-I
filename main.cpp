#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>

#include "globals.h"
#include "environment.h"
#include "player.h"

int w = 800, h = 600;
float MAP_LIMIT = 150.0f;
float playerX = 0.0f, playerY = 1.0f, playerZ = 25.0f;
float playerAngle = 0.0f;
float lx = 0.0f, lz = -1.0f;
bool isTPS = true;
bool isJumping = false;
bool canJump = true;
float yVelocity = 0.0f;
float gravity = 0.012f;
float jumpForce = 0.25f;
bool keys[256];
bool specialKeys[256];
Coin coins[MAX_COINS];
int collectedCoins = 0;
float kaabaSize = 3.0f;
float walkCycle = 0.0f;
const float animSpeed = 0.15f;
GLfloat light_position[] = { 20.0f, 80.0f, 20.0f, 1.0f };
float playerPitch = 0.0f;
int gameState = 0;
float timeLeft = 300.0f;
int gameStartTime = 0;
int lastMouseX = 0, lastMouseY = 0;
bool mouseCaptured = false;
Guard guards[MAX_GUARDS];

// ============= SOUND =============
void generateWav(const char* filename, int freq, int durationMs, int volume, int endFreq) {
    int sampleRate = 22050;
    int numSamples = sampleRate * durationMs / 1000;
    int dataSize = numSamples * 2;

    FILE* f = fopen(filename, "rb");
    if (f) { fclose(f); return; }

    f = fopen(filename, "wb");
    if (!f) return;

    unsigned char hdr[44] = {0};
    memcpy(hdr, "RIFF", 4);
    int fs = 36 + dataSize;
    hdr[4] = fs&0xFF; hdr[5]=(fs>>8)&0xFF; hdr[6]=(fs>>16)&0xFF; hdr[7]=(fs>>24)&0xFF;
    memcpy(hdr+8, "WAVE", 4);
    memcpy(hdr+12, "fmt ", 4);
    hdr[16]=16; hdr[20]=1; hdr[22]=1;
    hdr[24]=sampleRate&0xFF; hdr[25]=(sampleRate>>8)&0xFF; hdr[26]=(sampleRate>>16)&0xFF; hdr[27]=(sampleRate>>24)&0xFF;
    int br = sampleRate*2;
    hdr[28]=br&0xFF; hdr[29]=(br>>8)&0xFF; hdr[30]=(br>>16)&0xFF; hdr[31]=(br>>24)&0xFF;
    hdr[32]=2; hdr[34]=16;
    memcpy(hdr+36, "data", 4);
    hdr[40]=dataSize&0xFF; hdr[41]=(dataSize>>8)&0xFF; hdr[42]=(dataSize>>16)&0xFF; hdr[43]=(dataSize>>24)&0xFF;
    fwrite(hdr, 1, 44, f);

    for (int i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        float dur = (float)durationMs / 1000.0f;
        float freq_t = freq + (float)(endFreq - freq) * (t / dur);
        float env = 1.0f;
        int fadeLen = sampleRate / 50;
        if (i < fadeLen) env = (float)i / fadeLen;
        if (i > numSamples - fadeLen * 2) env = (float)(numSamples - i) / (fadeLen * 2);
        short s = (short)(volume * env * sin(6.28318f * freq_t * t));
        fwrite(&s, 2, 1, f);
    }
    fclose(f);
}

void generateFaahWav() {
    const char* filename = "faah.wav";
    FILE* f = fopen(filename, "rb");
    if (f) { fclose(f); return; }
    int sr = 22050, dur = 500, ns = sr * dur / 1000, ds = ns * 2;
    f = fopen(filename, "wb");
    if (!f) return;
    unsigned char h[44] = {0};
    memcpy(h,"RIFF",4); int fs=36+ds;
    h[4]=fs&0xFF;h[5]=(fs>>8)&0xFF;h[6]=(fs>>16)&0xFF;h[7]=(fs>>24)&0xFF;
    memcpy(h+8,"WAVE",4); memcpy(h+12,"fmt ",4);
    h[16]=16;h[20]=1;h[22]=1;
    h[24]=sr&0xFF;h[25]=(sr>>8)&0xFF;h[26]=(sr>>16)&0xFF;h[27]=(sr>>24)&0xFF;
    int br=sr*2; h[28]=br&0xFF;h[29]=(br>>8)&0xFF;h[30]=(br>>16)&0xFF;h[31]=(br>>24)&0xFF;
    h[32]=2;h[34]=16; memcpy(h+36,"data",4);
    h[40]=ds&0xFF;h[41]=(ds>>8)&0xFF;h[42]=(ds>>16)&0xFF;h[43]=(ds>>24)&0xFF;
    fwrite(h,1,44,f);
    for (int i=0;i<ns;i++) {
        float t=(float)i/sr, d=(float)dur/1000;
        float ft=200+1800*(t/d)+40*sin(6.28318f*12*t);
        float e=1.0f; int fl=sr/30;
        if(i<fl)e=(float)i/fl;
        if(i>ns-fl*2)e=(float)(ns-i)/(fl*2);
        float samp=sin(6.28318f*ft*t)+0.4f*sin(6.28318f*ft*1.5f*t)+0.2f*sin(6.28318f*ft*2.3f*t);
        short s=(short)(16000*e*samp);
        fwrite(&s,2,1,f);
    }
    fclose(f);
}

void initSounds() {
    generateWav("coin.wav", 1200, 80, 8000, 1400);
    generateWav("gameover.wav", 400, 400, 12000, 150);
    generateWav("win.wav", 500, 500, 10000, 1000);
    generateWav("jump.wav", 600, 60, 6000, 700);
    generateFaahWav();
}

// ============= PARTICLES =============
#define MAX_PARTICLES 80

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life;
    bool active;
};

Particle particles[MAX_PARTICLES];

void spawnParticlesBurst(float x, float y, float z, int count) {
    for (int n = 0; n < count; n++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].x = x;
                particles[i].y = y;
                particles[i].z = z;
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float speed = 0.03f + (rand() % 100) / 800.0f;
                particles[i].vx = cos(angle) * speed;
                particles[i].vy = 0.08f + (rand() % 100) / 500.0f;
                particles[i].vz = sin(angle) * speed;
                particles[i].life = 0.8f + (rand() % 50) / 100.0f;
                particles[i].active = true;
                break;
            }
        }
    }
}

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].z += particles[i].vz;
            particles[i].vy -= 0.003f;
            particles[i].life -= 0.015f;
            if (particles[i].life <= 0.0f) particles[i].active = false;
        }
    }
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(7.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            glColor4f(1.0f, 0.8f * particles[i].life, 0.0f, particles[i].life);
            glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        }
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ============= MINI-MAP =============
void drawMinimap() {
    int mapSize = 140;
    int mapX = w - mapSize - 15;
    int mapY = 15;
    float scale = mapSize / (MAP_LIMIT * 2.0f);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Background
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(mapX, mapY);
    glVertex2f(mapX + mapSize, mapY);
    glVertex2f(mapX + mapSize, mapY + mapSize);
    glVertex2f(mapX, mapY + mapSize);
    glEnd();

    // Border
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(mapX, mapY);
    glVertex2f(mapX + mapSize, mapY);
    glVertex2f(mapX + mapSize, mapY + mapSize);
    glVertex2f(mapX, mapY + mapSize);
    glEnd();

    // Kaaba
    glColor3f(0.1f, 0.1f, 0.1f);
    float kx = mapX + (0.0f + MAP_LIMIT) * scale;
    float ky = mapY + (0.0f + MAP_LIMIT) * scale;
    float ks = kaabaSize * 1.5f * scale;
    glBegin(GL_QUADS);
    glVertex2f(kx - ks, ky - ks);
    glVertex2f(kx + ks, ky - ks);
    glVertex2f(kx + ks, ky + ks);
    glVertex2f(kx - ks, ky + ks);
    glEnd();

    // Player (green triangle pointing in look direction)
    glColor3f(0.0f, 1.0f, 0.0f);
    float px = mapX + (playerX + MAP_LIMIT) * scale;
    float py = mapY + (playerZ + MAP_LIMIT) * scale;
    float dirSize = 5.0f;
    float dx = sin(playerAngle) * dirSize;
    float dy = -cos(playerAngle) * dirSize;
    glBegin(GL_TRIANGLES);
    glVertex2f(px + dx * 2.0f, py + dy * 2.0f);
    glVertex2f(px - dy * 0.5f - dx * 0.3f, py + dx * 0.5f - dy * 0.3f);
    glVertex2f(px + dy * 0.5f - dx * 0.3f, py - dx * 0.5f - dy * 0.3f);
    glEnd();

    // Coins
    glColor3f(1.0f, 1.0f, 0.0f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].active) {
            float cx = mapX + (coins[i].x + MAP_LIMIT) * scale;
            float cy = mapY + (coins[i].z + MAP_LIMIT) * scale;
            glVertex2f(cx, cy);
        }
    }
    glEnd();

    // Guards
    glPointSize(4.0f);
    for (int i = 0; i < MAX_GUARDS; i++) {
        float gx = mapX + (guards[i].x + MAP_LIMIT) * scale;
        float gy = mapY + (guards[i].z + MAP_LIMIT) * scale;
        if (guards[i].isChaser)
            glColor3f(1.0f, 0.0f, 0.0f);
        else
            glColor3f(0.3f, 0.3f, 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(gx, gy);
        glEnd();
    }

    // Walls (square outline)
    glColor3f(0.6f, 0.6f, 0.6f);
    float wallOffset = 5.0f;
    float wx1 = mapX + wallOffset * scale;
    float wy1 = mapY + wallOffset * scale;
    float wx2 = mapX + (MAP_LIMIT * 2.0f - wallOffset) * scale;
    float wy2 = mapY + (MAP_LIMIT * 2.0f - wallOffset) * scale;
    glBegin(GL_LINE_LOOP);
    glVertex2f(wx1, wy1);
    glVertex2f(wx2, wy1);
    glVertex2f(wx2, wy2);
    glVertex2f(wx1, wy2);
    glEnd();

    // Label
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(mapX, mapY + mapSize + 8);
    const char* lbl = "MINI-MAP";
    for (int i = 0; i < (int)strlen(lbl); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, lbl[i]);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ============= START MENU =============
void drawMenu() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Gradient background
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.15f);
    glVertex2f(0, 0); glVertex2f(w, 0);
    glColor3f(0.1f, 0.1f, 0.3f);
    glVertex2f(w, h); glVertex2f(0, h);
    glEnd();

    // Title shadow
    const char* title = "GAME TAWAF 3D";
    int titleLen = strlen(title);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(w/2 - titleLen*8 + 2, h - 140 - 2);
    for (int i = 0; i < titleLen; i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, title[i]);

    // Title
    glColor3f(1.0f, 0.85f, 0.0f);
    glRasterPos2i(w/2 - titleLen*8, h - 140);
    for (int i = 0; i < titleLen; i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, title[i]);

    // Subtitle
    glColor3f(0.8f, 0.8f, 0.9f);
    const char* subtitle = "Kumpulkan Koin, Hindari Hantu!";
    glRasterPos2i(w/2 - 130, h - 190);
    for (int i = 0; i < (int)strlen(subtitle); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, subtitle[i]);

    // Gold line
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(w/2 - 150, h - 210);
    glVertex2f(w/2 + 150, h - 210);
    glEnd();

    // Controls
    glColor3f(1.0f, 1.0f, 1.0f);
    const char* controls[] = {
        "W / A / S / D / Panah - Bergerak",
        "SPASI - Lompat",
        "Klik Kiri - Capture Mouse (lihat-lihat)",
        "Klik Kanan - Lepas Mouse",
        "C - Ganti Kamera (TPS / FPS)",
        "Q / ESC - Keluar",
        ""
    };
    for (int i = 0; controls[i][0] != '\0'; i++) {
        glRasterPos2i(w/2 - 170, h - 270 - i * 28);
        for (int j = 0; j < (int)strlen(controls[i]); j++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, controls[i][j]);
    }

    // Blinking start text
    int blink = (glutGet(GLUT_ELAPSED_TIME) / 500) % 2;
    if (blink) {
        glColor3f(0.0f, 1.0f, 0.2f);
        const char* start = "=== Tekan SPASI untuk Mulai ===";
        glRasterPos2i(w/2 - 150, 120);
        for (int i = 0; i < (int)strlen(start); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, start[i]);
    }

    // Credits
    glColor3f(0.4f, 0.4f, 0.4f);
    const char* credit = "Game Tawaf 3D - OpenGL / GLUT Project";
    glRasterPos2i(w/2 - 120, 30);
    for (int i = 0; i < (int)strlen(credit); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, credit[i]);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void initCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float radius = 8.0f + (rand() % (int)(MAP_LIMIT - 20.0f));
        coins[i].x = cos(angle) * radius;
        coins[i].z = sin(angle) * radius;
        coins[i].active = true;
        coins[i].rotAngle = rand() % 360;
    }
}

void resetGame();
void init() {
    glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    GLfloat light_ambient[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    initTextures();
    initCoins();
    initSounds();
    gameState = -1;
}

void resetGame() {
    playerX = 0.0f;
    playerY = 1.0f;
    playerZ = 25.0f;
    playerAngle = 0.0f;
    playerPitch = 0.0f;

    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;

    collectedCoins = 0;
    timeLeft = 300.0f;
    gameStartTime = glutGet(GLUT_ELAPSED_TIME);
    gameState = 0;
    isJumping = false;
    yVelocity = 0.0f;

    initCoins();

    guards[0].x = 30.0f; guards[0].z = 30.0f; guards[0].angle = 0.0f; guards[0].radius = 0.0f; guards[0].speed = 0.010f; guards[0].isChaser = true;
    guards[1].x = -40.0f; guards[1].z = -30.0f; guards[1].angle = 0.0f; guards[1].radius = 0.0f; guards[1].speed = 0.006f; guards[1].isChaser = true;
    guards[2].x = 0.0f; guards[2].z = 0.0f; guards[2].angle = 120.0f; guards[2].radius = 25.0f; guards[2].speed = -0.003f; guards[2].isChaser = false;
    guards[3].x = 0.0f; guards[3].z = 0.0f; guards[3].angle = 240.0f; guards[3].radius = 40.0f; guards[3].speed = 0.004f; guards[3].isChaser = false;
    guards[4].x = 0.0f; guards[4].z = 0.0f; guards[4].angle = 60.0f; guards[4].radius = 60.0f; guards[4].speed = 0.002f; guards[4].isChaser = false;
    guards[5].x = 0.0f; guards[5].z = 0.0f; guards[5].angle = 300.0f; guards[5].radius = 35.0f; guards[5].speed = -0.005f; guards[5].isChaser = false;
}

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 0.9f, 0.0f);
    glRasterPos2i(20, h - 35);

    char hudText[50];
    sprintf(hudText, "KOIN TERKUMPUL : %d / %d", collectedCoins, MAX_COINS);
    for (int i = 0; i < (int)strlen(hudText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, hudText[i]);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(20, h - 60);
    const char* camInfo = isTPS ? "Mode: TPS (Tekan C) | Lompat: Spasi" : "Mode: FPS (Tekan C) | Lompat: Spasi";
    for (int i = 0; i < (int)strlen(camInfo); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, camInfo[i]);
    }

    glColor3f(1.0f, 0.4f, 0.0f);
    glRasterPos2i(w / 2 - 50, h - 35);
    char timeText[50];
    int remaining = (int)timeLeft;
    if (remaining < 0) remaining = 0;
    sprintf(timeText, "WAKTU: %d", remaining);
    for (int i = 0; i < (int)strlen(timeText); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeText[i]);

    if (gameState == 1) {
        glColor3f(0.0f, 1.0f, 0.0f);
        const char* msg = "SELAMAT! ANDA MENANG!";
        glRasterPos2i(w / 2 - 100, h / 2);
        for (int i = 0; i < (int)strlen(msg); i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);

        glColor3f(1.0f, 1.0f, 1.0f);
        const char* info = "Tekan 'R' untuk Restart | 'Q' untuk Quit";
        glRasterPos2i(w / 2 - 120, h / 2 - 30);
        for (int i = 0; i < (int)strlen(info); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, info[i]);
    }
    else if (gameState == 2) {
        glColor3f(1.0f, 0.0f, 0.0f);
        const char* msg = "GAME OVER! ANDA KALAH!";
        glRasterPos2i(w / 2 - 110, h / 2);
        for (int i = 0; i < (int)strlen(msg); i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);

        glColor3f(1.0f, 1.0f, 1.0f);
        const char* info = "Tekan 'R' untuk Restart | 'Q' untuk Quit";
        glRasterPos2i(w / 2 - 120, h / 2 - 30);
        for (int i = 0; i < (int)strlen(info); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, info[i]);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void checkCollisions() {
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].active) {
            float dx = playerX - coins[i].x;
            float dy = playerY - 1.0f;
            float dz = playerZ - coins[i].z;
            float distance = sqrt(dx*dx + dy*dy + dz*dz);
            if (distance < 1.5f) {
                coins[i].active = false;
                collectedCoins++;
                spawnParticlesBurst(coins[i].x, 1.0f, coins[i].z, 10);
                PlaySoundA("coin.wav", NULL, SND_ASYNC | SND_FILENAME);
            }
        }
    }
}

void handleMovement() {
    if (gameState == -1) {
        updateParticles();
        return;
    }

    int prevState = gameState;
    float moveSpeed = 0.08f;
    float rotSpeed = 0.007f;
    float oldX = playerX;
    float oldZ = playerZ;
    bool isMoving = false;

    if (gameState != 0) {
        updateParticles();
        return;
    }

    timeLeft = 300.0f - (glutGet(GLUT_ELAPSED_TIME) - gameStartTime) / 1000.0f;
    if (timeLeft <= 0) {
        timeLeft = 0;
        gameState = 2;
    }

    if (collectedCoins >= MAX_COINS) {
        gameState = 1;
    }

    bool ghostHit = false;
    for (int i = 0; i < MAX_GUARDS; i++) {
        if (guards[i].isChaser) {
            float dx = playerX - guards[i].x;
            float dz = playerZ - guards[i].z;
            float dist = sqrt(dx*dx + dz*dz);
            if (dist > 0.1f) {
                guards[i].x += (dx / dist) * guards[i].speed;
                guards[i].z += (dz / dist) * guards[i].speed;
            }
        } else {
            guards[i].angle += guards[i].speed;
            if (guards[i].angle > 6.28318f) guards[i].angle -= 6.28318f;
            if (guards[i].angle < 0.0f) guards[i].angle += 6.28318f;

            guards[i].x = cos(guards[i].angle) * guards[i].radius;
            guards[i].z = sin(guards[i].angle) * guards[i].radius;
        }

        float distG = sqrt((playerX - guards[i].x)*(playerX - guards[i].x) + (playerZ - guards[i].z)*(playerZ - guards[i].z));
        if (distG < 1.5f) { gameState = 2; ghostHit = true; }
    }

    if (prevState != gameState) {
        if (gameState == 1) PlaySoundA("win.wav", NULL, SND_ASYNC | SND_FILENAME);
        if (gameState == 2 && ghostHit) PlaySoundA("faah.wav", NULL, SND_ASYNC | SND_FILENAME);
        else if (gameState == 2) PlaySoundA("gameover.wav", NULL, SND_ASYNC | SND_FILENAME);
    }

    if (keys[' '] && !isJumping && canJump) {
        isJumping = true;
        canJump = false;
        yVelocity = jumpForce;
        PlaySoundA("jump.wav", NULL, SND_ASYNC | SND_FILENAME);
    }

    if (isJumping) {
        playerY += yVelocity;
        yVelocity -= gravity;
        if (playerY <= 1.0f) {
            playerY = 1.0f;
            isJumping = false;
            yVelocity = 0.0f;
        }
    }

    if (specialKeys[GLUT_KEY_LEFT]) playerAngle -= rotSpeed;
    if (specialKeys[GLUT_KEY_RIGHT]) playerAngle += rotSpeed;
    if (specialKeys[GLUT_KEY_UP]) {
        playerPitch += 0.02f;
        if (playerPitch > 1.5f) playerPitch = 1.5f;
    }
    if (specialKeys[GLUT_KEY_DOWN]) {
        playerPitch -= 0.02f;
        if (playerPitch < -1.5f) playerPitch = -1.5f;
    }

    lx = sin(playerAngle);
    lz = -cos(playerAngle);

    if (keys['w'] || keys['W']) {
        playerX += lx * moveSpeed;
        playerZ += lz * moveSpeed;
        isMoving = true;
    }
    if (keys['s'] || keys['S']) {
        playerX -= lx * moveSpeed;
        playerZ -= lz * moveSpeed;
        isMoving = true;
    }
    if (keys['a'] || keys['A']) {
        playerX -= cos(playerAngle) * moveSpeed;
        playerZ -= sin(playerAngle) * moveSpeed;
        isMoving = true;
    }
    if (keys['d'] || keys['D']) {
        playerX += cos(playerAngle) * moveSpeed;
        playerZ += sin(playerAngle) * moveSpeed;
        isMoving = true;
    }

    if (isMoving) {
        walkCycle += animSpeed;
    } else {
        walkCycle *= 0.8f;
        if (walkCycle < 0.01f) walkCycle = 0.0f;
    }
    if (walkCycle > 6.28318f) walkCycle -= 6.28318f;

    if (playerX > -(kaabaSize+1.0f) && playerX < (kaabaSize+1.0f) &&
        playerZ > -(kaabaSize+1.0f) && playerZ < (kaabaSize+1.0f)) {
        playerX = oldX;
        playerZ = oldZ;
    }

    for (int i = 0; i < 360; i += 30) {
        float rad = i * 3.14159f / 180.0f;
        float px = cos(rad) * 40.0f;
        float pz = sin(rad) * 40.0f;
        float dx = playerX - px;
        float dz = playerZ - pz;
        if (sqrt(dx*dx + dz*dz) < 2.0f) {
            playerX = oldX;
            playerZ = oldZ;
        }
    }

    float minarets[4][2] = {{60.0f, 60.0f}, {-60.0f, 60.0f}, {60.0f, -60.0f}, {-60.0f, -60.0f}};
    for (int i = 0; i < 4; i++) {
        float dx = playerX - minarets[i][0];
        float dz = playerZ - minarets[i][1];
        if (sqrt(dx*dx + dz*dz) < 3.5f) {
            playerX = oldX;
            playerZ = oldZ;
        }
    }

    float dxM = playerX - 4.0f;
    float dzM = playerZ - -4.0f;
    if (sqrt(dxM*dxM + dzM*dzM) < 1.8f) {
        playerX = oldX; playerZ = oldZ;
    }

    for (int i = 0; i <= 180; i += 10) {
        float rad = i * 3.14159f / 180.0f;
        float wx = cos(rad) * 6.5f;
        float wz = sin(rad) * 6.5f;
        float dxH = playerX - wx;
        float dzH = playerZ - wz;
        if (sqrt(dxH*dxH + dzH*dzH) < 1.5f) {
            playerX = oldX;
            playerZ = oldZ;
        }
    }

    float playerRadius = 1.0f;
    if (playerX > MAP_LIMIT - playerRadius) playerX = MAP_LIMIT - playerRadius;
    if (playerX < -(MAP_LIMIT - playerRadius)) playerX = -(MAP_LIMIT - playerRadius);
    if (playerZ > MAP_LIMIT - playerRadius) playerZ = MAP_LIMIT - playerRadius;
    if (playerZ < -(MAP_LIMIT - playerRadius)) playerZ = -(MAP_LIMIT - playerRadius);

    checkCollisions();
    updateParticles();
}

void display() {
    if (gameState == -1) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawMenu();
        glutSwapBuffers();
        return;
    }

    handleMovement();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lookX = sin(playerAngle) * cos(playerPitch);
    float lookY = sin(playerPitch);
    float lookZ = -cos(playerAngle) * cos(playerPitch);

    if (isTPS) {
        float camDist = 7.0f;
        float camHeight = 3.5f;

        float camX = playerX - lookX * camDist;
        float camZ = playerZ - lookZ * camDist;

        gluLookAt(camX, playerY + camHeight, camZ,
                  playerX, playerY + 1.5f + (lookY * 15.0f), playerZ,
                  0.0f, 1.0f, 0.0f);
    } else {
        float eyeHeight = 1.6f;
        gluLookAt(playerX, playerY + eyeHeight, playerZ,
                  playerX + lookX, playerY + eyeHeight + lookY, playerZ + lookZ,
                  0.0f, 1.0f, 0.0f);
    }

    drawFloor();
    drawKaabaShadow();
    drawPlayerShadow();

    drawWalls();
    drawKaaba();
    drawHijrIsmail();
    drawMaqamIbrahim();
    for (int i = 0; i < 360; i += 30) {
        float rad = i * 3.14159f / 180.0f;
        drawMinaret(cos(rad) * 40.0f, sin(rad) * 40.0f);
    }
    float menara[4][2] = {{60,60},{-60,60},{60,-60},{-60,-60}};
    for (int i = 0; i < 4; i++) drawMinaret(menara[i][0], menara[i][1]);
    drawGuards();
    drawCoins();
    drawPlayer();
    drawSunAndClouds();

    drawParticles();
    drawHUD();
    drawMinimap();
    glutSwapBuffers();
}

void reshape(int w1, int h1) {
    if (h1 == 0) h1 = 1;
    w = w1; h = h1;
    float ratio = 1.0f * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(60.0f, ratio, 0.1f, 1000.0f);
}

void idle() {
    glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y) {
    keys[key] = true;

    if (gameState == -1) {
        if (key == ' ') {
            resetGame();
        }
        if (key == 27) exit(0);
        return;
    }

    if (key == 'c' || key == 'C') {
        isTPS = !isTPS;
    }

    if (key == 27) exit(0);

    if (gameState != 0) {
        if (key == 'r' || key == 'R') {
            resetGame();
        }
        if (key == 'q' || key == 'Q') {
            exit(0);
        }
    }
}

void keyUp(unsigned char key, int x, int y) {
    keys[key] = false;
    if (key == ' ') { canJump = true; }
}

void specialKeyDown(int key, int x, int y) {
    specialKeys[key] = true;
}

void specialKeyUp(int key, int x, int y) {
    specialKeys[key] = false;
}

void mouseClick(int button, int state, int x, int y) {
    if (gameState == -1) return;
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON && !mouseCaptured) {
            mouseCaptured = true;
            lastMouseX = x;
            lastMouseY = y;
            glutSetCursor(GLUT_CURSOR_NONE);
        } else if (button == GLUT_RIGHT_BUTTON && mouseCaptured) {
            mouseCaptured = false;
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
}

void mouseMove(int x, int y) {
    if (gameState == -1) return;
    if (mouseCaptured) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        playerAngle += dx * 0.001f;
        playerPitch -= dy * 0.001f;

        if (playerPitch > 1.5f) playerPitch = 1.5f;
        if (playerPitch < -1.5f) playerPitch = -1.5f;

        lastMouseX = w / 2;
        lastMouseY = h / 2;
        glutWarpPointer(lastMouseX, lastMouseY);
    } else {
        lastMouseX = x;
        lastMouseY = y;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Game Tawaf 3D - Modular Code");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(mouseMove);

    glutMainLoop();
    return 0;
}
