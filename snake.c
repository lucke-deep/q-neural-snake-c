#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "raylib.h"

#define GRA 800
#define ALT 600
#define BLOCCO 30
#define GRIGLIA_X 20
#define GRIGLIA_Y 14
#define OFFSET_X ((GRA - GRIGLIA_X * BLOCCO) / 2)
#define OFFSET_Y ((ALT - GRIGLIA_Y * BLOCCO) / 2)
#define numerin 24
#define nas 128
#define nout 3 

struct corpo { int x; int y; };
double pesi_nas[numerin][nas], pesi_out[nas][nout], bias_nas[nas], bias_out[nout];

double relu(double x) { return x > 0 ? x : 0.01 * x; }

void carica_pesi() {
    FILE *f = fopen("pesi_snake.bin", "rb");
    if (!f) exit(1);
    fread(pesi_nas, sizeof(double), numerin * nas, f);
    fread(pesi_out, sizeof(double), nas * nout, f);
    fread(bias_nas, sizeof(double), nas, f);
    fread(bias_out, sizeof(double), nout, f);
    fclose(f);
}

void calcola_input_8_dir(double inpu[], struct corpo serpe[], int lung, struct corpo mela) {
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1}, dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    for (int i = 0; i < 8; i++) {
        int cx = serpe[0].x, cy = serpe[0].y;
        double dist = 0, mela_t = 0, corpo_t = 0;
        while (true) {
            cx += dx[i]; cy += dy[i]; dist++;
            if (cx < 0 || cx >= GRIGLIA_X || cy < 0 || cy >= GRIGLIA_Y) break;
            if (cx == mela.x && cy == mela.y) mela_t = 1.0;
            for (int j = 1; j < lung; j++) if (cx == serpe[j].x && cy == serpe[j].y && corpo_t == 0) corpo_t = 1.0 / dist;
        }
        inpu[i * 3] = 1.0 / dist; inpu[i * 3 + 1] = mela_t; inpu[i * 3 + 2] = corpo_t;
    }
}

void passo(double inp[], double out[]) {
    double hid[nas];
    for (int r = 0; r < nas; r++) {
        double s = bias_nas[r];
        for (int l = 0; l < numerin; l++) s += inp[l] * pesi_nas[l][r];
        hid[r] = relu(s);
    }
    for (int r = 0; r < nout; r++) {
        double s = bias_out[r];
        for (int l = 0; l < nas; l++) s += hid[l] * pesi_out[l][r];
        out[r] = s;
    }
}

int main(void) {
    InitWindow(GRA, ALT, "SNAKE AI");
    carica_pesi();
    SetTargetFPS(60);
    struct corpo serpe[280];
    serpe[0] = (struct corpo){10, 7}; serpe[1] = (struct corpo){9, 7}; serpe[2] = (struct corpo){8, 7};
    struct corpo mela = {GetRandomValue(0, GRIGLIA_X - 1), GetRandomValue(0, GRIGLIA_Y - 1)};
    int lunghezza = 3, dir = 1, con = 0;
    bool modalita_ia = true;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) modalita_ia = !modalita_ia;
        if (!modalita_ia) {
            if (IsKeyPressed(KEY_UP) && dir != 2) dir = 0;
            if (IsKeyPressed(KEY_RIGHT) && dir != 3) dir = 1;
            if (IsKeyPressed(KEY_DOWN) && dir != 0) dir = 2;
            if (IsKeyPressed(KEY_LEFT) && dir != 1) dir = 3;
        }
        con++;
        if (con % 6 == 0) {
            if (modalita_ia) {
                double in[numerin], out[nout];
                calcola_input_8_dir(in, serpe, lunghezza, mela);
                passo(in, out);
                int az = (out[0] > out[1] && out[0] > out[2]) ? 0 : (out[1] > out[2] ? 1 : 2);
                if (az == 0) dir = (dir + 3) % 4; else if (az == 2) dir = (dir + 1) % 4;
            }
            for (int i = lunghezza - 1; i > 0; i--) serpe[i] = serpe[i - 1];
            if (dir == 0) serpe[0].y--; else if (dir == 1) serpe[0].x++; else if (dir == 2) serpe[0].y++; else serpe[0].x--;
            bool morto = (serpe[0].x < 0 || serpe[0].x >= GRIGLIA_X || serpe[0].y < 0 || serpe[0].y >= GRIGLIA_Y);
            for (int i = 1; i < lunghezza; i++) if (serpe[0].x == serpe[i].x && serpe[0].y == serpe[i].y) morto = true;
            if (morto) {
                lunghezza = 3; dir = 1;
                serpe[0] = (struct corpo){10, 7}; serpe[1] = (struct corpo){9, 7}; serpe[2] = (struct corpo){8, 7};
            }
            if (serpe[0].x == mela.x && serpe[0].y == mela.y) {
                lunghezza++;
                mela = (struct corpo){GetRandomValue(0, GRIGLIA_X - 1), GetRandomValue(0, GRIGLIA_Y - 1)};
            }
        }
        BeginDrawing();
            ClearBackground((Color){18, 18, 24, 255});
            for (int x = 0; x <= GRIGLIA_X; x++) DrawLine(OFFSET_X + x * BLOCCO, OFFSET_Y, OFFSET_X + x * BLOCCO, OFFSET_Y + GRIGLIA_Y * BLOCCO, (Color){35, 35, 45, 255});
            for (int y = 0; y <= GRIGLIA_Y; y++) DrawLine(OFFSET_X, OFFSET_Y + y * BLOCCO, OFFSET_X + GRIGLIA_X * BLOCCO, OFFSET_Y + y * BLOCCO, (Color){35, 35, 45, 255});
            DrawRectangleLinesEx((Rectangle){OFFSET_X - 5, OFFSET_Y - 5, GRIGLIA_X * BLOCCO + 10, GRIGLIA_Y * BLOCCO + 10}, 3, DARKGRAY);
            DrawCircleGradient(mela.x * BLOCCO + OFFSET_X + BLOCCO/2, mela.y * BLOCCO + OFFSET_Y + BLOCCO/2, BLOCCO/1.1, (Color){255, 0, 0, 80}, BLANK);
            DrawCircle(mela.x * BLOCCO + OFFSET_X + BLOCCO/2, mela.y * BLOCCO + OFFSET_Y + BLOCCO/2, BLOCCO/2 - 3, RED);
            for (int i = 0; i < lunghezza; i++) DrawRectangleRounded((Rectangle){serpe[i].x * BLOCCO + OFFSET_X + 2, serpe[i].y * BLOCCO + OFFSET_Y + 2, BLOCCO - 4, BLOCCO - 4}, 0.45, 6, i == 0 ? LIME : GREEN);
            DrawText(TextFormat("SCORE: %d", lunghezza - 3), 20, 20, 20, RAYWHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}