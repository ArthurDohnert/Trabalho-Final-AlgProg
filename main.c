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
#define INFMON_BIGGEST_NAME 30
#define TRUE 1
#define FALSE 0

// ESTRUTURAS
//*********************************************************************************************************************
// Estrutura do infmon
typedef struct
{
    int current_health_value;
    int max_health;
    int level;
    int current_xp;
    int level_up_xp_threshold;
    int attack;
    int defense;
    char infmon_type;
    char name[30];
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
    int must_exit;          // Determina se o jogador quer fechar o jogo
    int game_situation;     // 0: main menu, 1: exploração, 2: combate
    int current_game_saved; // guarda se o jogo atual esta salvo
    int randomInfmon;       // determina se o infmon com o qual havera o combate é aleatorio
    int choosenInfmon;      // determina qual sera o infmon a iniciar no combate
    int infmonMenu;         // determina se o menu de infmons esta aberto
    int menuCombat;         // 0: menu inicial, 1: ataques, 2: trocar infmon
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
void combat(GameInfo *game, Entity *player, Infmon *enemy, int *chooseV, int *chooseH);

// Função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS]);

// faz o menu de infmons
void openInfmonMenu(GameInfo *game, Entity *player, int *choose);

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

// gera um infmon aleatorio
Infmon generateRandomInfmon();

int tryToCatchInfmon(Infmon enemy);

int addInfmon(Entity *player, Infmon enemy);

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
    Infmon enemy;

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------

    game.game_is_paused = FALSE;
    game.exit_requested = FALSE;
    game.must_exit = FALSE;
    game.randomInfmon = FALSE;
    game.infmonMenu = FALSE;
    game.choosenInfmon = 0;  // inicia o jogo com primeiro infmon sendo escolhido para os combates
    game.game_situation = 0; // inicia na tela de menu
    game.menuCombat = 0;

    //---------------------------------------------------------------------------------------------------------------------

    // VARIÁVEIS
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int menuChooseVertical = 0, menuChooseHorizontal = 0;
    int mapNum = 5;
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
            if (confirmExit(&game, &menuChooseVertical))
            {
                saveGame(&saveData, player, mapNum);
            }
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Jogo pausado
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_is_paused)
        {
            pauseGame(&game, &saveData, &player, &mapNum, &menuChooseVertical, map);
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Menu de seleção de infmons
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.infmonMenu)
        {
            openInfmonMenu(&game, &player, &menuChooseVertical);
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Main menu
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == 0)
        {
            switch (mainMenu(&game, &menuChooseVertical))
            {
            case 1:
                loadGame(&saveData, &player, &mapNum);
                break;

            case 2:
                if (!newGame(&saveData))
                {
                    loadGame(&saveData, &player, &mapNum);
                    loadMap(map, mapNum, &player, 1);
                    saveGame(&saveData, player, mapNum);
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
            if (game.randomInfmon == TRUE)
            {
                enemy = generateRandomInfmon();
                game.randomInfmon = 2; // gera o infmon aleatorio e não executa mais
            }
            else if (game.randomInfmon == FALSE)
            {
            }
            combat(&game, &player, &enemy, &menuChooseVertical, &menuChooseHorizontal);
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
    char numStr[3];
    char fileName[20] = "maps/Mapa";
    FILE *arqMap;

    sprintf(numStr, "%d", numMap);

    strcat(fileName, numStr);
    strcat(fileName, ".txt");

    arqMap = fopen(fileName, "r");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// TODO :::: CORRIGIR ESTE IF
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
            *choose = 5;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 5)
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
    if (IsKeyPressed(KEY_I) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        game->infmonMenu = TRUE;
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_S) || (IsKeyPressed(KEY_ENTER) && *choose == 2))
    {
        saveGame(data, *player, *numMap);
        game->current_game_saved = TRUE;
    }
    if (IsKeyPressed(KEY_L) || (IsKeyPressed(KEY_ENTER) && *choose == 3))
    {
        loadGame(data, player, numMap);
        loadMap(map, *numMap, player, 0);
        *choose = 0;
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_B) || (IsKeyPressed(KEY_ENTER) && *choose == 4))
    {
        game->game_situation = 0;
        game->game_is_paused = FALSE;
        *choose = 0;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 5))
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
        DrawText("Selecionar Infmons (I)", SCREEN_WIDTH / 2 - 260, 500, 50, WHITE);
        DrawText("Salvar jogo (S)", SCREEN_WIDTH / 2 - 180, 650, 50, WHITE);
    }
    else
    {
        DrawText("Carregar jogo (L)", SCREEN_WIDTH / 2 - 210, 350, 50, WHITE);
        DrawText("Voltar ao menu (B)", SCREEN_WIDTH / 2 - 220, 500, 50, WHITE);
        DrawText("Sair (S)", SCREEN_WIDTH / 2 - 80, 650, 50, WHITE);
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
            if (randomEncounter(game))
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
void combat(GameInfo *game, Entity *player, Infmon *enemy, int *chooseV, int *chooseH)
{
    char enemyLvl[9] = "Nível ";
    char playerMonLvl[9] = "Nível ";
    char enemyHealth[10] = {};
    char playerMonHealth[10] = {};
    char numStr[5];

    // gera a string enemy lvl, para ser exibida no combate
    sprintf(numStr, "%d", enemy->level);
    strcat(enemyLvl, numStr);

    // gera a string player mon lvl, para ser exibida no combate
    sprintf(numStr, "%d", player->mon[game->choosenInfmon].level);
    strcat(playerMonLvl, numStr);

    // gera a string enemy health
    sprintf(numStr, "%d", enemy->current_health_value);
    strcat(enemyHealth, numStr);
    strcat(enemyHealth, "/");
    sprintf(numStr, "%d", enemy->max_health);
    strcat(enemyHealth, numStr);

    // gera a string enemy health
    sprintf(numStr, "%d", player->mon[game->choosenInfmon].current_health_value);
    strcat(playerMonHealth, numStr);
    strcat(playerMonHealth, "/");
    sprintf(numStr, "%d", player->mon[game->choosenInfmon].current_health_value);
    strcat(playerMonHealth, numStr);

    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*chooseV == 0)
            *chooseV = 1;
        else
            *chooseV -= 1;
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*chooseV == 1)
            *chooseV = 0;
        else
            *chooseV += 1;
    }

    // move para a direita a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_RIGHT))
    {
        if (*chooseH == 0)
            *chooseH = 1;
        else
            *chooseH -= 1;
    }

    // move para a esquerda a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_LEFT))
    {
        if (*chooseH == 1)
            *chooseH = 0;
        else
            *chooseH += 1;
    }

    // tenta capturar o infmon ininmigo
    if (IsKeyPressed(KEY_C) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 0)) && game->menuCombat == 0))
    {
        if (game->randomInfmon == 2)
        {
            if (tryToCatchInfmon(*enemy))
            {
                if (addInfmon(player, *enemy))
                {
                    game->randomInfmon = FALSE;
                    game->game_situation = 1;
                    game->menuCombat = 0;
                    *chooseH = 0;
                    *chooseV = 0;
                }
                else
                {
                    DrawText("Infbag cheia...", 100, 880, 30, BLACK);
                }
            }
            else
            {
                DrawText("Não foi possível capturar o Infmon", 100, 880, 30, BLACK);
            }
        }
        else
        {
            DrawText("Não é possível capturar Infmon não selvagem", 100, 880, 30, BLACK);
        }
    }

    // troca de infmon
    if (IsKeyPressed(KEY_ENTER) && game->menuCombat == 2)
    {
        if (*chooseH == 0 && *chooseV == 0 && player->mon[0].level != -1)
        {
            game->choosenInfmon = 0;
        }
        else if (*chooseH == 1 && *chooseV == 0 && player->mon[1].level != -1)
        {
            game->choosenInfmon = 1;
        }
        else if (*chooseH == 0 && *chooseV == 1 && player->mon[2].level != -1)
        {
            game->choosenInfmon = 2;
        }
    }

    // abre o submenu de ataque
    if (IsKeyPressed(KEY_A) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 0 && *chooseV == 0)) && game->menuCombat == 0))
    {
        game->menuCombat = 1;
    }

    // abre o submenu de seleção de infmons
    if (IsKeyPressed(KEY_T) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 0 && *chooseV == 1)) && game->menuCombat == 0))
    {
        game->menuCombat = 2;
    }

    // foge do combate, precisa ficar antes do if da opção de voltar pro menu padrão de combate
    if (IsKeyPressed(KEY_R) || (IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 1) && game->menuCombat == 0))
    {
        game->randomInfmon = FALSE;
        game->game_situation = 1;
        game->menuCombat = 0;
        *chooseH = 0;
        *chooseV = 0;
    }

    // volta para o menu padrao de combate
    if ((IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 1)) && (game->menuCombat == 1 || game->menuCombat == 2))
    {
        game->menuCombat = 0;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, 400, 250, BLACK);
    DrawText("COMBAT", 100, 100, 50, WHITE);

    // menu de opcoes
    DrawRectangle(1180, 620, SCREEN_WIDTH - 900, 10, BLACK);
    DrawRectangle(1180, 620, 10, SCREEN_HEIGHT - 500, BLACK);

    // desenha o quadrado ao redor da opcao escolhida no menu
    DrawRectangle(1230 + 330 * *chooseH, 670 + 130 * *chooseV, 330, 10, BLACK);
    DrawRectangle(1230 + 330 * *chooseH, 670 + 130 * *chooseV, 10, 130, BLACK);
    DrawRectangle(1230 + 330 * *chooseH, 790 + 130 * *chooseV, 330, 10, BLACK);
    DrawRectangle(1550 + 330 * *chooseH, 670 + 130 * *chooseV, 10, 130, BLACK);

    // opcoes do menu de combate
    switch (game->menuCombat)
    {
    case 0:
        DrawText("ATAQUE", 1290, 710, 50, BLACK);
        DrawText("CAPTURA", 1600, 710, 50, BLACK);
        DrawText("TROCA", 1300, 840, 50, BLACK);
        DrawText("FUGA", 1650, 840, 50, BLACK);
        break;

    case 1:
        DrawText("PATADA", 1290, 710, 50, BLACK);
        DrawText("FOGAREU", 1600, 710, 50, BLACK);
        DrawText("EXEMPLO", 1290, 840, 50, BLACK);
        DrawText("VOLTAR", 1600, 840, 50, BLACK);
        break;

    case 2:
        DrawText(player->mon[0].name, 1290, 710, 50, BLACK);
        DrawText(player->mon[1].name, 1600, 710, 50, BLACK);
        DrawText(player->mon[2].name, 1290, 840, 50, BLACK);
        DrawText("VOLTAR", 1600, 840, 50, BLACK);
        break;
    }

    // desenha o player ----------------------------------------
    switch (player->mon[game->choosenInfmon].infmon_type)
    {
    case 'f':
        DrawRectangle(400, 700, 64, 64, RED);
        break;

    case 'w':
        DrawRectangle(400, 700, 64, 64, BLUE);
        break;

    case 'g':
        DrawRectangle(400, 700, 64, 64, GREEN);
        break;
    }

    // exibe o nivel centralizado em cima do infmon combate
    if (player->mon[game->choosenInfmon].level >= 10)
        DrawText(playerMonLvl, 395, 680, 20, BLACK);
    else
        DrawText(playerMonLvl, 400, 680, 20, BLACK);

    // desenha a barra de vida e a quantidade de vida
    DrawText(playerMonHealth, 390, 780, 20, BLACK);
    DrawRectangle(330, 804, 204, 30, DARKGRAY);
    DrawRectangle(335, 809, 194, 20, RED);
    DrawRectangle(335, 809, 194 * ((float)(player->mon[game->choosenInfmon].current_health_value / player->mon[game->choosenInfmon].max_health)), 20, GREEN);

    // desenha o oponente ----------------------------------------
    switch (enemy->infmon_type)
    {
    case 'f':
        DrawRectangle(1500, 300, 64, 64, RED);
        break;

    case 'w':
        DrawRectangle(1500, 300, 64, 64, BLUE);
        break;

    case 'g':
        DrawRectangle(1500, 300, 64, 64, GREEN);
        break;
    }

    // exibe o nivel centralizado em cima do infmon combate
    if (enemy->level >= 10)
        DrawText(enemyLvl, 1495, 280, 20, BLACK);
    else
        DrawText(enemyLvl, 1500, 280, 20, BLACK);

    // desenha a barra de vida e a quantidade de vida
    DrawText(enemyHealth, 1490, 380, 20, BLACK);
    DrawRectangle(1430, 404, 204, 30, DARKGRAY);
    DrawRectangle(1435, 409, 194, 20, RED);
    DrawRectangle(1435, 409, 194 * ((float)(enemy->current_health_value / enemy->max_health)), 20, GREEN);

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

void openInfmonMenu(GameInfo *game, Entity *player, int *choose)
{
    int i = 0;

    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
            *choose = 3;
        else
            *choose -= 1;
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 3)
            *choose = 0;
        else
            *choose += 1;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 3))
    {
        game->infmonMenu = FALSE;
        game->game_is_paused = TRUE;
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
    DrawText("Selecione seu Infmon", SCREEN_WIDTH / 2 - 250, 180, 50, WHITE);

    for (i = 0; i < 3; i++)
    {
        if (player->mon[i].level != -1)
        {
            DrawText(player->mon[i].name, SCREEN_WIDTH / 2 - 200, 350 + i * 150, 50, WHITE);
        }
        else
        {
            DrawText("Infbola vazia!", SCREEN_WIDTH / 2 - 200, 350 + i * 150, 50, WHITE);
        }
    }

    DrawText("Voltar", SCREEN_WIDTH / 2 - 150, 800, 50, WHITE);

    EndDrawing();
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

    player->posX = data->Player_info.posX;
    player->posY = data->Player_info.posY;
    player->mon[0] = data->Player_info.mon[0];
    player->mon[1] = data->Player_info.mon[1];
    player->mon[2] = data->Player_info.mon[2];

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
    FILE *saveArq;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// TODO :::: ARRUMAR QUE POKEMONS O PLAYER DEVE TER AO COMEÇAR UM SAVE NOVO
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // cria as informacoes iniciais do player e do mapa ao criar um save novo
    data->mapNum = 5;
    data->Player_info.mon[1].level = -1;
    data->Player_info.mon[2].level = -1;
    strcpy(data->Player_info.mon[1].name, "");
    strcpy(data->Player_info.mon[2].name, "");

    // -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@-

    // define o infmon inicial do player
    data->Player_info.mon[0].current_xp = 0;
    data->Player_info.mon[0].level = 1;
    data->Player_info.mon[0].max_health = data->Player_info.mon[0].current_health_value = 275;
    data->Player_info.mon[0].level_up_xp_threshold = 10;
    data->Player_info.mon[0].attack = 5;
    data->Player_info.mon[0].defense = 3;
    data->Player_info.mon[0].infmon_type = 'f';
    strcpy(data->Player_info.mon[0].name, "fogomon");

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
int randomEncounter(GameInfo *game)
{
    int battle_start = FALSE;

    if (GetRandomValue(1, 20) == 1)
    {
        battle_start = TRUE;
        game->randomInfmon = TRUE;
    }

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
    int randomValueToType;

    randomEnemy.current_xp = 0;
    randomEnemy.level = GetRandomValue(1, 10);
    randomEnemy.max_health = randomEnemy.current_health_value = 275 + 25 * (randomEnemy.level - 1);
    randomEnemy.level_up_xp_threshold = 10 + 5 * (randomEnemy.level * 5);
    randomEnemy.attack = 4 + randomEnemy.level;
    randomEnemy.defense = 2 + randomEnemy.level;

    randomValueToType = GetRandomValue(1, 3);

    switch (randomValueToType)
    {
    case 1:
        randomEnemy.infmon_type = 'f';
        strcpy(randomEnemy.name, "fogomon");
        break;

    case 2:
        randomEnemy.infmon_type = 'w';
        strcpy(randomEnemy.name, "aguamon");
        break;

    case 3:
        randomEnemy.infmon_type = 'g';
        strcpy(randomEnemy.name, "gramamon");
        break;
    }

    return randomEnemy;
}

// tenta capturar o infmon com base na vida dele
int tryToCatchInfmon(Infmon enemy)
{
    int chance = enemy.current_health_value / 40; // determina a chance do infmon ser capturado

    if (GetRandomValue(1, chance) == 2)
    {
        return 1;
    }
    return 0;
}

int addInfmon(Entity *player, Infmon enemy)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        if (player->mon[i].level == -1)
        {
            player->mon[i] = enemy;
            return 1;
        }
    }

    return 0;
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