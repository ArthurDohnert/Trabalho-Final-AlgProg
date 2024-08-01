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
} Maps;
//*********************************************************************************************************************

// PROTÓTIPOS DE FUNÇÕES
//*********************************************************************************************************************

// Função que determina se jogador está passando por zona de encontro aleatório
int movingThroughGrass(int pX, int pY, char map[ROWS][COLUMNS]);

// Função que gera encontro aleatório
int randomEncounter();

void changeMap(char map[ROWS][COLUMNS], int numMap, int *teste);

// Função que desenha o mapa
void drawMap(Maps *m, Texture2D wall, char map[ROWS][COLUMNS]);

// Função que atualiza a posição do personagem
int movePlayer(int *pX, int *pY, char map[ROWS][COLUMNS]);
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

    int mapNum = 1;
    char map[ROWS][COLUMNS]; // Gerador de mapa provisório

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------

    int main_menu = TRUE;
    int exploring_map = FALSE;  // Determina se o jogador pode se mover pelo mapa
    int game_is_paused = FALSE; // Determina se o jogo está pausado
    int exit_requested = FALSE;
    int must_exit = FALSE; // Determina se o jogador quer fechar o jogo
    int in_combat = FALSE; // Determina se está em combate

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Infmon");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // Define texturas
    //--------------------------------------------------------------------------------------------------------------------
    Texture2D wallTexture = LoadTexture("sprites/Wall(AllMaps).png");
    grass.floor = LoadTexture("sprites/TerrenoDeGrama.png");
    grass.bush = LoadTexture("sprites/TerrenoDeTransicaoGrama.png");
    player.texture = LoadTexture("sprites/MainChar.png");

    // Gera o mapa
    //---------------------------------------------------------------------------------------------------------------------
    SetRandomSeed(time(NULL));
    changeMap(map, mapNum, &must_exit);
    //---------------------------------------------------------------------------------------------------------------------

    // Vínculo principal do jogo
    // #####################################################################################################################
    while (!(WindowShouldClose() || must_exit))
    {
        //---------------------------------------------------------------------------------------------------------------------
        // Pergunta se o jogador quer sair. Precisa ter preferência na sequência de ifs
        //---------------------------------------------------------------------------------------------------------------------
        if (exit_requested)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                exit_requested = FALSE;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                must_exit = TRUE;
                exit_requested = FALSE;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawRectangle(0, 100, SCREEN_WIDTH, SCREEN_HEIGHT - 200, BLACK);
            DrawText("Você deseja sair sem salvar?", 80, 180, 30, WHITE);
            DrawText("Pressione enter para sair", 80, 300, 30, WHITE);
            EndDrawing();
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Exploração
        //---------------------------------------------------------------------------------------------------------------------
        else if (main_menu)
        {
            if (IsKeyPressed(KEY_C))
            {
                // TODO: loadGame()
                main_menu = FALSE;
                exploring_map = TRUE;
            }
            if (IsKeyPressed(KEY_N))
            {
                // TODO: newGame()
            }
            if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
            {
                exit_requested = TRUE;
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

        else if (exploring_map)
        {
            if (IsKeyPressed(KEY_TAB))
            {
                game_is_paused = TRUE;
                exploring_map = FALSE;
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                exit_requested = TRUE;
            }
            if (movePlayer(&player.posX, &player.posY, map))
            {
                if (movingThroughGrass(player.posX, player.posY, map))
                {
                    if (randomEncounter())
                    {
                        exploring_map = FALSE;
                        in_combat = TRUE;
                    }
                }
            }

            movePlayer(&player.posX, &player.posY, map);
            BeginDrawing();
            drawMap(&grass, wallTexture, map);
            ClearBackground(RAYWHITE);
            DrawTexture(player.texture, player.posX, player.posY, WHITE);
            DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
            EndDrawing();
        }

        //--------------------------------------------------------------------------------------------------------------------
        //    Em combate
        //--------------------------------------------------------------------------------------------------------------------

        else if (in_combat)
        {

            if (IsKeyPressed(KEY_R))
            {
                in_combat = FALSE;
                exploring_map = TRUE;
            }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("COMBAT", 400, 300, 50, WHITE);
            EndDrawing();
        }

        //--------------------------------------------------------------------------------------------------------------------
        // Jogo pausado
        //---------------------------------------------------------------------------------------------------------------------

        else if (game_is_paused)
        {
            if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_C))
            {
                game_is_paused = FALSE;
                exploring_map = TRUE;
            }
            if (IsKeyPressed(KEY_Q))
            {
                exit_requested = TRUE;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("PAUSED", 400, 300, 50, WHITE);
            EndDrawing();
        }
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
int randomEncounter()
{
    int battle_start = FALSE;

    if (GetRandomValue(1, 100) == 1)
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

void changeMap(char map[ROWS][COLUMNS], int numMap, int *teste)
{
    int i, j;
    FILE *arqMap;

    arqMap = fopen("maps/Mapa1.txt", "r");

    if (arqMap == NULL)
    {
        *teste = 1;
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

void drawMap(Maps *m, Texture2D wall, char map[ROWS][COLUMNS])
{
    int i, j;
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLUMNS; j++)
        {
            switch (map[i][j])
            {
            case 'W':
                DrawTexture(wall, j * SQUARE_WIDTH, i * SQUARE_WIDTH, WHITE);
                break;
            case ' ':
            case 'J':
            case 'E':
                DrawRectangle(j * SQUARE_WIDTH, i * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                break;
            case 'G':
                DrawTexture(m->floor, j * SQUARE_WIDTH, i * SQUARE_WIDTH, WHITE); // TODO: corrigir bug e mudar de 'floor' para 'bush'
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
