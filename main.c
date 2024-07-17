#include "raylib.h"
#include <stdio.h>
#include <time.h>
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 1200
#define STARTING_POSITION_X 300
#define STARTING_POSITION_Y 300
#define MOVEMENT_SPEED 5
#define SQUARE_WIDTH 20
#define TRUE 1
#define FALSE 0

// PROTÓTIPOS DE FUNÇÕES
//*********************************************************************************************************************
// Função que desenha o mapa
void drawMap(char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH]);

// Função que atualiza a posição do personagem
void movePlayer(int *pX, int *pY, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH]);
//*********************************************************************************************************************

// ESTRUTURAS
//*********************************************************************************************************************
// Estrutura que define as propriedades de uma entidade
struct entity
{
    int posX;
    int posY;
};
//*********************************************************************************************************************

// MAIN
//=====================================================================================================================
int main(void)
{
    // Definindo estruturas e suas variáveis
    //--------------------------------------------------------------------------------------------------------------------

    struct entity player;
    player.posX = STARTING_POSITION_X;
    player.posY = STARTING_POSITION_Y;

    //---------------------------------------------------------------------------------------------------------------------

    // VARIÁVEIS
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int i, j;
    char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH]; // Gerador de mapa provisório

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------

    int exploring_map = TRUE;   // Determina se o jogador pode se mover pelo mapa
    int game_is_paused = FALSE; // Determina se o jogo está pausado

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Infmon");
    SetTargetFPS(60);

    // Gera o mapa
    //---------------------------------------------------------------------------------------------------------------------
    SetRandomSeed(time(NULL));
    for (i = 0; i < SCREEN_WIDTH / SQUARE_WIDTH; i++)
    {
        for (j = 0; j < SCREEN_HEIGHT / SQUARE_WIDTH; j++)
        {
            if (i == 0 || i == SCREEN_WIDTH / SQUARE_WIDTH - 1 || j == 0 || j == SCREEN_HEIGHT / SQUARE_WIDTH - 1)
                map[i][j] = 'W';
            else if (GetRandomValue(1, 5) == 1)
                map[i][j] = 'G';
            else
                map[i][j] = ' ';
        }
    }
    //---------------------------------------------------------------------------------------------------------------------

    // Vínculo principal do jogo
    // #####################################################################################################################
    while (!WindowShouldClose())
    {
        // Exploração
        //---------------------------------------------------------------------------------------------------------------------
        if (exploring_map)
        {
            if (IsKeyPressed(KEY_TAB))
            {
                game_is_paused = TRUE;
                exploring_map = FALSE;
            }
            movePlayer(&player.posX, &player.posY, map);
            BeginDrawing();
            drawMap(map);
            ClearBackground(RAYWHITE);
            DrawRectangle(player.posX, player.posY, SQUARE_WIDTH, SQUARE_WIDTH, RED);
            DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
            EndDrawing();
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Jogo pausado
        //---------------------------------------------------------------------------------------------------------------------

        else if (game_is_paused)
        {
            if (IsKeyPressed(KEY_TAB))
            {
                game_is_paused = FALSE;
                exploring_map = TRUE;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("PAUSED", 400, 300, 50, WHITE);
            EndDrawing();
        }
        //---------------------------------------------------------------------------------------------------------------------
    }
    // #####################################################################################################################
    CloseWindow(); // Fecha o jogo
    return 0;
}

//=====================================================================================================================
// FUNÇÕES CUSTOMIZADAS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawMap(char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH])
{
    int i, j;
    for (i = 0; i < SCREEN_WIDTH / SQUARE_WIDTH; i++)
    {
        for (j = 0; j < SCREEN_HEIGHT / SQUARE_WIDTH; j++)
        {
            if (map[i][j] == 'G')
                DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, DARKGREEN);
            else if (map[i][j] == 'W')
                DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, BLACK);
            else if (map[i][j] == ' ')
                DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
        }
    }
}

void movePlayer(int *pX, int *pY, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH])
{
    if (IsKeyDown(KEY_RIGHT) && map[(*pX + SQUARE_WIDTH) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
        *pX += MOVEMENT_SPEED;
    if (IsKeyDown(KEY_LEFT) && map[(*pX - MOVEMENT_SPEED) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
        *pX -= MOVEMENT_SPEED;
    if (IsKeyDown(KEY_UP) && map[*pX / SQUARE_WIDTH][(*pY - MOVEMENT_SPEED) / SQUARE_WIDTH] != 'W')
        *pY -= MOVEMENT_SPEED;
    if (IsKeyDown(KEY_DOWN) && map[*pX / SQUARE_WIDTH][(*pY + SQUARE_WIDTH) / SQUARE_WIDTH] != 'W')
        *pY += MOVEMENT_SPEED;
}