#include "raylib.h"
#include <stdio.h>
#include <time.h>
#define SCREEN_HEIGHT 960
#define SCREEN_WIDTH 1920
#define STARTING_POSITION_X 300
#define STARTING_POSITION_Y 300
#define MOVEMENT_SPEED 5
#define SQUARE_WIDTH 32
#define ENTITY_SIZE 32
#define TRUE 1
#define FALSE 0

// ESTRUTURAS
//*********************************************************************************************************************
// Estrutura que define as propriedades de uma entidade
typedef struct
{
    int posX;
    int posY;
    Texture2D texture;
} Entity;

// Estrutura do mapa da grama
typedef struct
{
    Texture2D floor;
    Texture2D bush;
} Maps;
//*********************************************************************************************************************

// PROTÓTIPOS DE FUNÇÕES
//*********************************************************************************************************************
// Função que desenha o mapa
void drawMap(Maps *m, Texture2D wall, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH]);

// Função que atualiza a posição do personagem
void movePlayer(int *pX, int *pY, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH]);
//*********************************************************************************************************************

// MAIN
//=====================================================================================================================
int main(void)
{
    // Definindo estruturas e suas variáveis
    //--------------------------------------------------------------------------------------------------------------------
    Maps grass;
    Entity player;
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

    // Define texturas
    //--------------------------------------------------------------------------------------------------------------------
    Texture2D wallTexture = LoadTexture("sprites/Wall(AllMaps).png");
    grass.floor = LoadTexture("sprites/TerrenoDeGrama.png");
    grass.bush = LoadTexture("sprites/TerrenoDeTransicaoGrama.png");
    player.texture = LoadTexture("sprites/MainChar.png");

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
            drawMap(&grass, wallTexture, map);
            ClearBackground(RAYWHITE);
            DrawTexture(player.texture, player.posX, player.posY, WHITE);
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

    UnloadTexture(player.texture);
    UnloadTexture(grass.floor);
    UnloadTexture(grass.bush);
    UnloadTexture(wallTexture);

    // #####################################################################################################################
    CloseWindow(); // Fecha o jogo
    return 0;
}

//=====================================================================================================================
// FUNÇÕES CUSTOMIZADAS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawMap(Maps *m, Texture2D wall, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH])
{
    int i, j;
    for (i = 0; i < SCREEN_WIDTH / SQUARE_WIDTH; i++)
    {
        for (j = 0; j < SCREEN_HEIGHT / SQUARE_WIDTH; j++)
        {
            switch (map[i][j])
            {
            case 'W':
                DrawTexture(wall, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;
            case ' ':
                DrawTexture(m->floor, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;
            case 'G':
                DrawTexture(m->floor, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE); // TODO: corrigir bug e mudar de 'floor' para 'bush'
                break;
            }
        }
    }
}

void movePlayer(int *pX, int *pY, char map[SCREEN_WIDTH / SQUARE_WIDTH][SCREEN_HEIGHT / SQUARE_WIDTH])
{
    if (IsKeyDown(KEY_RIGHT) && map[(*pX + ENTITY_SIZE) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
        *pX += MOVEMENT_SPEED;
    if (IsKeyDown(KEY_LEFT) && map[(*pX - MOVEMENT_SPEED) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
        *pX -= MOVEMENT_SPEED;
    if (IsKeyDown(KEY_UP) && map[*pX / SQUARE_WIDTH][(*pY - MOVEMENT_SPEED) / SQUARE_WIDTH] != 'W')
        *pY -= MOVEMENT_SPEED;
    if (IsKeyDown(KEY_DOWN) && map[*pX / SQUARE_WIDTH][(*pY + ENTITY_SIZE) / SQUARE_WIDTH] != 'W')
        *pY += MOVEMENT_SPEED;
}