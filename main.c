#include "raylib.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

// constantes gerais de jogo
#define SCREEN_HEIGHT 960
#define SCREEN_WIDTH 1920
#define MOVEMENT_SPEED 3
#define SQUARE_WIDTH 32
#define ENTITY_SIZE 32
#define ROWS 30
#define COLUMNS 60
#define MAX_INFMONS 3
#define INFMON_BIGGEST_NAME 30
#define AVG_TEXT_SIZE 50
#define SMALL_TEXT_SIZE 10
#define NUM_MAPAS 4
#define TRUE 1
#define FALSE 0

// constantes de infmons
#define MAX_HABILIDADES_INFMON 3
#define NIVEL_ESPACO_VAZIO -1
#define DANO_INICIAL_LVL1 100
#define DANO_POR_NIVEL 6
#define VIDA_INICIAL_LVL1 275
#define VIDA_POR_NIVEL 25
#define DEFESA_INICIAL_LVL1 3
#define DEFESA_POR_NIVEL 1
#define XP_THRESHOLD_INICIAL 10
#define XP_THRESHOLD_POR_NIVEL 20
#define XP_GANHO_POR_LVL_INIMIGO 5

// constantes de combate
#define BUFF_TIPO 2
#define NERF_TIPO 0.5
#define TIPO_NEUTRO 1
#define NIVEL_RAYANSAUR 15
#define NIVEL_BOSS 20
#define NIVEL_DENNISZARD 30
#define NIVEL_THIAGOTOISE 50
#define STD_ATK1_DMG 1
#define STD_ATK2_DMG 1.1
#define BUFF1_ATK2_DMG 1.15
#define BUFF2_ATK2_DMG 1.25
#define BUFF3_ATK2_DMG 1.4
#define STD_ATK3_DMG 1.25
#define BUFF1_ATK3_DMG 1.4
#define BUFF2_ATK3_DMG 1.5
#define BUFF3_ATK3_DMG 1.7

// constantes de menu
#define MAX_ITENS_MENU 3
#define MAIN_MENU 0
#define EXPLORACAO 1
#define COMBATE 2
#define PERDEU 3
#define GANHOU 4
#define MENU_COMBATE_PRINCIPAL 0
#define MENU_ATAQUE 1
#define MENU_TROCAR_INFMON 2

// ESTRUTURAS
//*********************************************************************************************************************

// ataques dos infmons
typedef struct
{
    char attack[AVG_TEXT_SIZE];
    char type;
    float multiplier;
} Attacks;

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
    char name[AVG_TEXT_SIZE];
    Attacks habilities[MAX_HABILIDADES_INFMON];
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
    Texture2D boss;
    Texture2D bossFight;
} Maps;

// guarda todas as informaçoes do estado do jogo
typedef struct
{
    int game_is_paused; // Determina se o jogo está pausado
    int exit_requested;
    int must_exit;          // Determina se o jogador quer fechar o jogo
    int game_situation;     // 0: main menu, 1: exploração, 2: combate, 3: perdeu o jogo, 4: ganhou o jogo
    int current_game_saved; // guarda se o jogo atual esta salvo
    int randomInfmon;       // determina se o infmon com o qual havera o combate é aleatorio
    int choosenInfmon;      // determina qual sera o infmon a iniciar no combate
    int infmonMenu;         // determina se o menu de infmons esta aberto
    int menuCombat;         // 0: menu inicial, 1: ataques, 2: trocar infmon
} GameInfo;

// estrutura com as informacoes do combate
typedef struct
{
    char enemyLvl[SMALL_TEXT_SIZE];
    char playerMonLvl[SMALL_TEXT_SIZE];
    char enemyHealth[SMALL_TEXT_SIZE];
    char playerMonHealth[SMALL_TEXT_SIZE];
    char numStr[SMALL_TEXT_SIZE / 2];
    char fightText[AVG_TEXT_SIZE];
    char enemyText[AVG_TEXT_SIZE];
    int infmonTurn; // 0 para o player, 1 para o inimigo
} CombatStats;

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
void loadMap(GameInfo *game, char map[COLUMNS][ROWS], int numMap, Entity *player, int alteraPlayerPos);

// confirma se o player realmente quer sair
int confirmExit(GameInfo *game, int *choose);

// faz o menu principal
int mainMenu(GameInfo *game, int *choose);

// controla a pause
void pauseGame(GameInfo *game, SaveInfo *data, Entity *player, int *numMap, int *choose, char map[COLUMNS][ROWS]);

// exploracao
void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[COLUMNS][ROWS], int numMap);

// combate
void combat(GameInfo *game, Entity *player, Infmon *enemy, CombatStats *combatInfo, Maps *mapInfo, SaveInfo *data, char map[COLUMNS][ROWS], int *chooseV, int *chooseH, int *numMap);

// ataque do inimigo no combate
void enemyAttack(GameInfo *game, Entity *player, Infmon *enemy, CombatStats *combatInfo);

// tela de que morreu e perdeu o jogo
void loseGame(GameInfo *game, int *choose);

// tela de que matou o boss final
void wonGame(GameInfo *game, int *choose);

// cura todos os infmons do player
void healInfmon(Entity *player);

// verifica se um infmon subiu de nível, se sim, melhora seus atributos e mostra na tela que ele subiu de nivel
int levelUpInfmon(Entity *player, int selectedInfmon);

// define o quao forte vai ser o ataque com base no tipo do ataque e o tipo do inimigo
float getTypeDamageMultiplier(char attackingType, char defenderType);

// Função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS], int numMap);

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

// tenta capturar o infmon da batalha
int tryToCatchInfmon(Infmon enemy);

// adiciona infmon capturado à infbag
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
    Maps grass, fire, fire2, water, usedTexture;
    Entity player;
    SaveInfo saveData;
    Infmon enemy;
    Infmon rayanSaur, boss1, dennisZard, thiagosToise;
    CombatStats combatInfo;

    // Variáveis de "estado de jogo"
    //---------------------------------------------------------------------------------------------------------------------
    game.game_is_paused = FALSE;
    game.exit_requested = FALSE;
    game.must_exit = FALSE;
    game.randomInfmon = FALSE;
    game.infmonMenu = FALSE;
    game.choosenInfmon = 0;          // inicia o jogo com primeiro infmon sendo escolhido para os combates
    game.game_situation = MAIN_MENU; // inicia na tela de menu
    game.menuCombat = MENU_COMBATE_PRINCIPAL;

    // variaveis de combate
    combatInfo.fightText[0] = '\0';
    combatInfo.enemyText[0] = '\0';
    combatInfo.infmonTurn = 0;

    // definindo o RayanSaur
    rayanSaur.level = NIVEL_RAYANSAUR;
    rayanSaur.infmon_type = 'g';
    rayanSaur.max_health = rayanSaur.current_health_value = VIDA_INICIAL_LVL1 + VIDA_POR_NIVEL * (rayanSaur.level - 1);
    rayanSaur.attack = DANO_INICIAL_LVL1 + DANO_POR_NIVEL * (rayanSaur.level - 1);
    rayanSaur.defense = DEFESA_INICIAL_LVL1 + DEFESA_POR_NIVEL * (rayanSaur.level - 1);

    strcpy(rayanSaur.name, "RayanSaur");
    strcpy(rayanSaur.habilities[0].attack, "FOTOSSÍNTESE");
    rayanSaur.habilities[0].type = 'g';
    rayanSaur.habilities[0].multiplier = STD_ATK1_DMG;
    strcpy(rayanSaur.habilities[1].attack, "SOCO DIRETO");
    rayanSaur.habilities[1].type = 'n';
    rayanSaur.habilities[1].type = STD_ATK2_DMG;
    strcpy(rayanSaur.habilities[2].attack, "INSPIRAÇÃO PROGRAMADA");
    rayanSaur.habilities[2].type = 'g';
    rayanSaur.habilities[2].type = STD_ATK3_DMG;

    // definindo o boss1
    boss1.level = NIVEL_BOSS;
    boss1.infmon_type = 'f';
    boss1.max_health = boss1.current_health_value = VIDA_INICIAL_LVL1 + VIDA_POR_NIVEL * (boss1.level - 1);
    boss1.attack = DANO_INICIAL_LVL1 + DANO_POR_NIVEL * (boss1.level - 1);
    boss1.defense = DEFESA_INICIAL_LVL1 + DEFESA_POR_NIVEL * (boss1.level - 1);

    strcpy(boss1.name, "Caramelo do Vale");
    strcpy(boss1.habilities[0].attack, "LANÇA-CHAMAS");
    boss1.habilities[0].type = 'f';
    boss1.habilities[0].multiplier = STD_ATK1_DMG;
    strcpy(boss1.habilities[1].attack, "SOCO DIRETO");
    boss1.habilities[1].type = 'n';
    boss1.habilities[1].type = BUFF1_ATK2_DMG;
    strcpy(boss1.habilities[2].attack, "EUJUROQUESOUHUMANO");
    boss1.habilities[2].type = 'f';
    boss1.habilities[2].type = BUFF1_ATK3_DMG;

    // definindo o DennisZard
    dennisZard.level = NIVEL_DENNISZARD;
    dennisZard.infmon_type = 'f';
    dennisZard.max_health = dennisZard.current_health_value = VIDA_INICIAL_LVL1 + VIDA_POR_NIVEL * (dennisZard.level - 1);
    dennisZard.attack = DANO_INICIAL_LVL1 + DANO_POR_NIVEL * (dennisZard.level - 1);
    dennisZard.defense = DEFESA_INICIAL_LVL1 + DEFESA_POR_NIVEL * (dennisZard.level - 1);

    strcpy(dennisZard.name, "DennisZard");
    strcpy(dennisZard.habilities[0].attack, "LANÇA-CHAMAS");
    dennisZard.habilities[0].type = 'f';
    dennisZard.habilities[0].multiplier = STD_ATK1_DMG;
    strcpy(dennisZard.habilities[1].attack, "SOCO DIRETO");
    dennisZard.habilities[1].type = 'n';
    dennisZard.habilities[1].type = BUFF2_ATK2_DMG;
    strcpy(dennisZard.habilities[2].attack, "LEGIBILIDADE");
    dennisZard.habilities[2].type = 'f';
    dennisZard.habilities[2].type = BUFF2_ATK3_DMG;

    // definindo o thiagosToise
    thiagosToise.level = NIVEL_THIAGOTOISE;
    thiagosToise.infmon_type = 'w';
    thiagosToise.max_health = thiagosToise.current_health_value = VIDA_INICIAL_LVL1 + VIDA_POR_NIVEL * (thiagosToise.level - 1);
    thiagosToise.attack = DANO_INICIAL_LVL1 + DANO_POR_NIVEL * (thiagosToise.level - 1);
    thiagosToise.defense = DEFESA_INICIAL_LVL1 + DEFESA_POR_NIVEL * (thiagosToise.level - 1);

    strcpy(thiagosToise.name, "ThiagosToise");
    strcpy(thiagosToise.habilities[0].attack, "JATO D'ÁGUA");
    thiagosToise.habilities[0].type = 'w';
    thiagosToise.habilities[0].multiplier = STD_ATK1_DMG;
    strcpy(thiagosToise.habilities[1].attack, "SOCO DIRETO");
    thiagosToise.habilities[1].type = 'n';
    thiagosToise.habilities[1].type = BUFF3_ATK2_DMG;
    strcpy(thiagosToise.habilities[2].attack, "CHUVA DE NOTA");
    thiagosToise.habilities[2].type = 'w';
    thiagosToise.habilities[2].type = BUFF3_ATK3_DMG;
    //---------------------------------------------------------------------------------------------------------------------

    // VARIÁVEIS
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    int menuChooseVertical = 0, menuChooseHorizontal = 0;
    int mapNum;
    char map[COLUMNS][ROWS]; // Gerador de mapa provisório

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Infmon");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // Define texturas
    //--------------------------------------------------------------------------------------------------------------------
    water.wall = fire.wall = fire2.wall = grass.wall = LoadTexture("sprites/Wall(AllMaps).png");

    grass.bush = LoadTexture("sprites/TerrenoDeGrama.png");
    grass.floor = LoadTexture("sprites/TerrenoDeTransicaoGrama.png");
    grass.boss = LoadTexture("sprites/RayanSaur.png");
    grass.bossFight = LoadTexture("sprites/RayanSaur 64bits.png");

    fire.bush = fire2.bush = LoadTexture("sprites/TerrenoDeFogo.png");
    fire.floor = fire2.floor = LoadTexture("sprites/TerrenoDeTransicaoFogo.png");
    fire.boss = LoadTexture("sprites/Boss.png");
    fire.bossFight = LoadTexture("sprites/Boss 64Bits.png");
    fire2.boss = LoadTexture("sprites/Denniszard(FireType).png");
    fire2.bossFight = LoadTexture("sprites/Denniszard 64bits.png");

    water.bush = LoadTexture("sprites/TerrenoDeAgua.png");
    water.floor = LoadTexture("sprites/TerrenoDeTransicaoAgua.png");
    water.boss = LoadTexture("sprites/ThiagosToise.png");
    water.bossFight = LoadTexture("sprites/ThiagosToise 64bits.png");

    player.texture = LoadTexture("sprites/MainChar.png");

    // Gera o mapa
    //---------------------------------------------------------------------------------------------------------------------
    SetRandomSeed(time(NULL));
    //---------------------------------------------------------------------------------------------------------------------

    // Vínculo principal do jogo
    // #####################################################################################################################
    while (!(WindowShouldClose() || game.must_exit))
    {
        switch (mapNum)
        {
        case 1:
            usedTexture = grass;
            break;

        case 2:
            usedTexture = fire;
            break;

        case 3:
            usedTexture = fire2;
            break;

        case 4:
            usedTexture = water;
            break;

        default:
            usedTexture = grass;
            break;
        }

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
        else if (game.game_situation == MAIN_MENU)
        {
            switch (mainMenu(&game, &menuChooseVertical))
            {
            case 1:
                loadGame(&saveData, &player, &mapNum);
                loadMap(&game, map, mapNum, &player, 0);
                break;

            case 2:
                if (!newGame(&saveData))
                {
                    loadGame(&saveData, &player, &mapNum);
                    loadMap(&game, map, mapNum, &player, 1);
                    saveGame(&saveData, player, mapNum);
                }
                break;
            }
        }

        //---------------------------------------------------------------------------------------------------------------------
        // Exploração
        //---------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == EXPLORACAO)
        {
            exploring(&game, &player, &usedTexture, map, mapNum);
        }

        //--------------------------------------------------------------------------------------------------------------------
        //    Em combate
        //--------------------------------------------------------------------------------------------------------------------

        else if (game.game_situation == COMBATE)
        {
            if (game.randomInfmon == TRUE)
            {
                enemy = generateRandomInfmon();
                game.randomInfmon = 2; // gera o infmon aleatorio e não executa mais
            }
            else if (game.randomInfmon == FALSE)
            {
                if (mapNum == 1)
                {
                    enemy = rayanSaur;
                }
                else if (mapNum == 2)
                {
                    enemy = boss1;
                }
                else if (mapNum == 3)
                {
                    enemy = dennisZard;
                }
                else if (mapNum == 4)
                {
                    enemy = thiagosToise;
                }
                else
                {
                    enemy = rayanSaur;
                }
                game.randomInfmon = 2;
            }
            else
            {
                combat(&game, &player, &enemy, &combatInfo, &usedTexture, &saveData, map, &menuChooseVertical, &menuChooseHorizontal, &mapNum);
            }
        }

        //--------------------------------------------------------------------------------------------------------------------
        //    Fim de jogo: perdeu
        //--------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == PERDEU)
        {
            loseGame(&game, &menuChooseVertical);
        }

        //--------------------------------------------------------------------------------------------------------------------
        //    Fim de jogo: ganhou
        //--------------------------------------------------------------------------------------------------------------------
        else if (game.game_situation == GANHOU)
        {
            wonGame(&game, &menuChooseVertical);
        }
    }

    UnloadTexture(player.texture);

    UnloadTexture(grass.floor);
    UnloadTexture(grass.bush);
    UnloadTexture(grass.wall);
    UnloadTexture(grass.boss);
    UnloadTexture(grass.bossFight);

    UnloadTexture(fire.floor);
    UnloadTexture(fire.bush);
    UnloadTexture(fire.wall);
    UnloadTexture(fire.boss);
    UnloadTexture(fire.bossFight);

    UnloadTexture(water.floor);
    UnloadTexture(water.bush);
    UnloadTexture(water.wall);
    UnloadTexture(water.boss);
    UnloadTexture(water.bossFight);

    // #####################################################################################################################
    CloseWindow(); // Fecha o jogo
    return 0;
}

//=====================================================================================================================
// FUNÇÕES CUSTOMIZADAS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// recarrega o mapa diferente na matriz map
void loadMap(GameInfo *game, char map[COLUMNS][ROWS], int numMap, Entity *player, int alteraPlayerPos)
{
    int i, j;
    int transpose[ROWS][COLUMNS];
    char numStr[3];
    char fileName[AVG_TEXT_SIZE] = "maps/Mapa";
    FILE *arqMap;

    sprintf(numStr, "%d", numMap);

    strcat(fileName, numStr);
    strcat(fileName, ".txt");

    arqMap = fopen(fileName, "r");

    if (arqMap == NULL)
    {
        game->game_situation = GANHOU;
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
            *choose = MAX_ITENS_MENU - 1;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == MAX_ITENS_MENU - 1)
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
        game->game_situation = EXPLORACAO;
        return 1;
    }
    if (IsKeyPressed(KEY_N) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        game->game_situation = EXPLORACAO;
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
        loadMap(game, map, *numMap, player, 0);
        *choose = 0;
        game->game_is_paused = FALSE;
    }
    if (IsKeyPressed(KEY_B) || (IsKeyPressed(KEY_ENTER) && *choose == 4))
    {
        game->game_situation = MAIN_MENU;
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
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * ((*choose) % MAX_ITENS_MENU), 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 450 + 150 * ((*choose) % MAX_ITENS_MENU), 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 450, 300 + 150 * ((*choose) % MAX_ITENS_MENU), 10, 160, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 + 450, 300 + 150 * ((*choose) % MAX_ITENS_MENU), 10, 160, WHITE);

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
void exploring(GameInfo *game, Entity *player, Maps *mapa, char map[COLUMNS][ROWS], int numMap)
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
                game->game_situation = COMBATE; // entra em combate
            }
        }
    }

    movePlayer(game, &player->posX, &player->posY, map);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    drawMap(mapa, map, numMap);
    DrawTexture(player->texture, player->posX, player->posY, WHITE);
    DrawText("Press TAB to open pause menu", 20, 0, 20, WHITE);
    EndDrawing();
}

// combate
void combat(GameInfo *game, Entity *player, Infmon *enemy, CombatStats *combatInfo, Maps *mapInfo, SaveInfo *data, char map[COLUMNS][ROWS], int *chooseV, int *chooseH, int *numMap)
{
    float typeMultiplier;

    strcpy(combatInfo->enemyLvl, "Nivel ");
    strcpy(combatInfo->playerMonLvl, "Nivel ");
    strcpy(combatInfo->enemyHealth, "");
    strcpy(combatInfo->playerMonHealth, "");

    // gera a string enemy lvl, para ser exibida no combate
    sprintf(combatInfo->numStr, "%d", enemy->level);
    strcat(combatInfo->enemyLvl, combatInfo->numStr);

    // gera a string player mon lvl, para ser exibida no combate
    sprintf(combatInfo->numStr, "%d", player->mon[game->choosenInfmon].level);
    strcat(combatInfo->playerMonLvl, combatInfo->numStr);

    // gera a string enemy health
    sprintf(combatInfo->numStr, "%d", enemy->current_health_value);
    strcat(combatInfo->enemyHealth, combatInfo->numStr);
    strcat(combatInfo->enemyHealth, "/");
    sprintf(combatInfo->numStr, "%d", enemy->max_health);
    strcat(combatInfo->enemyHealth, combatInfo->numStr);

    // gera a string player health
    sprintf(combatInfo->numStr, "%d", player->mon[game->choosenInfmon].current_health_value);
    strcat(combatInfo->playerMonHealth, combatInfo->numStr);
    strcat(combatInfo->playerMonHealth, "/");
    sprintf(combatInfo->numStr, "%d", player->mon[game->choosenInfmon].max_health);
    strcat(combatInfo->playerMonHealth, combatInfo->numStr);

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

    if (combatInfo->infmonTurn == 1)
    {
        enemyAttack(game, player, enemy, combatInfo);
    }

    if (enemy->current_health_value <= 0)
    {
        if (enemy->level == NIVEL_RAYANSAUR || enemy->level == NIVEL_BOSS || enemy->level == NIVEL_DENNISZARD || enemy->level == NIVEL_THIAGOTOISE)
        {
            if (*numMap == NUM_MAPAS)
            {
                game->game_situation = GANHOU;
            }
            else
            {
                *numMap += 1;
                loadMap(game, map, *numMap, player, 1);
                saveGame(data, *player, *numMap);
                loadGame(data, player, numMap);
            }
        }

        // retorna as variaveis de game ao normal
        combatInfo->infmonTurn = 0;
        game->randomInfmon = FALSE;
        if (enemy->level != 50)
            game->game_situation = EXPLORACAO;
        game->menuCombat = MENU_COMBATE_PRINCIPAL;
        *chooseH = 0;
        *chooseV = 0;
        combatInfo->fightText[0] = '\0';

        healInfmon(player); // heala os infmons

        // upa os infmons, se eles existem
        if (player->mon[0].level != NIVEL_ESPACO_VAZIO)
        {
            player->mon[0].current_xp += XP_GANHO_POR_LVL_INIMIGO * enemy->level;
            levelUpInfmon(player, 0);
        }

        if (player->mon[1].level != NIVEL_ESPACO_VAZIO)
        {
            player->mon[1].current_xp += XP_GANHO_POR_LVL_INIMIGO * enemy->level;
            levelUpInfmon(player, 1);
        }
        if (player->mon[2].level != NIVEL_ESPACO_VAZIO)
        {
            player->mon[2].current_xp += XP_GANHO_POR_LVL_INIMIGO * enemy->level;
            levelUpInfmon(player, 2);
        }
    }

    // verifica se o player morreu
    if (player->mon[game->choosenInfmon].current_health_value <= 0)
    {
        if (player->mon[(game->choosenInfmon + 1) % 3].current_health_value > 0 && player->mon[(game->choosenInfmon + 1) % 3].level != NIVEL_ESPACO_VAZIO)
        {
            game->choosenInfmon = (game->choosenInfmon + 1) % 3;
        }
        else if (player->mon[(game->choosenInfmon + 2) % 3].current_health_value > 0 && player->mon[(game->choosenInfmon + 2) % 3].level != NIVEL_ESPACO_VAZIO)
        {
            game->choosenInfmon = (game->choosenInfmon + 2) % 3;
        }
        else
        {
            strcpy(combatInfo->fightText, "");
            strcpy(combatInfo->enemyText, "");
            game->game_situation = PERDEU;
        }
    }

    // tenta capturar o infmon ininmigo
    if (IsKeyPressed(KEY_C) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 0)) && game->menuCombat == MENU_COMBATE_PRINCIPAL))
    {
        if (enemy->level == NIVEL_RAYANSAUR || enemy->level == NIVEL_BOSS || enemy->level == NIVEL_DENNISZARD || enemy->level == NIVEL_THIAGOTOISE)
        {
            strcpy(combatInfo->fightText, "Não é possível capturar este infmon");
        }
        else
        {
            combatInfo->infmonTurn = 1;
            if (game->randomInfmon == 2)
            {
                if (tryToCatchInfmon(*enemy))
                {
                    if (addInfmon(player, *enemy))
                    {
                        combatInfo->infmonTurn = 0;
                        game->randomInfmon = FALSE;
                        game->game_situation = EXPLORACAO;
                        game->menuCombat = MENU_COMBATE_PRINCIPAL;
                        *chooseH = 0;
                        *chooseV = 0;
                        combatInfo->fightText[0] = '\0';
                        healInfmon(player);
                    }
                    else
                    {
                        strcpy(combatInfo->fightText, "Infbag cheia...");
                    }
                }
                else
                {
                    strcpy(combatInfo->fightText, "Não foi possível capturar o Infmon");
                }
            }
            else
            {
                strcpy(combatInfo->fightText, "Não é possível capturar Infmon não selvagem");
            }
        }
    }

    // ataca o inimigo
    if (IsKeyPressed(KEY_ENTER) && game->menuCombat == MENU_ATAQUE)
    {
        if (*chooseH == 0 && *chooseV == 0)
        {
            combatInfo->infmonTurn = 1;
            typeMultiplier = getTypeDamageMultiplier(player->mon[game->choosenInfmon].habilities[0].type, enemy->infmon_type);
            enemy->current_health_value -= (int)(typeMultiplier * player->mon[game->choosenInfmon].habilities[0].multiplier * player->mon[game->choosenInfmon].attack) / enemy->defense;
        }
        else if (*chooseH == 1 && *chooseV == 0)
        {
            combatInfo->infmonTurn = 1;
            typeMultiplier = getTypeDamageMultiplier(player->mon[game->choosenInfmon].habilities[1].type, enemy->infmon_type);
            enemy->current_health_value -= (int)(typeMultiplier * player->mon[game->choosenInfmon].habilities[1].multiplier * player->mon[game->choosenInfmon].attack) / enemy->defense;
        }
        else if (*chooseH == 0 && *chooseV == 1)
        {
            combatInfo->infmonTurn = 1;
            typeMultiplier = getTypeDamageMultiplier(player->mon[game->choosenInfmon].habilities[2].type, enemy->infmon_type);
            enemy->current_health_value -= (int)(typeMultiplier * player->mon[game->choosenInfmon].habilities[2].multiplier * player->mon[game->choosenInfmon].attack) / enemy->defense;
        }
    }

    // troca de infmon
    if (IsKeyPressed(KEY_ENTER) && game->menuCombat == MENU_TROCAR_INFMON)
    {
        if (*chooseH == 0 && *chooseV == 0 && player->mon[0].level != NIVEL_ESPACO_VAZIO)
        {
            game->choosenInfmon = 0;
            combatInfo->infmonTurn = 1;
        }
        else if (*chooseH == 1 && *chooseV == 0 && player->mon[1].level != NIVEL_ESPACO_VAZIO)
        {
            game->choosenInfmon = 1;
            combatInfo->infmonTurn = 1;
        }
        else if (*chooseH == 0 && *chooseV == 1 && player->mon[2].level != NIVEL_ESPACO_VAZIO)
        {
            game->choosenInfmon = 2;
            combatInfo->infmonTurn = 1;
        }
    }

    // abre o submenu de ataque
    if (IsKeyPressed(KEY_A) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 0 && *chooseV == 0)) && game->menuCombat == MENU_COMBATE_PRINCIPAL))
    {
        game->menuCombat = MENU_ATAQUE;
    }

    // abre o submenu de seleção de infmons
    if (IsKeyPressed(KEY_T) || ((IsKeyPressed(KEY_ENTER) && (*chooseH == 0 && *chooseV == 1)) && game->menuCombat == MENU_COMBATE_PRINCIPAL))
    {
        game->menuCombat = MENU_TROCAR_INFMON;
    }

    // foge do combate, precisa ficar antes do if da opção de voltar pro menu padrão de combate
    if (IsKeyPressed(KEY_R) || (IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 1) && game->menuCombat == MENU_COMBATE_PRINCIPAL))
    {
        if (enemy->level == NIVEL_RAYANSAUR || enemy->level == NIVEL_BOSS || enemy->level == NIVEL_DENNISZARD || enemy->level == NIVEL_THIAGOTOISE)
        {
            strcpy(combatInfo->fightText, "Você não tem para onde fugir");
        }
        else
        {
            game->randomInfmon = FALSE;
            game->game_situation = EXPLORACAO;
            game->menuCombat = MENU_COMBATE_PRINCIPAL;
            *chooseH = 0;
            *chooseV = 0;
            strcpy(combatInfo->fightText, "");
            strcpy(combatInfo->enemyText, "");
            healInfmon(player);
        }
    }

    // volta para o menu padrao de combate
    if ((IsKeyPressed(KEY_ENTER) && (*chooseH == 1 && *chooseV == 1)) && (game->menuCombat == MENU_ATAQUE || game->menuCombat == MENU_TROCAR_INFMON))
    {
        game->menuCombat = MENU_COMBATE_PRINCIPAL;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, 400, 250, BLACK);
    DrawText("COMBAT", 100, 100, 50, WHITE);

    // texto do combate
    DrawText(combatInfo->fightText, 100, 880, 30, BLACK);
    DrawText(combatInfo->enemyText, 1250, 150, 30, BLACK);

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
        DrawText(player->mon[game->choosenInfmon].habilities[0].attack, 1290, 720, 28, BLACK);
        DrawText(player->mon[game->choosenInfmon].habilities[1].attack, 1600, 720, 28, BLACK);
        DrawText(player->mon[game->choosenInfmon].habilities[2].attack, 1290, 850, 28, BLACK);
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
        DrawText(combatInfo->playerMonLvl, 395, 680, 20, BLACK);
    else
        DrawText(combatInfo->playerMonLvl, 400, 680, 20, BLACK);

    // desenha a barra de vida e a quantidade de vida
    DrawText(combatInfo->playerMonHealth, 390, 780, 20, BLACK);
    DrawRectangle(330, 804, 204, 30, DARKGRAY);
    DrawRectangle(335, 809, 194, 20, RED);
    DrawRectangle(335, 809, 194 * ((float)player->mon[game->choosenInfmon].current_health_value / (float)player->mon[game->choosenInfmon].max_health), 20, GREEN);

    // desenha o oponente ----------------------------------------
    if (enemy->level == NIVEL_RAYANSAUR || enemy->level == NIVEL_BOSS || enemy->level == NIVEL_DENNISZARD || enemy->level == NIVEL_THIAGOTOISE)
    {
        DrawTexture(mapInfo->bossFight, 1500, 300, WHITE);
    }
    else
    {
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
    }

    // exibe o nivel centralizado em cima do infmon combate
    if (enemy->level >= 10)
        DrawText(combatInfo->enemyLvl, 1495, 280, 20, BLACK);
    else
        DrawText(combatInfo->enemyLvl, 1500, 280, 20, BLACK);

    // desenha a barra de vida e a quantidade de vida
    DrawText(combatInfo->enemyHealth, 1490, 380, 20, BLACK);
    DrawRectangle(1430, 404, 204, 30, DARKGRAY);
    DrawRectangle(1435, 409, 194, 20, RED);
    DrawRectangle(1435, 409, 194 * ((float)enemy->current_health_value / (float)enemy->max_health), 20, GREEN);

    EndDrawing();
}

// ataque do inimigo no combate
void enemyAttack(GameInfo *game, Entity *player, Infmon *enemy, CombatStats *combatInfo)
{
    int randomValueToAttack = GetRandomValue(1, 3);
    float damageMultiplier;

    strcpy(combatInfo->enemyText, enemy->name);
    strcat(combatInfo->enemyText, " usou ");

    switch (randomValueToAttack)
    {
    case 1:
        damageMultiplier = getTypeDamageMultiplier(enemy->habilities[0].type, player->mon[game->choosenInfmon].infmon_type);
        player->mon[game->choosenInfmon].current_health_value -= (int)(damageMultiplier * enemy->habilities[0].multiplier * enemy->attack) / player->mon[game->choosenInfmon].defense;
        strcat(combatInfo->enemyText, enemy->habilities[0].attack);
        break;

    case 2:
        damageMultiplier = getTypeDamageMultiplier(enemy->habilities[1].type, player->mon[game->choosenInfmon].infmon_type);
        player->mon[game->choosenInfmon].current_health_value -= (int)(damageMultiplier * enemy->habilities[1].multiplier * enemy->attack) / player->mon[game->choosenInfmon].defense;
        strcat(combatInfo->enemyText, enemy->habilities[1].attack);
        break;

    case 3:
        damageMultiplier = getTypeDamageMultiplier(enemy->habilities[2].type, player->mon[game->choosenInfmon].infmon_type);
        player->mon[game->choosenInfmon].current_health_value -= (int)(damageMultiplier * enemy->habilities[2].multiplier * enemy->attack) / player->mon[game->choosenInfmon].defense;
        strcat(combatInfo->enemyText, enemy->habilities[2].attack);
        break;
    }

    combatInfo->infmonTurn = 0;
}

void loseGame(GameInfo *game, int *choose)
{
    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
        {
            *choose = 1;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 1)
        {
            *choose = 0;
        }
        else
        {
            *choose += 1;
        }
    }

    if (IsKeyPressed(KEY_B) || (IsKeyPressed(KEY_ENTER) && *choose == 0))
    {
        game->game_situation = MAIN_MENU;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
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
    DrawText("Você perdeu", SCREEN_WIDTH / 2 - 150, 180, 50, WHITE);
    DrawText("Voltar ao menu principal (B)", SCREEN_WIDTH / 2 - 350, 350, 50, WHITE);
    DrawText("Sair do jogo (Q)", SCREEN_WIDTH / 2 - 190, 500, 50, WHITE);

    EndDrawing();
}

// tela de que matou o boss final e ganhou o jogo
void wonGame(GameInfo *game, int *choose)
{
    // sobe a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_UP))
    {
        if (*choose == 0)
        {
            *choose = 1;
        }
        else
        {
            *choose -= 1;
        }
    }

    // desce a seleção do botão do menu com as setas
    if (IsKeyPressed(KEY_DOWN))
    {
        if (*choose == 1)
        {
            *choose = 0;
        }
        else
        {
            *choose += 1;
        }
    }

    if (IsKeyPressed(KEY_B) || (IsKeyPressed(KEY_ENTER) && *choose == 0))
    {
        game->game_situation = MAIN_MENU;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && *choose == 1))
    {
        game->exit_requested = TRUE;
        *choose = 0;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

    // desenha o quadrado de escolha
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 450 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 600 + 150 * *choose, 910, 10, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 - 440, 450 + 150 * *choose, 10, 160, WHITE);
    DrawRectangle(SCREEN_WIDTH / 2 + 460, 450 + 150 * *choose, 10, 160, WHITE);

    // escreve os textos de opcoes
    DrawText("Parabéns!", SCREEN_WIDTH / 2 - 150, 180, 50, WHITE);
    DrawText("Você derrotou ThiagoToise e salvou o mundo de suas atividades práticas!", SCREEN_WIDTH / 2 - 800, 350, 45, WHITE);
    DrawText("Voltar ao menu principal (B)", SCREEN_WIDTH / 2 - 350, 500, 50, WHITE);
    DrawText("Sair do jogo (Q)", SCREEN_WIDTH / 2 - 190, 650, 50, WHITE);

    EndDrawing();
}

// cura os infmons
void healInfmon(Entity *player)
{
    player->mon[0].current_health_value = player->mon[0].max_health;
    player->mon[1].current_health_value = player->mon[1].max_health;
    player->mon[2].current_health_value = player->mon[2].max_health;
}

// verifica se um infmon subiu de nível, se sim, mostra na tela que ele subiu de nivel
int levelUpInfmon(Entity *player, int selectedInfmon)
{
    // texto de levelup
    char levelUpText[AVG_TEXT_SIZE] = "";

    strcat(levelUpText, "Seu ");
    strcat(levelUpText, player->mon[selectedInfmon].name);
    strcat(levelUpText, " subiu de nível!");

    while (player->mon[selectedInfmon].current_xp >= player->mon[selectedInfmon].level_up_xp_threshold)
    {
        // melhora os atributos
        player->mon[selectedInfmon].level++;
        player->mon[selectedInfmon].attack += DANO_POR_NIVEL;
        player->mon[selectedInfmon].defense += DEFESA_POR_NIVEL;
        player->mon[selectedInfmon].current_xp -= player->mon[selectedInfmon].level_up_xp_threshold;
        player->mon[selectedInfmon].max_health += VIDA_POR_NIVEL;
        player->mon[selectedInfmon].level_up_xp_threshold += XP_THRESHOLD_POR_NIVEL;

        // tela de que upou
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
        DrawText(levelUpText, SCREEN_WIDTH / 2 - 400, 400, 50, WHITE);
        EndDrawing();
        for (int i = 0; i < 2000000000; i++)
            ;
        healInfmon(player);
        return 1;
    }
    return 0;
}

// pega o quanto de dano que vai dar com base no tipo
float getTypeDamageMultiplier(char attackingType, char defenderType)
{
    float val;

    switch (attackingType)
    {
    case 'f':
        switch (defenderType)
        {
        case 'f':
            val = TIPO_NEUTRO;
            break;

        case 'w':
            val = NERF_TIPO;
            break;

        case 'g':
            val = BUFF_TIPO;
        }
        break;

    case 'w':
        switch (defenderType)
        {
        case 'f':
            val = BUFF_TIPO;
            break;

        case 'w':
            val = TIPO_NEUTRO;
            break;

        case 'g':
            val = NERF_TIPO;
        }
        break;

    case 'g':
        switch (defenderType)
        {
        case 'f':
            val = NERF_TIPO;
            break;

        case 'w':
            val = BUFF_TIPO;
            break;

        case 'g':
            val = TIPO_NEUTRO;
        }
        break;

    case 'n':
        val = TIPO_NEUTRO;
    }

    return val;
}

// função que desenha o mapa
void drawMap(Maps *m, char map[COLUMNS][ROWS], int numMap)
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
                switch (numMap)
                {
                case 1:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                    break;
                case 2:
                case 3:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, RED);
                    break;
                case 4:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, BLUE);
                    break;
                default:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                    break;
                }
                break;

            case 'E':
                switch (numMap)
                {
                case 1:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                    break;
                case 2:
                case 3:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, RED);
                    break;
                case 4:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, BLUE);
                    break;
                default:
                    DrawRectangle(i * SQUARE_WIDTH, j * SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, GREEN);
                    break;
                }

                DrawTexture(m->boss, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;

            case 'G':
                DrawTexture(m->bush, i * SQUARE_WIDTH, j * SQUARE_WIDTH, WHITE);
                break;
            }
        }
    }
}

// abre o menu de infmons
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
        if (player->mon[i].level != NIVEL_ESPACO_VAZIO)
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

    saveArq = fopen("saves/save.bin", "rb");

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

    saveArq = fopen("saves/save.bin", "wb");

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

    // cria as informacoes iniciais do player e do mapa ao criar um save novo
    data->mapNum = 1;
    data->Player_info.mon[1].level = NIVEL_ESPACO_VAZIO;
    data->Player_info.mon[2].level = NIVEL_ESPACO_VAZIO;
    strcpy(data->Player_info.mon[1].name, "");
    strcpy(data->Player_info.mon[2].name, "");

    // -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@- TESTE -@-@-@-@-@-@-@-@-@-@-@-@-

    // define o infmon inicial do player
    data->Player_info.mon[0].current_xp = 0;
    data->Player_info.mon[0].level = 1;
    data->Player_info.mon[0].max_health = data->Player_info.mon[0].current_health_value = VIDA_INICIAL_LVL1;
    data->Player_info.mon[0].level_up_xp_threshold = XP_THRESHOLD_INICIAL;
    data->Player_info.mon[0].attack = DANO_INICIAL_LVL1;
    data->Player_info.mon[0].defense = DEFESA_INICIAL_LVL1;
    data->Player_info.mon[0].infmon_type = 'f';
    strcpy(data->Player_info.mon[0].name, "Fogomon");

    strcpy(data->Player_info.mon[0].habilities[0].attack, "LANÇA-CHAMAS");
    data->Player_info.mon[0].habilities[0].type = 'f';
    data->Player_info.mon[0].habilities[0].multiplier = 1;
    strcpy(data->Player_info.mon[0].habilities[1].attack, "SOCO DIRETO");
    data->Player_info.mon[0].habilities[1].type = 'n';
    data->Player_info.mon[0].habilities[1].multiplier = STD_ATK2_DMG;
    strcpy(data->Player_info.mon[0].habilities[2].attack, "BAFO DE FOGO");
    data->Player_info.mon[0].habilities[2].type = 'f';
    data->Player_info.mon[0].habilities[2].multiplier = STD_ATK3_DMG;

    saveArq = fopen("saves/save.bin", "wb");

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

// Função que gera encontro aleatório
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

// Função que determina se jogador está passando por zona de encontro aleatório
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

    randomEnemy.level = GetRandomValue(1, 10);
    randomEnemy.max_health = randomEnemy.current_health_value = VIDA_INICIAL_LVL1 + VIDA_POR_NIVEL * (randomEnemy.level - 1);
    randomEnemy.attack = DANO_INICIAL_LVL1 + DANO_POR_NIVEL * (randomEnemy.level - 1);
    randomEnemy.defense = DEFESA_INICIAL_LVL1 + (randomEnemy.level - 1);

    randomValueToType = GetRandomValue(1, 3);

    switch (randomValueToType)
    {
    case 1:
        randomEnemy.infmon_type = 'f';
        strcpy(randomEnemy.name, "Fogomon");
        strcpy(randomEnemy.habilities[0].attack, "BAFO DE FOGO");
        randomEnemy.habilities[0].type = 'f';
        randomEnemy.habilities[0].multiplier = 1;
        strcpy(randomEnemy.habilities[1].attack, "TAPA NA CARA");
        randomEnemy.habilities[1].type = 'n';
        randomEnemy.habilities[1].multiplier = STD_ATK2_DMG;
        strcpy(randomEnemy.habilities[2].attack, "LANÇA-CHAMAS");
        randomEnemy.habilities[2].type = 'f';
        randomEnemy.habilities[2].multiplier = STD_ATK3_DMG;
        break;

    case 2:
        randomEnemy.infmon_type = 'w';
        strcpy(randomEnemy.name, "Águamon");
        strcpy(randomEnemy.habilities[0].attack, "JATO D'ÁGUA");
        randomEnemy.habilities[0].type = 'w';
        randomEnemy.habilities[0].multiplier = 1;
        strcpy(randomEnemy.habilities[1].attack, "SOCO DIRETO");
        randomEnemy.habilities[1].type = 'n';
        randomEnemy.habilities[1].multiplier = STD_ATK2_DMG;
        strcpy(randomEnemy.habilities[2].attack, "CACHOEIRA");
        randomEnemy.habilities[2].type = 'w';
        randomEnemy.habilities[2].multiplier = STD_ATK3_DMG;
        break;

    case 3:
        randomEnemy.infmon_type = 'g';
        strcpy(randomEnemy.name, "Gramamon");
        strcpy(randomEnemy.habilities[0].attack, "FOTOSSÍNTESE");
        randomEnemy.habilities[0].type = 'g';
        randomEnemy.habilities[0].multiplier = 1;
        strcpy(randomEnemy.habilities[1].attack, "SOCO DIRETO");
        randomEnemy.habilities[1].type = 'n';
        randomEnemy.habilities[1].multiplier = STD_ATK2_DMG;
        strcpy(randomEnemy.habilities[2].attack, "POLINIZAÇÃO");
        randomEnemy.habilities[2].type = 'g';
        randomEnemy.habilities[2].multiplier = STD_ATK3_DMG;
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
        if (player->mon[i].level == NIVEL_ESPACO_VAZIO)
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
    if (map[*pX / SQUARE_WIDTH][(*pY) / SQUARE_WIDTH] == 'E')
    {
        game->game_situation = COMBATE;
    }

    return moving;
}