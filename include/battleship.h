#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int row;
    int col;
} Coord;

typedef struct {
    char *name;          // heap-allocated
    char id;             // printable identifier
    int length;
    Coord start;
    char orientation;   
} Ship;

typedef struct {
    char *name;          
    char **grid;         
    bool **attacked;     

    Ship *ships;
    int ship_count;

    Coord *attacks;      
    size_t attacks_len;
    size_t attacks_cap;
} Player;

typedef struct {
    int matches;
    int rows;
    int cols;

    Player p1;
    Player p2;
} Game;

// lifecycle
bool game_read_setup(Game *g);
void game_play(Game *g);
void game_print_boards(const Game *g);
void game_destroy(Game *g);

int game_total_ships(int rows, int cols);
bool place_ship(Player *p, const Ship *s, int rows, int cols);
bool ship_is_sunk(const Player *enemy, int rows, int cols, char ship_id);
bool all_ships_sunk(const Player *enemy, int rows, int cols);

#ifdef __cplusplus
}
#endif

#endif
