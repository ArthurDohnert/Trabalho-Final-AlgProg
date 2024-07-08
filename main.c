#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ALTURA 600
#define LARGURA 1200
#define X_INICIAL 300
#define Y_INICIAL 300
#define VELOCIDADE 20
#define TAMANHO_QUADRADO 20

void movimentacao(int *pX, int *pY, char mapa[60][30]);

int main(void)
{
    int posX = X_INICIAL, posY = Y_INICIAL;
    int i, j;
    int *pX, *pY;
    char mapa[60][30];

    pX = &posX;
    pY = &posY;

    InitWindow(LARGURA, ALTURA, "Infmon");
    SetTargetFPS(60);
    srand(time(NULL));

    for (i = 0; i < 60; i++)
    {
        for (j = 0; j < 30; j++)
        {
            if (i == 0 || i == 59 || j == 0 || j == 29)
            {
                mapa[i][j] = 'W';
            }
            else if (rand() % 5 == 1)
            {
                mapa[i][j] = 'G';
            }
            else
            {
                mapa[i][j] = ' ';
            }
        }
    }

    while (!WindowShouldClose())
    {
        movimentacao(pX, pY, mapa);

        BeginDrawing();
        for (i = 0; i < 60; i++)
        {
            for (j = 0; j < 30; j++)
            {
                if (mapa[i][j] == 'G')
                {
                    DrawRectangle(i * 20, j * 20, 20, 20, DARKGREEN);
                }
                else if (mapa[i][j] == 'W')
                {
                    DrawRectangle(i * 20, j * 20, 20, 20, BLACK);
                }
                else if (mapa[i][j] == ' ')
                {
                    DrawRectangle(i * 20, j * 20, 20, 20, GREEN);
                }
            }
        }

        ClearBackground(RAYWHITE);
        DrawRectangle(posX, posY, TAMANHO_QUADRADO, TAMANHO_QUADRADO, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void movimentacao(int *pX, int *pY, char mapa[60][30])
{
    if (IsKeyPressed(KEY_RIGHT) && mapa[(*pX + VELOCIDADE) / 20][*pY / 20] != 'W')
        *pX += VELOCIDADE;
    if (IsKeyPressed(KEY_LEFT) && mapa[(*pX - VELOCIDADE) / 20][*pY / 20] != 'W')
        *pX -= VELOCIDADE;
    if (IsKeyPressed(KEY_UP) && mapa[*pX / 20][(*pY - VELOCIDADE) / 20] != 'W')
        *pY -= VELOCIDADE;
    if (IsKeyPressed(KEY_DOWN) && mapa[*pX / 20][(*pY + VELOCIDADE) / 20] != 'W')
        *pY += VELOCIDADE;
}