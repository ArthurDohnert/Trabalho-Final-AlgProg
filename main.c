#include "raylib.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#define SCREEN_HEIGHT 960
#define SCREEN_WIDTH 1920
#define STARTING_POSITION_X 300
#define STARTING_POSITION_Y 300
#define MOVEMENT_SPEED 3
#define SQUARE_WIDTH 32
#define ENTITY_SIZE 32
#define ROWS 30
#define COLUMNS 60
#define MAX_INFMONS 3
#define TRUE 1
#define FALSE 0

// ESTRUTURAS
//*********************************************************************************************************************
// Estrutura do infmon
typedef struct
{
    int current_health_value;
    int max_health;
    char infmon_type;
    int level;
    int current_xp;
    int level_up_xp_threshold;
} Infmon;

// Estrutura que define as propriedades de uma entidade
typedef struct
{
    int posX;
    int posY;
    Infmon mon[MAX_INFMONS];
    Texture2D texture;
} Entity;

// Estrutura dos mapas (podemos declarar um novo mapa e reutilizar as texturas com base nos mapas obtidos   )
typedef struct
{
    Texture2D floor;
    Texture2D bush;
    Texture2D wall;
} Maps;

// guarda todas as informaçoes do estado do jogo
typedef struct
{
    int game_is_paused; // Determina se o jogo está pausado
    int exit_requested;
    int must_exit;      // Determina se o jogador quer fechar o jogo
    int game_situation; // 0: main menu, 1: exploração, 2: combate

} GameInfo;

// acumula todas as informações que serao passadas pro arquivo de save
typedef struct
{
    Entity Player_info;
    int mapNum;

} SaveInfo;

//*********************************************************************************************************************
// PROTÓTIPOS DE FUNÇÕES
//*********************************************************************************************************************

// recarrega o mapa diferente na matriz map
void changeMap(char map[COLUMNS][ROWS], int numMap, Entity *player);

// confirma se o player realmente quer sair
void confirmExit(GameInfo *game);

// faz o menu principal
void mainMenu(GameInfo *game);

// controla a pause
void pauseGame(GameInfo *game);

// exploracao
void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[COLUMNS][ROWS]);

// combate
void combat(GameInfo *game);

// Função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS]);

// função pra salvar o jogo em save.bin
int saveGame(SaveInfo *data, Entity player, int numMap);

// Função que determina se jogador está passando por zona de encontro aleatório
int movingThroughGrass(int pX, int pY, char map[COLUMNS][ROWS]);

// Função que gera encontro aleatório
int randomEncounter();

// Função que atualiza a posição do personagem
int movePlayer(int *pX, int *pY, char map[COLUMNS][ROWS]);
//*********************************************************************************************************************

// MAIN
//=====================================================================================================================
int main(void)
{
    // Definindo estruturas e suas variáveis
    //--------------------------------------------------------------------------------------------------------------------
    GameInfo game;
    Maps grass;
    Entity player;
    SaveInfo saveData;
    player.posX = STARTING_POSITION_X;
    player.posY = STARTING_POSITION_Y;

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------

    game.game_is_paused = FALSE;
    game.exit_requested = FALSE;
    game.must_exit = FALSE;
    game.game_situation = 0; // inicia na tela de menu

    //---------------------------------------------------------------------------------------------------------------------

    // VARIÁVEIS
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int mapNum = 3;
    char map[COLUMNS][ROWS]; // Gerador de mapa provisório

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Infmon");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // Define texturas
    //--------------------------------------------------------------------------------------------------------------------
    grass.wall = LoadTexture("sprites/Wall(AllMaps).png");
    grass.bush = LoadTexture("sprites/TerrenoDeGrama.png");
    grass.floor = LoadTexture("sprites/TerrenoDeTransicaoGrama.png");
    player.texture = LoadTexture("sprites/MainChar.png");

    // Gera o mapa
    //---------------------------------------------------------------------------------------------------------------------
    SetRandomSeed(time(NULL));
    changeMap(map, mapNum, &player);
    //---------------------------------------------------------------------------------------------------------------------

    // Vínculo principal do jogo
    // #####################################################################################################################
    while (!(WindowShouldClose() || game.must_exit))
    {
        //---------------------------------------------------------------------------------------------------------------------
        // Pergunta se o jogador quer sair. Precisa ter preferência na sequência de ifs
        //---------------------------------------------------------------------------------------------------------------------
        if (game.exit_requested)
        {
            confirmExit(&game);
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Main menu
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == 0)
        {
            mainMenu(&game);
        }

        //--------------------------------------------------------------------------------------------------------------------
        // Jogo pausado
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_is_paused)
        {
            pauseGame(&game);
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Exploração
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == 1)
        {
            exploring(&game, &player, &grass, map);
        }

        //--------------------------------------------------------------------------------------------------------------------
        //    Em combate
        //--------------------------------------------------------------------------------------------------------------------

        else if (game.game_situation == 2)
        {
            combat(&game);
        }
    }
    saveGame(&saveData, player, mapNum);

    UnloadTexture(player.texture);
    UnloadTexture(grass.floor);
    UnloadTexture(grass.bush);
    UnloadTexture(grass.wall);

    // #####################################################################################################################
    CloseWindow(); // Fecha o jogo
    return 0;
}

//=====================================================================================================================
// FUNÇÕES CUSTOMIZADAS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// recarrega o mapa diferente na matriz map
void changeMap(char map[COLUMNS][ROWS], int numMap, Entity *player)
{
    int i, j;
    int transpose[ROWS][COLUMNS];
    char numStr[5];
    char fileName[20] = "maps/Mapa";
    FILE *arqMap;

    sprintf(numStr, "%d", numMap);

    strcat(fileName, numStr);
    strcat(fileName, ".txt");

    arqMap = fopen(fileName, "r");

    if (arqMap == NULL)
    {
    }
    else
    {
        for (i = 0; i < ROWS; i++)
        {
            for (j = 0; j < COLUMNS; j++)
            {
                if ((transpose[i][j] = getc(arqMap)) == '\n')
                    j--;

                map[j][i] = transpose[i][j];

                if (map[j][i] == 'J')
                {
                    player->posX = j * ENTITY_SIZE;
                    player->posY = i * ENTITY_SIZE;
                }
            }
        }
    }

    fclose(arqMap);
}

// confirma se o player realmente quer sair
void confirmExit(GameInfo *game)
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        game->exit_requested = FALSE;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_Q))
    {
        game->must_exit = TRUE;
        game->exit_requested = FALSE;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 100, SCREEN_WIDTH, SCREEN_HEIGHT - 200, BLACK);
    DrawText("Você deseja sair sem salvar?", 80, 180, 30, WHITE);
    DrawText("Pressione enter para sair", 80, 300, 30, WHITE);
    EndDrawing();
}

// faz o menu principal
void mainMenu(GameInfo *game)
{
    if (IsKeyPressed(KEY_C))
    {
        // TODO: loadGame()
        game->game_situation = 1;
    }
    if (IsKeyPressed(KEY_N))
    {
        // TODO: newGame()
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
    {
        game->exit_requested = TRUE;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    DrawText("INFMON  !", SCREEN_WIDTH / 2 - 140, 180, 50, WHITE);
    DrawText("-1", SCREEN_WIDTH / 2 + 60, 165, 35, WHITE);
    DrawText("Pressione C para carregar seu jogo", SCREEN_WIDTH / 2 - 500, 300, 50, WHITE);
    DrawText("Pressione N para criar um novo jogo", SCREEN_WIDTH / 2 - 500, 400, 50, WHITE);
    DrawText("Pressione Q para fechar o jogo", SCREEN_WIDTH / 2 - 450, 500, 50, WHITE);

    EndDrawing();
}

// controla o pause
void pauseGame(GameInfo *game)
{
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_C))
    {
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
    {
        game->exit_requested = TRUE;
    }
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("PAUSED", 400, 300, 50, WHITE);
    EndDrawing();
}

// exploracao
void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[COLUMNS][ROWS])
{
    if (IsKeyPressed(KEY_TAB))
    {
        game->game_is_paused = TRUE;
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        game->exit_requested = TRUE;
    }
    if (movePlayer(&player->posX, &player->posY, map))
    {
        if (movingThroughGrass(player->posX, player->posY, map))
        {
            if (randomEncounter())
            {
                game->game_situation = 2;
            }
        }
    }

    movePlayer(&player->posX, &player->posY, map);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    drawMap(mapa, map);
    DrawTexture(player->texture, player->posX, player->posY, WHITE);
    DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
    EndDrawing();
}

// combate
void combat(GameInfo *game)
{
    if (IsKeyPressed(KEY_R))
    {
        game->game_situation = 1;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("COMBAT", 400, 300, 50, WHITE);
    EndDrawing();
}

// função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS])
{
    int i, j;
    for (i = 0; i < COLUMNS; i++)
    {
        for (j = 0; j < ROWS; j++)
        {
            switch (map[i][j])
            {
            case 'W':
                DrawTexture(m->wall, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;
            case ' ':
            case 'J':
            case 'E':
                DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                break;
            case 'G':
                DrawTexture(m->bush, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;
            }
        }
    }
}

int saveGame(SaveInfo *data, Entity player, int numMap)
{
    FILE *saveArq;

    data->mapNum = numMap;
    data->Player_info = player;

    saveArq = fopen("save.bin", "w+");

    if (saveArq == NULL)
    {
        return 1;
    }
    else
    {
        fwrite(data, sizeof(*data), 1, saveArq);
        fclose(saveArq);
    }
    return 0;
}

int randomEncounter()
{
    int battle_start = FALSE;

    if (GetRandomValue(1, 300) == 1)
        battle_start = TRUE;

    return battle_start;
}

int movingThroughGrass(int pX, int pY, char map[COLUMNS][ROWS])
{

    int in_grass = FALSE;

    if (map[(pX / SQUARE_WIDTH)][(pY / SQUARE_WIDTH)] == 'G')
        in_grass = TRUE;

    return in_grass;
}

int movePlayer(int *pX, int *pY, char map[COLUMNS][ROWS])
{
    int moving = FALSE;
    if (IsKeyDown(KEY_RIGHT) && map[(*pX + ENTITY_SIZE) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
    {
        *pX += MOVEMENT_SPEED;
        moving = TRUE;
    }
    if (IsKeyDown(KEY_LEFT) && map[(*pX - MOVEMENT_SPEED) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
    {
        *pX -= MOVEMENT_SPEED;
        moving = TRUE;
    }
    if (IsKeyDown(KEY_UP) && map[*pX / SQUARE_WIDTH][(*pY - MOVEMENT_SPEED) / SQUARE_WIDTH] != 'W')
    {
        *pY -= MOVEMENT_SPEED;
        moving = TRUE;
    }
    if (IsKeyDown(KEY_DOWN) && map[*pX / SQUARE_WIDTH][(*pY + ENTITY_SIZE) / SQUARE_WIDTH] != 'W')
    {
        *pY += MOVEMENT_SPEED;
        moving = TRUE;
    }

    return moving;
}
