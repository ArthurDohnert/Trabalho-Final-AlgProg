#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ALTURA 600
#define LARGURA 1200
#define X_INICIAL 300
#define Y_INICIAL 300
#define VELOCIDADE 3
#define TAMANHO_QUADRADO 20

void movimentacao(int *pX, int *pY);

int main(void)
{
    int posX = X_INICIAL, posY = Y_INICIAL;
    int quadradoNaTela = 1;
    int *pX, *pY;

    pX = &posX;
    pY = &posY;

    InitWindow(LARGURA, ALTURA, "Infmon");
    SetTargetFPS(60);

    srand(time(NULL));

    while (!WindowShouldClose() && quadradoNaTela)
    {
        movimentacao(pX, pY);

        if (posX == 0 - TAMANHO_QUADRADO || posY == 0 - TAMANHO_QUADRADO || posX == LARGURA || posY == ALTURA)
            quadradoNaTela = 0;

        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawRectangle(posX, posY, TAMANHO_QUADRADO, TAMANHO_QUADRADO, GREEN);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void movimentacao(int *pX, int *pY)
{
    if (IsKeyDown(KEY_RIGHT))
        *pX += VELOCIDADE;
    if (IsKeyDown(KEY_LEFT))
        *pX -= VELOCIDADE;
    if (IsKeyDown(KEY_UP))
        *pY -= VELOCIDADE;
    if (IsKeyDown(KEY_DOWN))
        *pY += VELOCIDADE;
}