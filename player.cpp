#include "globals.h"
#include "player.h"

// Konstanta sudut ayunan (dalam derajat)
const float maxArmSwing = 30.0f; 
const float maxLegSwing = 25.0f; 

void drawPlayer() {
    if (!isTPS) return; 

    GLfloat mat_ambient[] = { 0.1f, 0.4f, 0.1f, 1.0f };
    GLfloat mat_diffuse[] = { 0.2f, 0.8f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    // --- HITUNG SUDUT AYUNAN BERDASARKAN walkCycle ---
    // Gunakan fungsi sin() agar gerakan berayun halus bolak-balik
    float armSwing = maxArmSwing * sin(walkCycle);
    float legSwing = maxLegSwing * sin(walkCycle);

    glPushMatrix();
        glTranslatef(playerX, playerY, playerZ);
        glRotatef(playerAngle * 180.0f / 3.14159f, 0.0f, -1.0f, 0.0f);
        
        // --- 1. BADAN UTAMA (TORSO) ---
        glPushMatrix();
            glScalef(0.6f, 1.0f, 0.4f); // Sedikit dipendekkan agar pas
            glutSolidCube(1.0f);
        glPopMatrix();
        
        // --- 2. KEPALA ---
        glPushMatrix();
            glTranslatef(0.0f, 0.8f, 0.0f); // Sesuaikan ketinggian
            glutSolidSphere(0.3f, 20, 20);
        glPopMatrix();

        // --- 3. TANGAN KIRI ---
        glPushMatrix();
            glTranslatef(-0.35f, 0.4f, 0.0f); // Posisi sendi bahu
            glRotatef(armSwing, 1.0f, 0.0f, 0.0f); // Sendi berputar ke depan/belakang
            glTranslatef(0.0f, -0.4f, 0.0f); // Pindahkan pivot ke atas lengan
            glScalef(0.2f, 0.8f, 0.2f); // Skala menjadi bentuk lengan
            glutSolidCube(1.0f);
        glPopMatrix();

        // --- 4. TANGAN KANAN ---
        glPushMatrix();
            glTranslatef(0.35f, 0.4f, 0.0f); // Posisi sendi bahu
            glRotatef(-armSwing, 1.0f, 0.0f, 0.0f); // Berlawanan arah dengan tangan kiri
            glTranslatef(0.0f, -0.4f, 0.0f); // Pindahkan pivot ke atas lengan
            glScalef(0.2f, 0.8f, 0.2f); // Skala menjadi bentuk lengan
            glutSolidCube(1.0f);
        glPopMatrix();

        // --- 5. KAKI KIRI ---
        glPushMatrix();
            glTranslatef(-0.2f, -0.5f, 0.0f); // Posisi sendi pinggul
            glRotatef(-legSwing, 1.0f, 0.0f, 0.0f); // Berlawanan arah dengan tangan kiri (sinkron dengan tangan kanan)
            glTranslatef(0.0f, -0.5f, 0.0f); // Pindahkan pivot ke atas kaki
            glScalef(0.3f, 1.0f, 0.3f); // Skala menjadi bentuk kaki
            glutSolidCube(1.0f);
        glPopMatrix();

        // --- 6. KAKI KANAN ---
        glPushMatrix();
            glTranslatef(0.2f, -0.5f, 0.0f); // Posisi sendi pinggul
            glRotatef(legSwing, 1.0f, 0.0f, 0.0f); // Berlawanan arah dengan kaki kiri (sinkron dengan tangan kiri)
            glTranslatef(0.0f, -0.5f, 0.0f); // Pindahkan pivot ke atas kaki
            glScalef(0.3f, 1.0f, 0.3f); // Skala menjadi bentuk kaki
            glutSolidCube(1.0f);
        glPopMatrix();

    glPopMatrix();
}

void drawPlayerShadow() {
    if (!isTPS) return; 

    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.2f); // Warna bayangan gelap
    
    glPushMatrix();
        glTranslatef(0.0f, 0.05f, 0.0f); // Di atas lantai sedikit
        
        // --- MATRIKS PROYEKSI BAYANGAN ---
        float Lx = light_position[0];
        float Ly = light_position[1];
        float Lz = light_position[2];
        
        float shadowMat[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            -Lx/Ly, 0.0f, -Lz/Ly, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glMultMatrixf(shadowMat);
        
        // --- GAMBAR KERANGKA PLAYER ---
        float armSwing = 30.0f * sin(walkCycle);
        float legSwing = 25.0f * sin(walkCycle);

        glTranslatef(playerX, playerY, playerZ);
        glRotatef(playerAngle * 180.0f / 3.14159f, 0.0f, -1.0f, 0.0f);
        
        // Badan
        glPushMatrix();
            glScalef(0.6f, 1.0f, 0.4f);
            glutSolidCube(1.0f);
        glPopMatrix();
        // Kepala
        glPushMatrix();
            glTranslatef(0.0f, 0.8f, 0.0f);
            glutSolidSphere(0.3f, 20, 20);
        glPopMatrix();
        // Tangan Kiri
        glPushMatrix();
            glTranslatef(-0.35f, 0.4f, 0.0f);
            glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.4f, 0.0f);
            glScalef(0.2f, 0.8f, 0.2f);
            glutSolidCube(1.0f);
        glPopMatrix();
        // Tangan Kanan
        glPushMatrix();
            glTranslatef(0.35f, 0.4f, 0.0f);
            glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.4f, 0.0f);
            glScalef(0.2f, 0.8f, 0.2f);
            glutSolidCube(1.0f);
        glPopMatrix();
        // Kaki Kiri
        glPushMatrix();
            glTranslatef(-0.2f, -0.5f, 0.0f);
            glRotatef(-legSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.5f, 0.0f);
            glScalef(0.3f, 1.0f, 0.3f);
            glutSolidCube(1.0f);
        glPopMatrix();
        // Kaki Kanan
        glPushMatrix();
            glTranslatef(0.2f, -0.5f, 0.0f);
            glRotatef(legSwing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.5f, 0.0f);
            glScalef(0.3f, 1.0f, 0.3f);
            glutSolidCube(1.0f);
        glPopMatrix();

    glPopMatrix();
    glEnable(GL_LIGHTING);
}
