#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define inp 24
#define nas 128
#define out 3
#define GRIGLIA_X 20
#define GRIGLIA_Y 14

struct corpo { int x; int y; };

double pesinas[inp][nas], pesiout[nas][out], biasnas[nas], biasout[out];

double relu(double x) { return x > 0 ? x : 0.01 * x; }
double der_relu(double x) { return x > 0 ? 1.0 : 0.01; }
double iniz_pes() { return ((double)rand() / RAND_MAX) * 0.2 - 0.1; }

void calcola_input_8_dir(double inpu[], struct corpo serpe[], int lung, struct corpo mela) {
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    for (int i = 0; i < 8; i++) {
        int cx = serpe[0].x;
        int cy = serpe[0].y;
        double dist = 0;
        double mela_trovata = 0, corpo_trovato = 0;

        while (true) {
            cx += dx[i]; cy += dy[i];
            dist++;
            if (cx < 0 || cx >= GRIGLIA_X || cy < 0 || cy >= GRIGLIA_Y) break;
            if (cx == mela.x && cy == mela.y) mela_trovata = 1.0;
            for (int j = 1; j < lung; j++) {
                if (cx == serpe[j].x && cy == serpe[j].y && corpo_trovato == 0) 
                    corpo_trovato = 1.0 / dist;
            }
        }
        inpu[i*3] = 1.0 / dist;
        inpu[i*3+1] = mela_trovata;
        inpu[i*3+2] = corpo_trovato;
    }
}

void passo(double inpu[], double outp[], double attivazioni_nas[]) {
    for (int r = 0; r < nas; r++) {
        double somma = biasnas[r];
        for (int l = 0; l < inp; l++) somma += inpu[l] * pesinas[l][r];
        attivazioni_nas[r] = somma;
    }
    double output_nas[nas];
    for(int r=0; r<nas; r++) output_nas[r] = relu(attivazioni_nas[r]);
    for (int r = 0; r < out; r++) {
        double somma = biasout[r];
        for (int l = 0; l < nas; l++) somma += output_nas[l] * pesiout[l][r];
        outp[r] = somma; 
    }
}

int main() {
    srand(time(NULL));
    for (int i = 0; i < inp; i++) for (int j = 0; j < nas; j++) pesinas[i][j] = iniz_pes();
    for (int i = 0; i < nas; i++) for (int j = 0; j < out; j++) pesiout[i][j] = iniz_pes();
    for (int i = 0; i < nas; i++) biasnas[i] = iniz_pes();
    for (int i = 0; i < out; i++) biasout[i] = iniz_pes();

    int episodi = 300000;
    double lr_base = 0.0003;
    struct corpo serpe[280];

    for (int i = 0; i < episodi; i++) {
        double lr = lr_base* (1.0 - (double)i / episodi);
        struct corpo mela = {rand()%GRIGLIA_X, rand()%GRIGLIA_Y};
        int lung = 3, dir = 1, passi_stallo = 0, score = 0;
        serpe[0] = (struct corpo){10,7}; serpe[1] = (struct corpo){9,7}; serpe[2] = (struct corpo){8,7};
        bool morto = false;

        double epsilon = fmax(0.01, 1.0 - (double)i / (episodi * 0.85));

        while (!morto && passi_stallo < 200) {
            passi_stallo++;
            double input[inp], output[out], attivazioni_nas[nas];
            calcola_input_8_dir(input, serpe, lung, mela);
            passo(input, output, attivazioni_nas);

            int az = ((double)rand()/RAND_MAX < epsilon) ? rand()%3 : (output[0]>output[1] && output[0]>output[2] ? 0 : (output[1]>output[2] ? 1 : 2));
            if (az == 0) dir = (dir + 3) % 4; else if (az == 2) dir = (dir + 1) % 4;

            struct corpo vecchia_testa = serpe[0];
            for (int j = lung - 1; j > 0; j--) serpe[j] = serpe[j - 1];
            if(dir==0) serpe[0].y--; else if(dir==1) serpe[0].x++; else if(dir==2) serpe[0].y++; else serpe[0].x--;

            if (serpe[0].x < 0 || serpe[0].x >= GRIGLIA_X || serpe[0].y < 0 || serpe[0].y >= GRIGLIA_Y) morto = true;
            for (int j = 1; j < lung; j++) if (serpe[0].x == serpe[j].x && serpe[0].y == serpe[j].y) morto = true;

            double reward = -0.25;
            if (morto) reward = -25.0;
            else if (serpe[0].x == mela.x && serpe[0].y == mela.y) {
                lung++; score++; reward = 65.0; passi_stallo = 0;
                mela = (struct corpo){rand()%GRIGLIA_X, rand()%GRIGLIA_Y};
            } else {
                double d_vec = abs(vecchia_testa.x - mela.x) + abs(vecchia_testa.y - mela.y);
                double d_nuo = abs(serpe[0].x - mela.x) + abs(serpe[0].y - mela.y);
                if (d_nuo < d_vec) reward = 2;
                else {
                    reward = -1.5; 
                }
            }

            double input_f[inp], output_f[out], dumm[nas];
            calcola_input_8_dir(input_f, serpe, lung, mela);
            passo(input_f, output_f, dumm);
            double max_f = fmax(output_f[0], fmax(output_f[1], output_f[2]));
            double target = morto ? reward : reward + 0.95 * max_f;

            double deltaout[out];
            for(int j=0; j<out; j++) deltaout[j] = (j == az) ? (target - output[j]) : 0;
            
            for (int j = 0; j < nas; j++) {
                double err = 0;
                for (int k = 0; k < out; k++) err += deltaout[k] * pesiout[j][k];
                double d_nas = err * der_relu(attivazioni_nas[j]);
                biasnas[j] += d_nas * lr;
                for (int k = 0; k < inp; k++) pesinas[k][j] += input[k] * d_nas * lr;
            }
            for (int j = 0; j < out; j++) {
                biasout[j] += deltaout[j] * lr;
                for (int k = 0; k < nas; k++) pesiout[k][j] += relu(attivazioni_nas[k]) * deltaout[j] * lr;
            }
        }
        if (i % 10000 == 0) printf("Ep: %d | Sc: %d | Eps: %.2f\n", i, score, epsilon);
    }

    FILE *f = fopen("pesi_snake.bin", "wb");
    fwrite(pesinas, sizeof(double), inp*nas, f);
    fwrite(pesiout, sizeof(double), nas*out, f);
    fwrite(biasnas, sizeof(double), nas, f);
    fwrite(biasout, sizeof(double), out, f);
    fclose(f);
    return 0;
}