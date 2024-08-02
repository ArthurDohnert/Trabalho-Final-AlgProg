#include "raylib.h"

#include <stdio.h>
#include <time.h>

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

/*NOVO: Estrutura 'Infmon', flag de combate, funções pra gerar encontro aleatório, constantes pra matriz mapa,
função "movePlayer" agora retorna um inteiro (temporário, pra testar o encontro aleatório)
*/

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

void changeMap(char map[ROWS][COLUMNS], int numMap);

void confirmExit(GameInfo *game);

void mainMenu(GameInfo *game);

void pauseGame(GameInfo *game);

void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[ROWS][COLUMNS]);

void combat(GameInfo *game);

// Função que desenha o mapa
void drawMap(Maps *m, char map[ROWS][COLUMNS]);

// Função que determina se jogador está passando por zona de encontro aleatório
int movingThroughGrass(int pX, int pY, char map[ROWS][COLUMNS]);

// Função que gera encontro aleatório
int randomEncounter();

// Função que atualiza a posição do personagem
int movePlayer(int *pX, int *pY, char map[ROWS][COLUMNS]);
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

    int mapNum = 1;
    char map[ROWS][COLUMNS]; // Gerador de mapa provisório

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Infmon");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // Define texturas
    //--------------------------------------------------------------------------------------------------------------------
    grass.wall = LoadTexture("sprites/Wall(AllMaps).png");
    grass.floor = LoadTexture("sprites/TerrenoDeGrama.png");
    grass.bush = LoadTexture("sprites/TerrenoDeTransicaoGrama.png");
    player.texture = LoadTexture("sprites/MainChar.png");

    // Gera o mapa
    //---------------------------------------------------------------------------------------------------------------------
    SetRandomSeed(time(NULL));
    changeMap(map, mapNum);
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

    UnloadTexture(player.texture);
    UnloadTexture(grass.floor);
    UnloadTexture(grass.bush);
    UnloadTexture(grass.wall);

    // #####################################################################################################################
    CloseWindow(); // Fecha o jogo
    return 0;
}

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

void pauseGame(GameInfo *game)
{
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_C))
    {
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_Q))
    {
        game->exit_requested = TRUE;
    }
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("PAUSED", 400, 300, 50, WHITE);
    EndDrawing();
}

void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[ROWS][COLUMNS])
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
    drawMap(mapa, map);
    ClearBackground(RAYWHITE);
    DrawTexture(player->texture, player->posX, player->posY, WHITE);
    DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
    EndDrawing();
}

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

//=====================================================================================================================
// FUNÇÕES CUSTOMIZADAS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int randomEncounter()
{
    int battle_start = FALSE;

    if (GetRandomValue(1, 300) == 1)
        battle_start = TRUE;

    return battle_start;
}

int movingThroughGrass(int pX, int pY, char map[ROWS][COLUMNS])
{

    int in_grass = FALSE;

    if (map[(pX / SQUARE_WIDTH)][(pY / SQUARE_WIDTH)] == 'G')
        in_grass = TRUE;

    return in_grass;
}

void changeMap(char map[ROWS][COLUMNS], int numMap)
{
    int i, j;
    FILE *arqMap;

    arqMap = fopen("maps/Mapa1.txt", "r");

    if (arqMap == NULL)
    {
    }
    else
    {
        for (i = 0; i < ROWS; i++)
        {
            for (j = 0; j < COLUMNS; j++)
            {
                if ((map[i][j] = getc(arqMap)) == '\n')
                    j--;
            }
        }
    }

    fclose(arqMap);
}

void drawMap(Maps *m, char map[ROWS][COLUMNS])
{
    int i, j;
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLUMNS; j++)
        {
            switch (map[i][j])
            {
            case 'W':
                DrawTexture(m->wall, j * SQUARE_WIDTH, i * SQUARE_WIDTH, WHITE);
                break;
            case ' ':
            case 'J':
            case 'E':
                DrawRectangle(j * SQUARE_WIDTH, i * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                break;
            case 'G':
                DrawTexture(m->floor, j * SQUARE_WIDTH, i * SQUARE_WIDTH, WHITE);
                break;
            }
        }
    }
}

int movePlayer(int *pX, int *pY, char map[ROWS][COLUMNS])
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
