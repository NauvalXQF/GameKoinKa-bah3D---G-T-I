#include "globals.h"
#include "environment.h"

GLuint kaabaTex;
GLuint wallTex;

unsigned char* loadBMP(const char* file, int* w, int* h) {
    FILE* f = fopen(file, "rb");
    if (!f) return NULL;

    unsigned char hdr[128];
    int read = fread(hdr, 1, 128, f);
    if (read < 54 || hdr[0] != 'B' || hdr[1] != 'M') { fclose(f); return NULL; }

    int offset = *(int*)&hdr[10];
    *w = *(int*)&hdr[18];
    *h = abs(*(int*)&hdr[22]);
    int bpp = *(short*)&hdr[28];
    if (*w <= 0 || *h <= 0 || (bpp != 24 && bpp != 32)) { fclose(f); return NULL; }

    int bppBytes = bpp / 8;
    int rowSize = ((*w * bpp + 31) / 32) * 4;

    if (offset > 128) fseek(f, offset, SEEK_SET);
    else if (offset > 54) fseek(f, offset, SEEK_SET);

    unsigned char* data = new unsigned char[*w * *h * 3];
    unsigned char* row = new unsigned char[rowSize];

    for (int y = 0; y < *h; y++) {
        fread(row, 1, rowSize, f);
        for (int x = 0; x < *w; x++) {
            int si = x * bppBytes;
            int di = (y * *w + x) * 3;
            data[di]     = row[si + 2];
            data[di + 1] = row[si + 1];
            data[di + 2] = row[si];
        }
    }

    delete[] row;
    fclose(f);
    return data;
}

GLuint loadTex(const char* file, bool repeat) {
    int w, h;
    unsigned char* data = loadBMP(file, &w, &h);
    if (!data) { printf("Failed to load texture: %s\n", file); return 0; }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP);

    delete[] data;
    return tex;
}

void initTextures() {
    kaabaTex = loadTex("kaaba.bmp", false);
    wallTex = loadTex("wall.bmp", true);
}

void drawWalls() {
    if (!wallTex) {
        GLfloat wall_ambient[] = { 0.7f, 0.7f, 0.6f, 1.0f };
        GLfloat wall_diffuse[] = { 0.8f, 0.8f, 0.7f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, wall_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, wall_diffuse);
        GLfloat no_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, no_specular);

        float wallHeight = 15.0f;
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-MAP_LIMIT, 0.0f, -MAP_LIMIT);
        glVertex3f( MAP_LIMIT, 0.0f, -MAP_LIMIT);
        glVertex3f( MAP_LIMIT, wallHeight, -MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, wallHeight, -MAP_LIMIT);

        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f( MAP_LIMIT, 0.0f, MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, 0.0f, MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, wallHeight, MAP_LIMIT);
        glVertex3f( MAP_LIMIT, wallHeight, MAP_LIMIT);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-MAP_LIMIT, 0.0f,  MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, 0.0f, -MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, wallHeight, -MAP_LIMIT);
        glVertex3f(-MAP_LIMIT, wallHeight,  MAP_LIMIT);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f( MAP_LIMIT, 0.0f, -MAP_LIMIT);
        glVertex3f( MAP_LIMIT, 0.0f,  MAP_LIMIT);
        glVertex3f( MAP_LIMIT, wallHeight,  MAP_LIMIT);
        glVertex3f( MAP_LIMIT, wallHeight, -MAP_LIMIT);
        glEnd();
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wallTex);

    GLfloat mat_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    GLfloat mat_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    float h = 15.0f;
    float repS = (MAP_LIMIT * 2.0f) / 10.0f;
    float repT = h / 5.0f;

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0); glVertex3f(-MAP_LIMIT, 0, -MAP_LIMIT);
    glTexCoord2f(repS, 0); glVertex3f(MAP_LIMIT, 0, -MAP_LIMIT);
    glTexCoord2f(repS, repT); glVertex3f(MAP_LIMIT, h, -MAP_LIMIT);
    glTexCoord2f(0, repT); glVertex3f(-MAP_LIMIT, h, -MAP_LIMIT);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 0); glVertex3f(MAP_LIMIT, 0, MAP_LIMIT);
    glTexCoord2f(repS, 0); glVertex3f(-MAP_LIMIT, 0, MAP_LIMIT);
    glTexCoord2f(repS, repT); glVertex3f(-MAP_LIMIT, h, MAP_LIMIT);
    glTexCoord2f(0, repT); glVertex3f(MAP_LIMIT, h, MAP_LIMIT);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-MAP_LIMIT, 0, MAP_LIMIT);
    glTexCoord2f(repS, 0); glVertex3f(-MAP_LIMIT, 0, -MAP_LIMIT);
    glTexCoord2f(repS, repT); glVertex3f(-MAP_LIMIT, h, -MAP_LIMIT);
    glTexCoord2f(0, repT); glVertex3f(-MAP_LIMIT, h, MAP_LIMIT);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(MAP_LIMIT, 0, -MAP_LIMIT);
    glTexCoord2f(repS, 0); glVertex3f(MAP_LIMIT, 0, MAP_LIMIT);
    glTexCoord2f(repS, repT); glVertex3f(MAP_LIMIT, h, MAP_LIMIT);
    glTexCoord2f(0, repT); glVertex3f(MAP_LIMIT, h, -MAP_LIMIT);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawFloor() {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    int tileSize = 5;
    for (int i = -MAP_LIMIT; i < MAP_LIMIT; i += tileSize) {
        for (int j = -MAP_LIMIT; j < MAP_LIMIT; j += tileSize) {
            if (abs((i + j) / tileSize) % 2 == 0) {
                GLfloat mat_amb_diff[] = { 0.85f, 0.85f, 0.85f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
            } else {
                GLfloat mat_amb_diff[] = { 0.75f, 0.75f, 0.75f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
            }

            GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
            glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
            glMaterialf(GL_FRONT, GL_SHININESS, 20.0f);

            glVertex3f(i, 0.0f, j);
            glVertex3f(i, 0.0f, j + tileSize);
            glVertex3f(i + tileSize, 0.0f, j + tileSize);
            glVertex3f(i + tileSize, 0.0f, j);
        }
    }
    glEnd();
}

void drawKaaba() {
    if (!kaabaTex) {
        GLfloat mat_ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
        GLfloat mat_diffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        GLfloat mat_specular[]= { 0.2f, 0.2f, 0.2f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);

        glPushMatrix();
        glTranslatef(0.0f, kaabaSize, 0.0f);
        glScalef(kaabaSize, kaabaSize, kaabaSize);
        glutSolidCube(2.0f);
        glPopMatrix();
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, kaabaTex);

    GLfloat mat_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat mat_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    float s = kaabaSize;
    float t = kaabaSize * 2.0f;

    glBegin(GL_QUADS);

    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-s, 0, s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, s);
    glTexCoord2f(1, 1); glVertex3f(s, t, s);
    glTexCoord2f(0, 1); glVertex3f(-s, t, s);

    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 0); glVertex3f(s, 0, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, 0, -s);
    glTexCoord2f(1, 1); glVertex3f(-s, t, -s);
    glTexCoord2f(0, 1); glVertex3f(s, t, -s);

    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, 0, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, 0, s);
    glTexCoord2f(1, 1); glVertex3f(-s, t, s);
    glTexCoord2f(0, 1); glVertex3f(-s, t, -s);

    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(s, 0, s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, -s);
    glTexCoord2f(1, 1); glVertex3f(s, t, -s);
    glTexCoord2f(0, 1); glVertex3f(s, t, s);

    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, t, -s);
    glTexCoord2f(1, 0); glVertex3f(s, t, -s);
    glTexCoord2f(1, 1); glVertex3f(s, t, s);
    glTexCoord2f(0, 1); glVertex3f(-s, t, s);

    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 0); glVertex3f(-s, 0, s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, s);
    glTexCoord2f(1, 1); glVertex3f(s, 0, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, 0, -s);

    glEnd();

    GLfloat gold_ambient[] = { 0.5f, 0.4f, 0.1f, 1.0f };
    GLfloat gold_diffuse[] = { 0.8f, 0.7f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, gold_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gold_diffuse);

    glPushMatrix();
    glTranslatef(0.0f, kaabaSize * 1.5f, 0.0f);
    glScalef(kaabaSize * 1.01f, kaabaSize * 0.15f, kaabaSize * 1.01f);
    glutSolidCube(2.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void drawCoins() {
    GLfloat coin_ambient[] = { 0.7f, 0.6f, 0.1f, 1.0f };
    GLfloat coin_diffuse[] = { 1.0f, 0.9f, 0.2f, 1.0f };
    GLfloat coin_specular[]= { 1.0f, 1.0f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, coin_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, coin_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, coin_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].active) {
            glPushMatrix();
            glTranslatef(coins[i].x, 0.6f, coins[i].z);
            glRotatef(coins[i].rotAngle, 0.0f, 1.0f, 0.0f);
            glutSolidTorus(0.1f, 0.3f, 15, 25);
            glPopMatrix();

            coins[i].rotAngle += 1.5f;
        }
    }
}

void drawMinaret(float x, float z) {
    GLfloat mat_white[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white);

    glPushMatrix();
    glTranslatef(x, 2.5f, z);
    glScalef(2.5f, 5.0f, 2.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, 5.0f, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 1.0, 0.7, 18.0, 16, 16);
    gluDeleteQuadric(q);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, 23.0f, z);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.8, 3.0, 16, 16);
    glPopMatrix();
}

void drawSunAndClouds() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glColor3f(1.0f, 0.9f, 0.0f);
    glTranslatef(light_position[0], light_position[1], light_position[2]);
    glutSolidSphere(4.0f, 20, 20);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 60.0f, -40.0f);
    glutSolidSphere(4.0, 20, 20);
    glTranslatef(4.0f, 0.0f, 2.0f);
    glutSolidSphere(5.0, 20, 20);
    glTranslatef(-8.0f, -1.0f, 1.0f);
    glutSolidSphere(3.5, 20, 20);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawKaabaShadow() {
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    float Lx = light_position[0], Ly = light_position[1], Lz = light_position[2];
    float shadowMat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        -Lx/Ly, 0.0f, -Lz/Ly, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(shadowMat);
    glTranslatef(0.0f, kaabaSize, 0.0f);
    glScalef(kaabaSize, kaabaSize, kaabaSize);
    glutSolidCube(2.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawHijrIsmail() {
    GLfloat mat_white[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white);

    glPushMatrix();
    for (int i = 0; i <= 180; i += 10) {
        float rad = i * 3.14159f / 180.0f;
        float x = cos(rad) * 6.5f;
        float z = sin(rad) * 6.5f;

        glPushMatrix();
        glTranslatef(x, 0.5f, z);
        glScalef(1.5f, 1.0f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawMaqamIbrahim() {
    glPushMatrix();
    glTranslatef(4.0f, 0.5f, -4.0f);

    GLfloat mat_gold[] = { 0.8f, 0.6f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_gold);
    glPushMatrix();
    glScalef(1.0f, 1.0f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glTranslatef(0.0f, 0.7f, 0.0f);
    glutSolidSphere(0.5f, 15, 15);
    glPopMatrix();
}

void drawGuards() {
    for (int i = 0; i < MAX_GUARDS; i++) {
        glPushMatrix();
        glTranslatef(guards[i].x, 0.2f, guards[i].z);

        // Ghost body
        GLfloat ghostColor[4];
        if (guards[i].isChaser) {
            // Reddish ghost (aggressive)
            ghostColor[0] = 0.9f; ghostColor[1] = 0.2f; ghostColor[2] = 0.2f; ghostColor[3] = 0.7f;
        } else {
            // Pale blue ghost (patrol)
            ghostColor[0] = 0.5f; ghostColor[1] = 0.7f; ghostColor[2] = 1.0f; ghostColor[3] = 0.6f;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLfloat mat_ghost[] = { ghostColor[0], ghostColor[1], ghostColor[2], ghostColor[3] };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ghost);

        // Body cone (ghostly trail)
        glPushMatrix();
        glTranslatef(0.0f, 1.5f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.6f, 1.8f, 15, 15);
        glPopMatrix();

        // Head
        glPushMatrix();
        glTranslatef(0.0f, 1.5f, 0.0f);
        glutSolidSphere(0.6f, 15, 15);
        glPopMatrix();

        // Eyes (glowing)
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(-0.2f, 1.7f, 0.5f);
        glutSolidSphere(0.1f, 8, 8);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.2f, 1.7f, 0.5f);
        glutSolidSphere(0.1f, 8, 8);
        glPopMatrix();
        glEnable(GL_LIGHTING);

        glDisable(GL_BLEND);

        glPopMatrix();
    }
}
