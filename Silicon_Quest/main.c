//--------------------------------------------------
// Inclusão das bibliotecas do Allegro 5
//--------------------------------------------------
#include <allegro5/allegro.h>   // Biblioteca padrão allegro
#include <allegro5/allegro_font.h>  // Biblioteca para utilização de fontes
#include <allegro5/allegro_ttf.h>   // Biblioteca para utilização de fontes
#include <allegro5/allegro_primitives.h>    // Biblioteca para utilização de primitivos
#include <allegro5/allegro_image.h>         // Biblioteca para utilização de bitmaps
#include <allegro5/allegro_native_dialog.h> // Biblioteca para utilização de caixas de texto do SO

//--------------------------------------------------
// Inclusão das bibliotecas padrões C
//--------------------------------------------------
#include <stdio.h>      // Biblioteca utilizada para escrita em arquivo
#include <string.h>     // Biblioteca para a manipulação de Strings

//--------------------------------------------------
// Definição dos valores para o compilador
//--------------------------------------------------

// Definições da tela
#define FULLSCREEN      0
#define DISPLAY_WIDTH   1600
#define DISPLAY_HEIGHT  900

// Definições dos Timers
#define FPS             60
#define MOVEMENT_SPEED  300
#define MOVEMENT_STEP   1
#define MENU_SPEED      7

// Definições de desenho
#define SHOWMOUSE       1
#define SHOW_BORDER     1
#define SHOW_MAP_LIMITS 1

// Definições do mapa
#define MAX_COLUNAS 2500
#define MAX_LINHAS  2500
#define NUM_BLOCOS  6

// Definições das cores
#define COR_BORDAS  242, 210, 99
//#define COR_BORDAS  255, 255, 255
#define COR_LIMITS  255, 255, 255
#define COR_AR      0, 0, 0
#define COR_TERRA   94, 28, 13
#define COR_PEDRA   53, 53, 53
#define COR_SILICIO 249, 249, 249
#define COR_LAVA    255, 116, 21
#define COR_AGUA    0, 128, 255

//--------------------------------------------------
// Definição das variaveis globais para o Allegro
//--------------------------------------------------
ALLEGRO_TIMER *drawTimer = NULL;
ALLEGRO_TIMER *movementTimer = NULL;
ALLEGRO_TIMER *menuTimer = NULL;

ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_EVENT ev;

ALLEGRO_DISPLAY *display = NULL;

//--------------------------------------------------
// Definição das variaveis constantes globais
//--------------------------------------------------
const int blockHeight = 50;
const int blockWidth = 50;

//--------------------------------------------------
// Definição das structs globais
//--------------------------------------------------
struct Objeto
{
    int     x;
    int     y;
    float   force;
    bool    jump;
};

struct Posicao
{
    int     x;
    int     y;
    int     z;
};

struct matriz
{
    int     coluna;
    int     linha;
};

//--------------------------------------------------
// Definição das variaveis para INPUT
//--------------------------------------------------
bool keys[62] = {false};
enum keys {A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
           N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
           F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
           UP, DOWN, RIGHT, LEFT, TAB, SHIFT, CTRL, ALT, ENTER, BACKSPACE, SPACE, ESC, MOUSE_1, MOUSE_2
          };

struct Posicao mouse = {0, 0, 0};
int mouseWheelBefore = 0;

//--------------------------------------------------
// Definição das variaveis de estado
//--------------------------------------------------
bool done = false;
bool draw = false;
bool movement = false;
bool readMenu = false;

//--------------------------------------------------
// Definição das variaveis gerais globais
//--------------------------------------------------
char blocos[MAX_LINHAS][MAX_COLUNAS] = {{0}};
struct matriz mouseBlock = {0};
struct Posicao mapa = {0};
struct Posicao jogador = {((DISPLAY_WIDTH/2)-50), ((DISPLAY_HEIGHT/2)-50), 0};

int numColunas = 25;
int numLinhas = 25;

int selectedBlock = 1;

//--------------------------------------------------
// Definição das funções utilizadas
//--------------------------------------------------
int checkEvents();
void readInputs();
void saveMap();
int detectColisionLeft_Matriz(struct Posicao character, struct Posicao mapaPos, char *blockPos[]);

int main()
{
    //--------------------------------------------------
    // Setup do Allegro
    //--------------------------------------------------

    // Inicialização do allegro
    if(!al_init())
    {
        al_show_native_message_box(NULL, "ERRO", "Incapaz de inicializar o Allegro", NULL, NULL, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    // Inicialização dos módulos adicionados
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();

    // Setup do Display e opções de visualização
    if(FULLSCREEN)
        al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    if(!SHOWMOUSE)
        al_hide_mouse_cursor(display);

    if(!display)
    {
        al_show_native_message_box(NULL, "ERRO", "Incapaz de inicializar o Display", NULL, NULL, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    // Setup das Fontes
    ALLEGRO_FONT *arial_24 = al_load_font("arial.ttf", 24, 0);

    // Setup dos Timers
    drawTimer = al_create_timer(1.0 / FPS);
    movementTimer = al_create_timer(1.0 / MOVEMENT_SPEED);
    menuTimer = al_create_timer(1.0 / MENU_SPEED);


    // Setup dos Eventos
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(drawTimer));
    al_register_event_source(event_queue, al_get_timer_event_source(movementTimer));
    al_register_event_source(event_queue, al_get_timer_event_source(menuTimer));

    // Inicialização dos Timers
    al_start_timer(drawTimer);
    al_start_timer(movementTimer);
    al_start_timer(menuTimer);

    //--------------------------------------------------
    // Definição das variaveis de estado
    //--------------------------------------------------
    char selectedOption = 0;
    int gameState = 0;

    //--------------------------------------------------
    // Definição das variaveis gerais
    //--------------------------------------------------
    ALLEGRO_BITMAP *blocoTerra = NULL;
    ALLEGRO_BITMAP *blocoGrama = NULL;
    ALLEGRO_BITMAP *blocoPedra = NULL;
    ALLEGRO_BITMAP *blocoSilicio = NULL;
    ALLEGRO_BITMAP *blocoLava = NULL;
    ALLEGRO_BITMAP *blocoAgua = NULL;

    blocoTerra = al_load_bitmap("Bitmaps/Terra.bmp");
    blocoGrama = al_load_bitmap("Bitmaps/Grama.bmp");
    blocoPedra = al_load_bitmap("Bitmaps/Pedra.bmp");
    blocoSilicio = al_load_bitmap("Bitmaps/Silicio.bmp");
    blocoLava = al_load_bitmap("Bitmaps/Lava.bmp");
    blocoAgua = al_load_bitmap("Bitmaps/Agua.bmp");

    struct Posicao source = {al_get_bitmap_width(blocoTerra), al_get_bitmap_height(blocoTerra), 0};

    //--------------------------------------------------
    // Definição das variaveis auxiliares
    //--------------------------------------------------
    int j, i;
    bool colisionLeft   = 0;
    bool colisionRight  = 0;
    bool colisionUp     = 0;
    bool colisionDown   = 0;

    //--------------------------------------------------
    // Loop para o jogo
    //--------------------------------------------------
    while(!done)
    {
        al_wait_for_event(event_queue, &ev);
        done = checkEvents();

        if(keys[ESC])
            done = true;

        if(gameState == 0)
        {
            if(draw)
            {
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, "        NEW MAP");
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, "        QUIT  ");

                if(selectedOption == 0)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, ">                           <");
                }
                else
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, ">                           <");
                }

                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));

                draw = false;
            }

            if(readMenu)
            {
                if(keys[UP] | keys[DOWN] | keys[W] | keys[S])
                {
                    selectedOption = !selectedOption;
                    readMenu = false;
                }

                if(keys[ENTER])
                {
                    done = selectedOption;
                    selectedOption = 0;
                    gameState = 1;
                    readMenu = false;
                }
            }


        }

        if(gameState == 1)
        {
            if(draw)
            {
                if(numLinhas == 0)
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, "        LINHAS:     _");
                else
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, "        LINHAS:     %d_", numLinhas);
                if(numColunas == 0)
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, "        COLUNAS:    _");
                else
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, "        COLUNAS:    %d_", numColunas);

                if(selectedOption == 0)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, ">                                   <");
                }
                else if (selectedOption == 1)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, ">                                   <");
                }

                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));

                draw = false;
            }

            if(readMenu)
            {
                readMenu = false;

                if(selectedOption)
                {
                    if(keys[BACKSPACE])
                        numColunas = (numColunas/10);
                    else if((keys[A] || keys[LEFT]) && (numColunas>1))
                        numColunas--;
                    else if((keys[D] | keys[RIGHT]) && (numColunas < MAX_COLUNAS))
                        numColunas++;
                    else if(keys[N0])
                        numColunas = (numColunas * 10) + 0;
                    else if(keys[N1])
                        numColunas = (numColunas * 10) + 1;
                    else if(keys[N2])
                        numColunas = (numColunas * 10) + 2;
                    else if(keys[N3])
                        numColunas = (numColunas * 10) + 3;
                    else if(keys[N4])
                        numColunas = (numColunas * 10) + 4;
                    else if(keys[N5])
                        numColunas = (numColunas * 10) + 5;
                    else if(keys[N6])
                        numColunas = (numColunas * 10) + 6;
                    else if(keys[N7])
                        numColunas = (numColunas * 10) + 7;
                    else if(keys[N8])
                        numColunas = (numColunas * 10) + 8;
                    else if(keys[N9])
                        numColunas = (numColunas * 10) + 9;
                    else if(keys[UP] | keys[DOWN] | keys[W] | keys[S])
                        selectedOption = !selectedOption;
                    else
                        readMenu = true;
                    if(numColunas > MAX_COLUNAS)
                        numColunas = MAX_COLUNAS;
                }
                else
                {
                    if(keys[BACKSPACE])
                        numLinhas = (numLinhas/10);
                    else if((keys[A] | keys[LEFT]) && (numLinhas > 1))
                        numLinhas--;
                    else if((keys[D] | keys[RIGHT]) && (numLinhas < MAX_LINHAS))
                        numLinhas++;
                    else if(keys[N0])
                        numLinhas = (numLinhas * 10) + 0;
                    else if(keys[N1])
                        numLinhas = (numLinhas * 10) + 1;
                    else if(keys[N2])
                        numLinhas = (numLinhas * 10) + 2;
                    else if(keys[N3])
                        numLinhas = (numLinhas * 10) + 3;
                    else if(keys[N4])
                        numLinhas = (numLinhas * 10) + 4;
                    else if(keys[N5])
                        numLinhas = (numLinhas * 10) + 5;
                    else if(keys[N6])
                        numLinhas = (numLinhas * 10) + 6;
                    else if(keys[N7])
                        numLinhas = (numLinhas * 10) + 7;
                    else if(keys[N8])
                        numLinhas = (numLinhas * 10) + 8;
                    else if(keys[N9])
                        numLinhas = (numLinhas * 10) + 9;
                    else if(keys[UP] | keys[DOWN] | keys[W] | keys[S])
                        selectedOption = !selectedOption;
                    else
                        readMenu = true;
                    if(numLinhas > MAX_LINHAS)
                        numLinhas = MAX_LINHAS;
                }

                if(keys[ENTER])
                {
                    mapa.x = ((DISPLAY_WIDTH/2) - ((numColunas/2) * blockWidth));
                    mapa.y = ((DISPLAY_HEIGHT/2) - ((numLinhas/2) * blockHeight));
                    gameState = 2;
                }
            }

        }

        if(gameState == 2)
        {
            // READ MOUSE MOVEMENT (TO BLOCK LIMITS)
            if(mouse.x < 0)
                mouse.x = 0;
            if(mouse.y < 0)
                mouse.y = 0;

            if(mouse.x < mapa.x)
                mouse.x = mapa.x;
            if(mouse.y < mapa.y)
                mouse.y = mapa.y;

            if(mouse.x > DISPLAY_WIDTH)
                mouse.x = DISPLAY_WIDTH;
            if(mouse.y > DISPLAY_HEIGHT)
                mouse.y = DISPLAY_HEIGHT;

            if(mouse.x >= (mapa.x + (numColunas * blockWidth)))
                mouse.x = mapa.x + (numColunas * blockWidth) - 1;
            if(mouse.y >= (mapa.y + (numLinhas * blockHeight)))
                mouse.y = mapa.y + (numLinhas * blockHeight) - 1;

            mouseBlock.coluna = (mouse.x - mapa.x)/blockWidth;
            mouseBlock.linha = (mouse.y - mapa.y)/blockHeight;

            // READ MOUSE WHEEL MOVEMENT
            if(mouse.z > mouseWheelBefore)
            {
                mouseWheelBefore = mouse.z;
                selectedBlock++;
                if(selectedBlock >= NUM_BLOCOS)
                    selectedBlock = 1;
            }
            else if(mouse.z < mouseWheelBefore)
            {
                mouseWheelBefore = mouse.z;
                selectedBlock--;
                if(selectedBlock < 1)
                    selectedBlock = NUM_BLOCOS - 1;
            }

            if(keys[MOUSE_1])
                blocos[mouseBlock.linha][mouseBlock.coluna] = selectedBlock;
            if(keys[MOUSE_2])
                blocos[mouseBlock.linha][mouseBlock.coluna] = 0;

            if(movement)
            {
                // READ MOVEMENT KEYS (WASD + ARROWS)
                if((keys[LEFT] || keys[A]) && (mapa.x < 0))
                    mapa.x += MOVEMENT_STEP + (2 * keys[SHIFT]);
                if((keys[RIGHT] || keys[D]) && (((mapa.x + (numColunas * blockWidth)) > DISPLAY_WIDTH)))
                    mapa.x -= MOVEMENT_STEP + (2 * keys[SHIFT]);

                if((keys[UP] || keys[W]) && (mapa.y < 0))
                    mapa.y += MOVEMENT_STEP + (2 * keys[SHIFT]);
                if((keys[DOWN] || keys[S]) && (((mapa.y + (numLinhas * blockHeight)) > DISPLAY_HEIGHT)))
                    mapa.y -= MOVEMENT_STEP + (2 * keys[SHIFT]);

                movement = false;
            }

            if(keys[P])
                gameState = 3;
            if(draw)
            {
                draw = false;
                for(i = 0; i < numLinhas; i++)
                {
                    for(j = 0; j < numColunas; j++)
                        if(((mapa.x + (j * blockWidth) + blockWidth) >= 0)&&((mapa.y + (i * blockHeight) + blockHeight)>= 0)&&((mapa.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapa.y + i * blockHeight) < DISPLAY_HEIGHT))
                        {
                            switch(blocos[i][j])
                            {
                            case 0: // AR
                                al_draw_filled_rectangle(mapa.x + j * blockWidth, mapa.y + i * blockHeight, mapa.x + (j * blockWidth) + blockWidth, mapa.y + (i * blockHeight) + blockHeight, al_map_rgb(COR_AR));
                                break;
                            case 1: // TERRA
                                if(blocos[i-1][j] == 0)
                                    al_draw_scaled_bitmap(blocoGrama, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                else
                                    al_draw_scaled_bitmap(blocoTerra, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 2: // PEDRA
                                al_draw_scaled_bitmap(blocoPedra, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 3: // SILICIO
                                al_draw_scaled_bitmap(blocoSilicio, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 4: // LAVA
                                al_draw_scaled_bitmap(blocoLava, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 5: // AGUA
                                al_draw_scaled_bitmap(blocoAgua, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            }
                        }
                }

                // DRAW BORDERS
                if(SHOW_BORDER)
                {
                    al_draw_rectangle(mapa.x + mouseBlock.coluna * blockWidth, mapa.y + mouseBlock.linha * blockHeight, mapa.x + (mouseBlock.coluna * blockWidth) + blockWidth, mapa.y + (mouseBlock.linha * blockHeight) + blockHeight, al_map_rgb(COR_BORDAS), 1);
                    al_draw_rectangle(1, 1, DISPLAY_WIDTH, DISPLAY_HEIGHT, al_map_rgb(COR_BORDAS), 1);
                    if(SHOW_MAP_LIMITS)
                        al_draw_rectangle(mapa.x, mapa.y, (mapa.x + (numColunas * blockWidth)), (mapa.y + (numLinhas * blockHeight)), al_map_rgb(COR_LIMITS), 1);
                }

                // DRAW SELECTED BLOCK PREVIEW
                switch(selectedBlock)
                {
                case 1: // TERRA
                    if(blocos[i-1][j] == 0)
                        al_draw_scaled_bitmap(blocoGrama, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    else
                        al_draw_scaled_bitmap(blocoTerra, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 2: // PEDRA
                    al_draw_scaled_bitmap(blocoPedra, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 3: // SILICIO
                    al_draw_scaled_bitmap(blocoSilicio, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 4: // LAVA
                    al_draw_scaled_bitmap(blocoLava, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 5: // AGUA
                    al_draw_scaled_bitmap(blocoAgua, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                }

                if(SHOW_BORDER)
                    al_draw_rectangle(DISPLAY_WIDTH - (10 + blockWidth), 10, DISPLAY_WIDTH - 10, 10 + blockHeight, al_map_rgb(COR_BORDAS), 1);

                //FLIP BUFFERS========================
                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));
            }
        }

        if(gameState == 3)
        {
            if(draw)
            {
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, "        CONTINUE");
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, "        SAVE MAP");
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 150, 0, "        TEST MAP");
                al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 175, 0, "        QUIT  ");

                if(selectedOption == 0)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 100, 0, ">                           <");
                }
                else if(selectedOption == 1)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 125, 0, ">                           <");
                }
                else if(selectedOption == 2)
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 150, 0, ">                           <");
                }
                else
                {
                    al_draw_textf(arial_24, al_map_rgb(COR_BORDAS), 100, 175, 0, ">                           <");
                }

                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));

                draw = false;
            }

            if(readMenu)
            {
                readMenu = false;

                if(keys[UP] | keys[W])
                    selectedOption--;
                else if(keys[DOWN] | keys[S])
                    selectedOption++;
                else if(keys[ENTER])
                {
                    if(selectedOption == 0)
                        gameState = 2;
                    else if(selectedOption == 1)
                        saveMap();
                    else if(selectedOption == 2)
                        gameState = 4;
                    else
                        done = true;
                    selectedOption = 0;
                    keys[ENTER] = false;
                }
                else
                    readMenu = true;

                if(selectedOption > 3)
                    selectedOption = 0;

                if(selectedOption < 0)
                    selectedOption = 3;
            }
        }

        if(gameState == 4)  // Gamestate de teste
        {
            colisionLeft = detectColisionLeft_Matriz(jogador, mapa, blocos);
            colisionRight = detectColisionRight_Matriz(jogador, mapa, blocos);
            colisionUp = detectColisionUp_Matriz(jogador, mapa, blocos);
            colisionDown = detectColisionDown_Matriz(jogador, mapa, blocos);

            // READ MOUSE MOVEMENT (TO BLOCK LIMITS)
            if(mouse.x < 0)
                mouse.x = 0;
            if(mouse.y < 0)
                mouse.y = 0;

            if(mouse.x < mapa.x)
                mouse.x = mapa.x;
            if(mouse.y < mapa.y)
                mouse.y = mapa.y;

            if(mouse.x > DISPLAY_WIDTH)
                mouse.x = DISPLAY_WIDTH;
            if(mouse.y > DISPLAY_HEIGHT)
                mouse.y = DISPLAY_HEIGHT;

            if(mouse.x >= (mapa.x + (numColunas * blockWidth)))
                mouse.x = mapa.x + (numColunas * blockWidth) - 1;
            if(mouse.y >= (mapa.y + (numLinhas * blockHeight)))
                mouse.y = mapa.y + (numLinhas * blockHeight) - 1;

            mouseBlock.coluna = (mouse.x - mapa.x)/blockWidth;
            mouseBlock.linha = (mouse.y - mapa.y)/blockHeight;

            // READ MOUSE WHEEL MOVEMENT
            if(mouse.z > mouseWheelBefore)
            {
                mouseWheelBefore = mouse.z;
                selectedBlock++;
                if(selectedBlock >= NUM_BLOCOS)
                    selectedBlock = 1;
            }
            else if(mouse.z < mouseWheelBefore)
            {
                mouseWheelBefore = mouse.z;
                selectedBlock--;
                if(selectedBlock < 1)
                    selectedBlock = NUM_BLOCOS - 1;
            }

            if(keys[MOUSE_1])
                blocos[mouseBlock.linha][mouseBlock.coluna] = selectedBlock;
            if(keys[MOUSE_2])
                blocos[mouseBlock.linha][mouseBlock.coluna] = 0;

            if(movement)
            {
                // READ MOVEMENT KEYS (WASD + ARROWS)

                /*if((!(mapa.x >= 0))&&(jogador.x == ((DISPLAY_WIDTH/2)-(blockWidth/2)))&&(!colisionLeft))
                    mapa.x += (keys[LEFT] | keys[A]) * MOVEMENT_STEP;
                else if(jogador.x >= 0)
                {
                    jogador.x -= (keys[LEFT] | keys[A]) * MOVEMENT_STEP;
                    if(jogador.x < 0)
                        jogador.x = 0;
                    if((mapa.x <= ((-numColunas * blockWidth) + DISPLAY_WIDTH)) && (jogador.x < ((DISPLAY_WIDTH/2)-(blockWidth/2) + MOVEMENT_STEP)))
                        jogador.x = ((DISPLAY_WIDTH/2)-(blockWidth/2));
                }
                if((!(mapa.y >= 0))&&(jogador.y == ((DISPLAY_HEIGHT/2)-(blockHeight/2))))
                    mapa.y += (keys[UP] | keys[W]) * MOVEMENT_STEP;
                else if((jogador.y >= 0))
                {
                    jogador.y -= (keys[UP] | keys[W]) * MOVEMENT_STEP;
                    if(jogador.y < 0)
                        jogador.y = 0;
                    if((mapa.y <= ((-numLinhas * blockHeight) + DISPLAY_HEIGHT)) && (jogador.y < ((DISPLAY_HEIGHT/2)-(blockHeight/2) + MOVEMENT_STEP)))
                        jogador.y = ((DISPLAY_HEIGHT/2)-(blockHeight/2));
                }
                if((!(mapa.x <= ((-numColunas * blockWidth) + DISPLAY_WIDTH)))&&(jogador.x == ((DISPLAY_WIDTH/2)-(blockWidth/2))))
                    mapa.x -= (keys[RIGHT] | keys[D]) * MOVEMENT_STEP;
                else if(jogador.x < (DISPLAY_WIDTH - blockWidth))
                {
                    jogador.x += (keys[RIGHT] | keys[D]) * MOVEMENT_STEP;
                    if(jogador.x > ((DISPLAY_WIDTH - blockWidth)))
                        jogador.x = (DISPLAY_WIDTH - blockWidth);
                    if((mapa.x >= 0) && (jogador.x > ((DISPLAY_WIDTH/2)-(blockWidth/2) - MOVEMENT_STEP)))
                        jogador.x = ((DISPLAY_WIDTH/2)-(blockWidth/2));

                }
                if((!(mapa.y <= ((-numLinhas * blockHeight) + DISPLAY_HEIGHT)))&&(jogador.y == ((DISPLAY_HEIGHT/2)-(blockHeight/2))))
                    mapa.y -= (keys[DOWN] | keys[S]) * MOVEMENT_STEP;
                else if((jogador.y <= (DISPLAY_HEIGHT - blockHeight)))
                {
                    jogador.y += (keys[DOWN] | keys[S]) * MOVEMENT_STEP;
                    if(jogador.y > (DISPLAY_HEIGHT - blockHeight))
                        jogador.y = (DISPLAY_HEIGHT - blockHeight);
                    if((mapa.y >= 0) && ((jogador.y > ((DISPLAY_HEIGHT/2)-(blockHeight/2) - MOVEMENT_STEP))))
                        jogador.y = ((DISPLAY_HEIGHT/2)-(blockHeight/2));
                }*/

                if((keys[LEFT] || keys[A]) && (mapa.x < 0) && (!colisionLeft))
                    mapa.x += MOVEMENT_STEP + (2 * keys[SHIFT]);
                if((keys[RIGHT] || keys[D]) && (((mapa.x + (numColunas * blockWidth)) > DISPLAY_WIDTH)) && (!colisionRight))
                    mapa.x -= MOVEMENT_STEP + (2 * keys[SHIFT]);

                if((keys[UP] || keys[W]) && (mapa.y < 0) && (!colisionUp))
                    mapa.y += MOVEMENT_STEP + (2 * keys[SHIFT]);
                if((keys[DOWN] || keys[S]) && (((mapa.y + (numLinhas * blockHeight)) > DISPLAY_HEIGHT)) && (!colisionDown))
                    mapa.y -= MOVEMENT_STEP + (2 * keys[SHIFT]);


                movement = false;
            }

            if(keys[P])
                gameState = 3;
            if(draw)
            {
                draw = false;
                for(i = 0; i < numLinhas; i++)
                {
                    for(j = 0; j < numColunas; j++)
                        if(((mapa.x + (j * blockWidth) + blockWidth) >= 0)&&((mapa.y + (i * blockHeight) + blockHeight)>= 0)&&((mapa.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapa.y + i * blockHeight) < DISPLAY_HEIGHT))
                        {
                            switch(blocos[i][j])
                            {
                            case 0: // AR
                                al_draw_filled_rectangle(mapa.x + j * blockWidth, mapa.y + i * blockHeight, mapa.x + (j * blockWidth) + blockWidth, mapa.y + (i * blockHeight) + blockHeight, al_map_rgb(COR_AR));
                                break;
                            case 1: // TERRA
                                if(blocos[i-1][j] == 0)
                                    al_draw_scaled_bitmap(blocoGrama, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                else
                                    al_draw_scaled_bitmap(blocoTerra, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 2: // PEDRA
                                al_draw_scaled_bitmap(blocoPedra, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 3: // SILICIO
                                al_draw_scaled_bitmap(blocoSilicio, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 4: // LAVA
                                al_draw_scaled_bitmap(blocoLava, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            case 5: // AGUA
                                al_draw_scaled_bitmap(blocoAgua, 0, 0, source.x, source.y, mapa.x + j * blockWidth, mapa.y + i * blockHeight, blockWidth, blockHeight, 0);
                                break;
                            }
                        }
                }

                al_draw_filled_rectangle(jogador.x, jogador.y, jogador.x + blockWidth, jogador.y + blockHeight, al_map_rgb(COR_LIMITS));

                // DRAW BORDERS
                if(SHOW_BORDER)
                {
                    al_draw_rectangle(mapa.x + mouseBlock.coluna * blockWidth, mapa.y + mouseBlock.linha * blockHeight, mapa.x + (mouseBlock.coluna * blockWidth) + blockWidth, mapa.y + (mouseBlock.linha * blockHeight) + blockHeight, al_map_rgb(COR_BORDAS), 1);
                    al_draw_rectangle(1, 1, DISPLAY_WIDTH, DISPLAY_HEIGHT, al_map_rgb(COR_BORDAS), 1);
                    if(SHOW_MAP_LIMITS)
                        al_draw_rectangle(mapa.x, mapa.y, (mapa.x + (numColunas * blockWidth)), (mapa.y + (numLinhas * blockHeight)), al_map_rgb(COR_LIMITS), 1);
                }

                // DRAW SELECTED BLOCK PREVIEW
                switch(selectedBlock)
                {
                case 1: // TERRA
                    if(blocos[i-1][j] == 0)
                        al_draw_scaled_bitmap(blocoGrama, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    else
                        al_draw_scaled_bitmap(blocoTerra, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 2: // PEDRA
                    al_draw_scaled_bitmap(blocoPedra, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 3: // SILICIO
                    al_draw_scaled_bitmap(blocoSilicio, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 4: // LAVA
                    al_draw_scaled_bitmap(blocoLava, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                case 5: // AGUA
                    al_draw_scaled_bitmap(blocoAgua, 0, 0, source.x, source.y, DISPLAY_WIDTH - (10 + blockWidth), 10, blockWidth, blockHeight, 0);
                    break;
                }

                if(SHOW_BORDER)
                    al_draw_rectangle(DISPLAY_WIDTH - (10 + blockWidth), 10, DISPLAY_WIDTH - 10, 10 + blockHeight, al_map_rgb(COR_BORDAS), 1);

                //FLIP BUFFERS========================
                al_flip_display();
                al_clear_to_color(al_map_rgb(0,0,0));
            }
        }

        // Reset timers
        if(!readMenu)
            al_start_timer(menuTimer);
    }

    //--------------------------------------------------
    // Finalização do Allegro
    //--------------------------------------------------

    al_destroy_bitmap(blocoTerra);
    al_destroy_bitmap(blocoGrama);
    al_destroy_bitmap(blocoPedra);
    al_destroy_bitmap(blocoSilicio);
    al_destroy_bitmap(blocoLava);
    al_destroy_bitmap(blocoAgua);

    al_destroy_event_queue(event_queue);
    al_destroy_timer(drawTimer);
    al_destroy_timer(movementTimer);
    al_destroy_timer(menuTimer);
    al_destroy_display(display);

    return 0;
}

int checkEvents()
{
    switch(ev.type)
    {
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP:
    case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
    case ALLEGRO_EVENT_MOUSE_AXES:
        readInputs();
        break;
    case ALLEGRO_EVENT_TIMER:
        if(ev.timer.source == drawTimer)
            draw = true;
        else if(ev.timer.source == movementTimer)
            movement = true;
        else if(ev.timer.source == menuTimer)
        {
            al_stop_timer(menuTimer);
            readMenu = true;
        }
        break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
        return 1;
        break;
    }
    return 0;
}

void readInputs()
{
    // Leitura dos eixos do mouse
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES)
    {
        mouse.x = ev.mouse.x;
        mouse.y = ev.mouse.y;
        mouse.z = ev.mouse.z;
    }

    // Leitura dos botões do mouse
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
    {
        if(ev.mouse.button & 1)
            keys[MOUSE_1] = true;

        if(ev.mouse.button & 2)
            keys[MOUSE_2] = true;
    }
    else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
    {
        if(ev.mouse.button & 1)
            keys[MOUSE_1] = false;

        if(ev.mouse.button & 2)
            keys[MOUSE_2] = false;
    }
    // READ KEYBOARD INPUT
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        switch(ev.keyboard.keycode)
        {
        case ALLEGRO_KEY_A:
            keys[A] = true;
            break;
        case ALLEGRO_KEY_B:
            keys[B] = true;
            break;
        case ALLEGRO_KEY_C:
            keys[C] = true;
            break;
        case ALLEGRO_KEY_D:
            keys[D] = true;
            break;
        case ALLEGRO_KEY_E:
            keys[E] = true;
            break;
        case ALLEGRO_KEY_F:
            keys[F] = true;
            break;
        case ALLEGRO_KEY_G:
            keys[G] = true;
            break;
        case ALLEGRO_KEY_H:
            keys[H] = true;
            break;
        case ALLEGRO_KEY_I:
            keys[I] = true;
            break;
        case ALLEGRO_KEY_J:
            keys[J] = true;
            break;
        case ALLEGRO_KEY_K:
            keys[K] = true;
            break;
        case ALLEGRO_KEY_L:
            keys[L] = true;
            break;
        case ALLEGRO_KEY_M:
            keys[M] = true;
            break;
        case ALLEGRO_KEY_N:
            keys[N] = true;
            break;
        case ALLEGRO_KEY_O:
            keys[O] = true;
            break;
        case ALLEGRO_KEY_P:
            keys[P] = true;
            break;
        case ALLEGRO_KEY_Q:
            keys[Q] = true;
            break;
        case ALLEGRO_KEY_R:
            keys[R] = true;
            break;
        case ALLEGRO_KEY_S:
            keys[S] = true;
            break;
        case ALLEGRO_KEY_T:
            keys[T] = true;
            break;
        case ALLEGRO_KEY_U:
            keys[U] = true;
            break;
        case ALLEGRO_KEY_V:
            keys[V] = true;
            break;
        case ALLEGRO_KEY_W:
            keys[W] = true;
            break;
        case ALLEGRO_KEY_X:
            keys[X] = true;
            break;
        case ALLEGRO_KEY_Y:
            keys[Y] = true;
            break;
        case ALLEGRO_KEY_Z:
            keys[Z] = true;
            break;

        case ALLEGRO_KEY_0:
        case ALLEGRO_KEY_PAD_0:
            keys[N0] = true;
            break;
        case ALLEGRO_KEY_1:
        case ALLEGRO_KEY_PAD_1:
            keys[N1] = true;
            break;
        case ALLEGRO_KEY_2:
        case ALLEGRO_KEY_PAD_2:
            keys[N2] = true;
            break;
        case ALLEGRO_KEY_3:
        case ALLEGRO_KEY_PAD_3:
            keys[N3] = true;
            break;
        case ALLEGRO_KEY_4:
        case ALLEGRO_KEY_PAD_4:
            keys[N4] = true;
            break;
        case ALLEGRO_KEY_5:
        case ALLEGRO_KEY_PAD_5:
            keys[N5] = true;
            break;
        case ALLEGRO_KEY_6:
        case ALLEGRO_KEY_PAD_6:
            keys[N6] = true;
            break;
        case ALLEGRO_KEY_7:
        case ALLEGRO_KEY_PAD_7:
            keys[N7] = true;
            break;
        case ALLEGRO_KEY_8:
        case ALLEGRO_KEY_PAD_8:
            keys[N8] = true;
            break;
        case ALLEGRO_KEY_9:
        case ALLEGRO_KEY_PAD_9:
            keys[N9] = true;
            break;

        case ALLEGRO_KEY_F1:
            keys[F1] = true;
            break;
        case ALLEGRO_KEY_F2:
            keys[F2] = true;
            break;
        case ALLEGRO_KEY_F3:
            keys[F3] = true;
            break;
        case ALLEGRO_KEY_F4:
            keys[F4] = true;
            break;
        case ALLEGRO_KEY_F5:
            keys[F5] = true;
            break;
        case ALLEGRO_KEY_F6:
            keys[F6] = true;
            break;
        case ALLEGRO_KEY_F7:
            keys[F7] = true;
            break;
        case ALLEGRO_KEY_F8:
            keys[F8] = true;
            break;
        case ALLEGRO_KEY_F9:
            keys[F9] = true;
            break;
        case ALLEGRO_KEY_F10:
            keys[F10] = true;
            break;
        case ALLEGRO_KEY_F11:
            keys[F11] = true;
            break;
        case ALLEGRO_KEY_F12:
            keys[F12] = true;
            break;

        case ALLEGRO_KEY_UP:
            keys[UP] = true;
            break;
        case ALLEGRO_KEY_DOWN:
            keys[DOWN] = true;
            break;
        case ALLEGRO_KEY_RIGHT:
            keys[RIGHT] = true;
            break;
        case ALLEGRO_KEY_LEFT:
            keys[LEFT] = true;
            break;
        case ALLEGRO_KEY_TAB:
            keys[TAB] = true;
            break;
        case ALLEGRO_KEY_LSHIFT:
        case ALLEGRO_KEY_RSHIFT:
            keys[SHIFT] = true;
            break;
        case ALLEGRO_KEY_LCTRL:
        case ALLEGRO_KEY_RCTRL:
            keys[CTRL] = true;
            break;
        case ALLEGRO_KEY_ALT:
            keys[ALT] = true;
            break;
        case ALLEGRO_KEY_ENTER:
        case ALLEGRO_KEY_PAD_ENTER:
            keys[ENTER] = true;
            break;
        case ALLEGRO_KEY_BACKSPACE:
            keys[BACKSPACE] = true;
            break;
        case ALLEGRO_KEY_SPACE:
            keys[SPACE] = true;
            break;
        case ALLEGRO_KEY_ESCAPE:
            keys[ESC] = true;
            break;
        }
    }
    else if(ev.type == ALLEGRO_EVENT_KEY_UP)
    {
        switch(ev.keyboard.keycode)
        {
        case ALLEGRO_KEY_A:
            keys[A] = false;
            break;
        case ALLEGRO_KEY_B:
            keys[B] = false;
            break;
        case ALLEGRO_KEY_C:
            keys[C] = false;
            break;
        case ALLEGRO_KEY_D:
            keys[D] = false;
            break;
        case ALLEGRO_KEY_E:
            keys[E] = false;
            break;
        case ALLEGRO_KEY_F:
            keys[F] = false;
            break;
        case ALLEGRO_KEY_G:
            keys[G] = false;
            break;
        case ALLEGRO_KEY_H:
            keys[H] = false;
            break;
        case ALLEGRO_KEY_I:
            keys[I] = false;
            break;
        case ALLEGRO_KEY_J:
            keys[J] = false;
            break;
        case ALLEGRO_KEY_K:
            keys[K] = false;
            break;
        case ALLEGRO_KEY_L:
            keys[L] = false;
            break;
        case ALLEGRO_KEY_M:
            keys[M] = false;
            break;
        case ALLEGRO_KEY_N:
            keys[N] = false;
            break;
        case ALLEGRO_KEY_O:
            keys[O] = false;
            break;
        case ALLEGRO_KEY_P:
            keys[P] = false;
            break;
        case ALLEGRO_KEY_Q:
            keys[Q] = false;
            break;
        case ALLEGRO_KEY_R:
            keys[R] = false;
            break;
        case ALLEGRO_KEY_S:
            keys[S] = false;
            break;
        case ALLEGRO_KEY_T:
            keys[T] = false;
            break;
        case ALLEGRO_KEY_U:
            keys[U] = false;
            break;
        case ALLEGRO_KEY_V:
            keys[V] = false;
            break;
        case ALLEGRO_KEY_W:
            keys[W] = false;
            break;
        case ALLEGRO_KEY_X:
            keys[X] = false;
            break;
        case ALLEGRO_KEY_Y:
            keys[Y] = false;
            break;
        case ALLEGRO_KEY_Z:
            keys[Z] = false;
            break;

        case ALLEGRO_KEY_0:
        case ALLEGRO_KEY_PAD_0:
            keys[N0] = false;
            break;
        case ALLEGRO_KEY_1:
        case ALLEGRO_KEY_PAD_1:
            keys[N1] = false;
            break;
        case ALLEGRO_KEY_2:
        case ALLEGRO_KEY_PAD_2:
            keys[N2] = false;
            break;
        case ALLEGRO_KEY_3:
        case ALLEGRO_KEY_PAD_3:
            keys[N3] = false;
            break;
        case ALLEGRO_KEY_4:
        case ALLEGRO_KEY_PAD_4:
            keys[N4] = false;
            break;
        case ALLEGRO_KEY_5:
        case ALLEGRO_KEY_PAD_5:
            keys[N5] = false;
            break;
        case ALLEGRO_KEY_6:
        case ALLEGRO_KEY_PAD_6:
            keys[N6] = false;
            break;
        case ALLEGRO_KEY_7:
        case ALLEGRO_KEY_PAD_7:
            keys[N7] = false;
            break;
        case ALLEGRO_KEY_8:
        case ALLEGRO_KEY_PAD_8:
            keys[N8] = false;
            break;
        case ALLEGRO_KEY_9:
        case ALLEGRO_KEY_PAD_9:
            keys[N9] = false;
            break;

        case ALLEGRO_KEY_F1:
            keys[F1] = false;
            break;
        case ALLEGRO_KEY_F2:
            keys[F2] = false;
            break;
        case ALLEGRO_KEY_F3:
            keys[F3] = false;
            break;
        case ALLEGRO_KEY_F4:
            keys[F4] = false;
            break;
        case ALLEGRO_KEY_F5:
            keys[F5] = false;
            break;
        case ALLEGRO_KEY_F6:
            keys[F6] = false;
            break;
        case ALLEGRO_KEY_F7:
            keys[F7] = false;
            break;
        case ALLEGRO_KEY_F8:
            keys[F8] = false;
            break;
        case ALLEGRO_KEY_F9:
            keys[F9] = false;
            break;
        case ALLEGRO_KEY_F10:
            keys[F10] = false;
            break;
        case ALLEGRO_KEY_F11:
            keys[F11] = false;
            break;
        case ALLEGRO_KEY_F12:
            keys[F12] = false;
            break;

        case ALLEGRO_KEY_UP:
            keys[UP] = false;
            break;
        case ALLEGRO_KEY_DOWN:
            keys[DOWN] = false;
            break;
        case ALLEGRO_KEY_RIGHT:
            keys[RIGHT] = false;
            break;
        case ALLEGRO_KEY_LEFT:
            keys[LEFT] = false;
            break;
        case ALLEGRO_KEY_TAB:
            keys[TAB] = false;
            break;
        case ALLEGRO_KEY_LSHIFT:
        case ALLEGRO_KEY_RSHIFT:
            keys[SHIFT] = false;
            break;
        case ALLEGRO_KEY_LCTRL:
        case ALLEGRO_KEY_RCTRL:
            keys[CTRL] = false;
            break;
        case ALLEGRO_KEY_ALT:
            keys[ALT] = false;
            break;
        case ALLEGRO_KEY_ENTER:
        case ALLEGRO_KEY_PAD_ENTER:
            keys[ENTER] = false;
            break;
        case ALLEGRO_KEY_BACKSPACE:
            keys[BACKSPACE] = false;
            break;
        case ALLEGRO_KEY_SPACE:
            keys[SPACE] = false;
            break;
        case ALLEGRO_KEY_ESCAPE:
            keys[ESC] = false;
            break;
        }
    }
}

void saveMap()
{
    //==============================================
    //PROJECT VARIABLES
    //==============================================
    FILE *fp;
    ALLEGRO_FILECHOOSER *file;

    //==============================================
    //AUXILIAR VARIABLES
    //==============================================
    int j, i;

    file = al_create_native_file_dialog("", "Choose File location and name", "*.txt",ALLEGRO_FILECHOOSER_MULTIPLE);
    al_show_native_file_dialog(display, file);
    char mapNameTxt[100] = "";

    if(al_get_native_file_dialog_count(file) != 0)
    {
        const char *mapName = al_get_native_file_dialog_path(file, 0);

        strcpy(mapNameTxt, mapName);

        if((mapNameTxt[strlen(mapNameTxt)-1] != 't')||(mapNameTxt[strlen(mapNameTxt)-2] != 'x')||(mapNameTxt[strlen(mapNameTxt)-3] != 't')||(mapNameTxt[strlen(mapNameTxt)-4] != '.'))
            strcat(mapNameTxt, ".txt");

        // SAVE MAP TO FILE
        fp = fopen(mapNameTxt, "w");
        fprintf(fp, "%d %d\n", numLinhas, numColunas);
        for(i = 0; i < numLinhas; i++)
        {
            for(j = 0; j < numColunas; j++)
                fprintf(fp, "%d ", blocos[i][j]);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
}

int detectColisionLeft_Matriz(struct Posicao character, struct Posicao mapaPos, char *blockPos[])
{
    int i, j;
    bool result = 0;

    for(i = 0; i < numLinhas; i++)
    {
        for(j = 0; j < numColunas; j++)
            if(((mapaPos.x + (j * blockWidth) + blockWidth) >= 0)&&((mapaPos.y + (i * blockHeight) + blockHeight)>= 0)&&((mapaPos.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapaPos.y + i * blockHeight) < DISPLAY_HEIGHT))
            {
                if((character.x == (mapaPos.x + ((1 + j) * blockWidth) + 1))
                        && ((((character.y) >= (mapaPos.y + (i * blockHeight)))&& ((character.y) <= (mapaPos.y + ((i + 1) * blockHeight))))
                            || ((character.y + blockHeight) >= (mapaPos.y + (i * blockHeight)) && ((character.y + blockHeight) <= (mapaPos.y + ((i + 1) * blockHeight))))))

                {
                    switch(blocos[i][j])
                    {
                    case 0: // AR
                        result |= 0;
                        break;
                    case 1: // TERRA
                    case 2: // PEDRA
                    case 3: // SILICIO
                        result |= 1;
                        break;
                    case 4: // LAVA
                    case 5: // AGUA
                        result |= 0;
                        break;
                    }
                }
            }
    }
    return result;
}

int detectColisionRight_Matriz(struct Posicao character, struct Posicao mapaPos, char *blockPos[])
{
    int i, j;
    bool result = 0;

    for(i = 0; i < numLinhas; i++)
    {
        for(j = 0; j < numColunas; j++)
            if(((mapaPos.x + (j * blockWidth) + blockWidth) >= 0)&&((mapaPos.y + (i * blockHeight) + blockHeight)>= 0)&&((mapaPos.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapaPos.y + i * blockHeight) < DISPLAY_HEIGHT))
            {
                if(((character.x + blockWidth) == (mapaPos.x + (j * blockWidth) - 1))
                        && ((((character.y) >= (mapaPos.y + (i * blockHeight)))&& ((character.y) <= (mapaPos.y + ((i + 1) * blockHeight))))
                            || ((character.y + blockHeight) >= (mapaPos.y + (i * blockHeight)) && ((character.y + blockHeight) <= (mapaPos.y + ((i + 1) * blockHeight))))))

                {
                    switch(blocos[i][j])
                    {
                    case 0: // AR
                        result |= 0;
                        break;
                    case 1: // TERRA
                    case 2: // PEDRA
                    case 3: // SILICIO
                        result |= 1;
                        break;
                    case 4: // LAVA
                    case 5: // AGUA
                        result |= 0;
                        break;
                    }
                }
            }
    }
    return result;
}

int detectColisionUp_Matriz(struct Posicao character, struct Posicao mapaPos, char *blockPos[])
{
    int i, j;
    bool result = 0;

    for(i = 0; i < numLinhas; i++)
    {
        for(j = 0; j < numColunas; j++)
            if(((mapaPos.x + (j * blockWidth) + blockWidth) >= 0)&&((mapaPos.y + (i * blockHeight) + blockHeight)>= 0)&&((mapaPos.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapaPos.y + i * blockHeight) < DISPLAY_HEIGHT))
            {
                if((character.y == (mapaPos.y + ((i + 1) * blockWidth) + 1))
                        && ((((character.x) >= (mapaPos.x + (j * blockHeight)))&& ((character.x) <= (mapaPos.x + ((j + 1) * blockHeight))))
                            || ((character.x + blockHeight) >= (mapaPos.x + (j * blockHeight)) && ((character.x + blockHeight) <= (mapaPos.x + ((j + 1) * blockHeight))))))

                {
                    switch(blocos[i][j])
                    {
                    case 0: // AR
                        result |= 0;
                        break;
                    case 1: // TERRA
                    case 2: // PEDRA
                    case 3: // SILICIO
                        result |= 1;
                        break;
                    case 4: // LAVA
                    case 5: // AGUA
                        result |= 0;
                        break;
                    }
                }
            }
    }
    return result;
}

int detectColisionDown_Matriz(struct Posicao character, struct Posicao mapaPos, char *blockPos[])
{
    int i, j;
    bool result = 0;

    for(i = 0; i < numLinhas; i++)
    {
        for(j = 0; j < numColunas; j++)
            if(((mapaPos.x + (j * blockWidth) + blockWidth) >= 0)&&((mapaPos.y + (i * blockHeight) + blockHeight)>= 0)&&((mapaPos.x + j * blockWidth) < DISPLAY_WIDTH)&&((mapaPos.y + i * blockHeight) < DISPLAY_HEIGHT))
            {
                if(((character.y + blockHeight) == (mapaPos.y + (i * blockWidth) - 1))
                        && ((((character.x) >= (mapaPos.x + (j * blockHeight)))&& ((character.x) <= (mapaPos.x + ((j + 1) * blockHeight))))
                            || ((character.x + blockHeight) >= (mapaPos.x + (j * blockHeight)) && ((character.x + blockHeight) <= (mapaPos.x + ((j + 1) * blockHeight))))))

                {
                    switch(blocos[i][j])
                    {
                    case 0: // AR
                        result |= 0;
                        break;
                    case 1: // TERRA
                    case 2: // PEDRA
                    case 3: // SILICIO
                        result |= 1;
                        break;
                    case 4: // LAVA
                    case 5: // AGUA
                        result |= 0;
                        break;
                    }
                }
            }
    }
    return result;
}
