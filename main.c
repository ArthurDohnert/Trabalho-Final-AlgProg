#include "raylib.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#define SCREEN_HEIGHT 960
#define SCREEN_WIDTH 1920
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
    int current_game_saved;
    int randomInfmon;

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
void loadMap(char map[COLUMNS][ROWS], int numMap, Entity *player, int alteraPlayerPos);

// confirma se o player realmente quer sair
int confirmExit(GameInfo *game, int *choose);

// faz o menu principal
int mainMenu(GameInfo *game, int *choose);

// controla a pause
void pauseGame(GameInfo *game, SaveInfo *data, Entity *player, int *numMap, int *choose, char map[COLUMNS][ROWS]);

// exploracao
void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[COLUMNS][ROWS]);

// combate
void combat(GameInfo *game, Infmon *enemy);

// Função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS]);

// função pra carregar o jogo em save.bin
int loadGame(SaveInfo *data, Entity *player, int *numMap);

// função pra salvar o jogo em save.bin
int saveGame(SaveInfo *data, Entity player, int numMap);

// função pra criar novo jogo em save.bin
int newGame(SaveInfo *data);

// Função que determina se jogador está passando por zona de encontro aleatório
int movingThroughGrass(int pX, int pY, char map[COLUMNS][ROWS]);

// Função que gera encontro aleatório
int randomEncounter();

Infmon generateRandomInfmon();

// Função que atualiza a posição do personagem
int movePlayer(GameInfo *game, int *pX, int *pY, char map[COLUMNS][ROWS]);
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

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------

    game.game_is_paused = FALSE;
    game.exit_requested = FALSE;
    game.must_exit = FALSE;
    game.randomInfmon = FALSE;
    game.game_situation = 0; // inicia na tela de menu

    //---------------------------------------------------------------------------------------------------------------------

    // VARIÁVEIS
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int menuChoose = 0, mapNum = 1;
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
    loadMap(map, mapNum, &player, 1);
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
            if (confirmExit(&game, &menuChoose))
            {
                saveGame(&saveData, player, mapNum);
            }
        }

        //--------------------------------------------------------------------------------------------------------------------
        // Jogo pausado
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_is_paused)
        {
            pauseGame(&game, &saveData, &player, &mapNum, &menuChoose, map);
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Main menu
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == 0)
        {
            switch (mainMenu(&game, &menuChoose))
            {
            case 1:
                loadGame(&saveData, &player, &mapNum);
                break;

            case 2:
                if (!newGame(&saveData))
                {
                    loadMap(map, mapNum, &player, 1);
                }
                break;
            }
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
            if (game.randomInfmon)
            {
                combat(&game, generateRandomInfmon());
            }
            else
            {
            }
        }
    }

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
void loadMap(char map[COLUMNS][ROWS], int numMap, Entity *player, int alteraPlayerPos)
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

                if (alteraPlayerPos && map[j][i] == 'J')
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
int confirmExit(GameInfo *game, int *choose)
{
    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
        {
            *choose = 2;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 2)
        {
            *choose = 0;
        }
        else
        {
            *choose += 1;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 0))
    {
        game->exit_requested = FALSE;
    }
    if (IsKeyPressed(KEY_Q) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        game->must_exit = TRUE;
        game->exit_requested = FALSE;
        return 1;
    }
    if (IsKeyPressed(KEY_ENTER) && *choose == 2)
    {
        game->must_exit = TRUE;
        game->exit_requested = FALSE;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

    if (game->current_game_saved)
    {
        DrawText("Jogo salvo!", SCREEN_WIDTH - 200, 800, 20, WHITE);
    }

    // desenha o quadrado de escolha
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 450 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * *choose, 10, 160, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 + 450, 300 + 150 * *choose, 10, 160, WHITE);

    DrawText("VOCÊ REALMENTE DESEJA SAIR?", SCREEN_WIDTH / 2 - 435, 180, 50, WHITE);
    DrawText("Voltar ao jogo", SCREEN_WIDTH / 2 - 200, 350, 50, WHITE);
    DrawText("Salvar e sair", SCREEN_WIDTH / 2 - 185, 500, 50, WHITE);
    DrawText("Sair sem salvar", SCREEN_WIDTH / 2 - 205, 650, 50, WHITE);
    EndDrawing();

    return 0;
}

// faz o menu principal
int mainMenu(GameInfo *game, int *choose)
{
    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
        {
            *choose = 2;
        }
        else
        {
            *choose -= 1;
        }
    }

    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 2)
        {
            *choose = 0;
        }
        else
        {
            *choose += 1;
        }
    }

    if (IsKeyPressed(KEY_C) || (IsKeyPressed(KEY_ENTER) && *choose == 0))
    {
        game->game_situation = 1;
        return 1;
    }
    if (IsKeyPressed(KEY_N) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        game->game_situation = 1;
        *choose = 0;
        return 2;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 2))
    {
        game->exit_requested = TRUE;
        *choose = 0;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

    // desenha o quadrado de escolha
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 300 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 450 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 300 + 150 * *choose, 10, 160, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 + 460, 300 + 150 * *choose, 10, 160, WHITE);

    // escreve os textos de opcoes
    DrawText("INFMON  !", SCREEN_WIDTH / 2 - 115, 180, 50, WHITE);
    DrawText("-1", SCREEN_WIDTH / 2 + 95, 165, 35, WHITE);
    DrawText("Carregar jogo (C)", SCREEN_WIDTH / 2 - 215, 350, 50, WHITE);
    DrawText("Criar novo jogo (N)", SCREEN_WIDTH / 2 - 235, 500, 50, WHITE);
    DrawText("Fechar o jogo (Q)", SCREEN_WIDTH / 2 - 215, 650, 50, WHITE);

    EndDrawing();

    return 0;
}

// controla o pause
void pauseGame(GameInfo *game, SaveInfo *data, Entity *player, int *numMap, int *choose, char map[COLUMNS][ROWS])
{
    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
        {
            *choose = 4;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 4)
        {
            *choose = 0;
        }
        else
        {
            *choose += 1;
        }
    }

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_C) || (IsKeyPressed(KEY_ENTER) && *choose == 0))
    {
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_L) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        loadGame(data, player, numMap);
        loadMap(map, *numMap, player, 0);
        *choose = 0;
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_S) || (IsKeyPressed(KEY_ENTER) && *choose == 2))
    {
        saveGame(data, *player, *numMap);
        game->current_game_saved = TRUE;
    }
    if (IsKeyPressed(KEY_B) || (IsKeyPressed(KEY_ENTER) && *choose == 3))
    {
        game->game_situation = 0;
        game->game_is_paused = FALSE;
        *choose = 0;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 4))
    {
        game->exit_requested = TRUE;
        *choose = 0;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("JOGO PAUSADO", SCREEN_WIDTH / 2 - 190, 180, 50, WHITE);

    if (game->current_game_saved)
    {
        DrawText("Jogo salvo!", SCREEN_WIDTH - 200, 800, 20, WHITE);
    }

    // desenha o quadrado de escolha
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * ((*choose) % 3), 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 450 + 150 * ((*choose) % 3), 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * ((*choose) % 3), 10, 160, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 + 450, 300 + 150 * ((*choose) % 3), 10, 160, WHITE);

    // escreve os textos de opcoes
    if (*choose <= 2)
    {
        DrawText("Continuar jogo (C)", SCREEN_WIDTH / 2 - 215, 350, 50, WHITE);
        DrawText("Carregar jogo (L)", SCREEN_WIDTH / 2 - 210, 500, 50, WHITE);
        DrawText("Salvar jogo (S)", SCREEN_WIDTH / 2 - 180, 650, 50, WHITE);
    }
    else
    {
        DrawText("Voltar ao menu (B)", SCREEN_WIDTH / 2 - 220, 350, 50, WHITE);
        DrawText("Sair (S)", SCREEN_WIDTH / 2 - 80, 500, 50, WHITE);
    }

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
    if (movePlayer(game, &player->posX, &player->posY, map))
    {
        if (movingThroughGrass(player->posX, player->posY, map))
        {
            if (randomEncounter())
            {
                game->game_situation = 2;
            }
        }
    }

    movePlayer(game, &player->posX, &player->posY, map);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    drawMap(mapa, map);
    DrawTexture(player->texture, player->posX, player->posY, WHITE);
    DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
    EndDrawing();
}

// combate
void combat(GameInfo *game, Infmon *enemy)
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

// função pra carregar o jogo em save.bin
int loadGame(SaveInfo *data, Entity *player, int *numMap)
{
    FILE *saveArq;

    saveArq = fopen("saves/save.bin", "r");

    if (saveArq == NULL)
    {
        return 1;
    }
    else
    {
        fread(data, sizeof(*data), 1, saveArq);
        fclose(saveArq);
    }

    *player = data->Player_info;
    *numMap = data->mapNum;

    return 0;
}

// função pra salvar o jogo em save.bin
int saveGame(SaveInfo *data, Entity player, int numMap)
{
    FILE *saveArq;

    // atualiza a estrutura com o save
    data->mapNum = numMap;
    data->Player_info = player;

    saveArq = fopen("saves/save.bin", "w");

    if (saveArq == NULL)
    {
        return 1;
    }
    else
    {
        // escreve e salva o arquivo com o save
        fwrite(data, sizeof(*data), 1, saveArq);
        fclose(saveArq);
    }
    return 0;
}

// função pra criar novo jogo em save.bin
int newGame(SaveInfo *data)
{
    int i;
    FILE *saveArq;

    // zera todas as informaçoes para criar novo save
    data->mapNum = 1;
    for (i = 0; i < 3; i++)
    {
        data->Player_info.mon[0].current_health_value = 0;
        data->Player_info.mon[0].current_xp = 0;
        data->Player_info.mon[0].infmon_type = 0;
        data->Player_info.mon[0].level = 0;
        data->Player_info.mon[0].level_up_xp_threshold = 0;
        data->Player_info.mon[0].max_health = 0;
    }

    saveArq = fopen("saves/save.bin", "w");

    if (saveArq == NULL)
    {
        return 1;
    }
    else
    {
        // escreve e salva o arquivo com o save
        fwrite(data, sizeof(*data), 1, saveArq);
        fclose(saveArq);
    }
    return 0;
}

// Função que determina se jogador está passando por zona de encontro aleatório
int randomEncounter()
{
    int battle_start = FALSE;

    if (GetRandomValue(1, 300) == 1)
        battle_start = TRUE;

    return battle_start;
}

// Função que gera encontro aleatório
int movingThroughGrass(int pX, int pY, char map[COLUMNS][ROWS])
{

    int in_grass = FALSE;

    if (map[(pX / SQUARE_WIDTH)][(pY / SQUARE_WIDTH)] == 'G')
        in_grass = TRUE;

    return in_grass;
}

// funcao que cria um infmon aleatorio para a luta aleatoria
Infmon generateRandomInfmon()
{
    Infmon randomEnemy;

    randomEnemy.current_xp = 0;

    return randomEnemy;
}

// Função que atualiza a posição do personagem
int movePlayer(GameInfo *game, int *pX, int *pY, char map[COLUMNS][ROWS])
{
    int moving = FALSE;
    if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && map[(*pX + ENTITY_SIZE) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
    {
        *pX += MOVEMENT_SPEED;
        moving = TRUE;
    }
    if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && map[(*pX - MOVEMENT_SPEED) / SQUARE_WIDTH][*pY / SQUARE_WIDTH] != 'W')
    {
        *pX -= MOVEMENT_SPEED;
        moving = TRUE;
    }
    if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && map[*pX / SQUARE_WIDTH][(*pY - MOVEMENT_SPEED) / SQUARE_WIDTH] != 'W')
    {
        *pY -= MOVEMENT_SPEED;
        moving = TRUE;
    }
    if ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && map[*pX / SQUARE_WIDTH][(*pY + ENTITY_SIZE) / SQUARE_WIDTH] != 'W')
    {
        *pY += MOVEMENT_SPEED;
        moving = TRUE;
    }
    if (moving)
    {
        game->current_game_saved = FALSE;
    }

    return moving;
}