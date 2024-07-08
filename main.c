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
#define TAMANHO_OBSTACULO 20

void movimentacao(int *pX, int *pY);

int main(void)
{
    int posX = X_INICIAL, posY = Y_INICIAL;
    int i, j;
    int quadradoNaTela = 1;
    int *pX, *pY;

    pX = &posX;
    pY = &posY;

    InitWindow(LARGURA, ALTURA, "Quadrado");
    SetTargetFPS(60);

    srand(time(NULL));

    while (!WindowShouldClose() && quadradoNaTela)
    {
        movimentacao(pX, pY);
        if (posX == 0 - TAMANHO_QUADRADO || posY == 0 - TAMANHO_QUADRADO || posX == ALTURA || posY == LARGURA)
            quadradoNaTela = 0;

        BeginDrawing();

        for (i = 0; i < 30; i++)
        {
            for (j = 0; j < 30; j++)
            {
            }
        }

        ClearBackground(RAYWHITE);
        DrawRectangle(posX, posY, TAMANHO_QUADRADO, TAMANHO_QUADRADO, GREEN);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void movimentacao(int *pX, int *pY)
{
    if (IsKeyPressed(KEY_RIGHT))
        *pX += VELOCIDADE;
    if (IsKeyPressed(KEY_LEFT))
        *pX -= VELOCIDADE;
    if (IsKeyPressed(KEY_UP))
        *pY -= VELOCIDADE;
    if (IsKeyPressed(KEY_DOWN))
        *pY += VELOCIDADE;
}