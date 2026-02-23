#include "battleship.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

static void *xcalloc(size_t n, size_t s) {
    void *p = calloc(n, s);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

static void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return q;
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = (char *)xmalloc(n);
    memcpy(d, s, n);
    return d;
}

int game_total_ships(int rows, int cols) {
    // kept compatible with original formula
    int area = rows * cols;
    return area / 20 + area / 30 + area / 40 + area / 55 + area / 70;
}

static void player_init(Player *p, const char *name, int rows, int cols) {
    memset(p, 0, sizeof(*p));
    p->name = xstrdup(name);

    // 1-indexed grids: allocate rows+1 pointers, each cols+1
    p->grid = (char **)xcalloc((size_t)rows + 1, sizeof(char *));
    p->attacked = (bool **)xcalloc((size_t)rows + 1, sizeof(bool *));
    for (int r = 0; r <= rows; r++) {
        p->grid[r] = (char *)xcalloc((size_t)cols + 1, sizeof(char));
        p->attacked[r] = (bool *)xcalloc((size_t)cols + 1, sizeof(bool));
    }

    p->attacks_cap = 64;
    p->attacks = (Coord *)xmalloc(p->attacks_cap * sizeof(Coord));
    p->attacks_len = 0;

    p->ship_count = game_total_ships(rows, cols);
    p->ships = (Ship *)xcalloc((size_t)p->ship_count, sizeof(Ship));
}

static void player_destroy(Player *p, int rows) {
    if (!p) return;

    if (p->grid) {
        for (int r = 0; r <= rows; r++) free(p->grid[r]);
        free(p->grid);
    }
    if (p->attacked) {
        for (int r = 0; r <= rows; r++) free(p->attacked[r]);
        free(p->attacked);
    }

    if (p->ships) {
        for (int i = 0; i < p->ship_count; i++) free(p->ships[i].name);
        free(p->ships);
    }

    free(p->attacks);
    free(p->name);

    memset(p, 0, sizeof(*p));
}

static bool in_bounds(int r, int c, int rows, int cols) {
    return r >= 1 && r <= rows && c >= 1 && c <= cols;
}

static bool ship_fits(const Ship *s, int rows, int cols) {
    if (s->length <= 0) return false;
    if (!in_bounds(s->start.row, s->start.col, rows, cols)) return false;

    if (s->orientation == 'H') {
        return s->start.col + s->length - 1 <= cols;
    }
    if (s->orientation == 'V') {
        return s->start.row + s->length - 1 <= rows;
    }
    return false;
}

static bool ship_overlaps(const Player *p, const Ship *s) {
    int r = s->start.row;
    int c = s->start.col;

    for (int i = 0; i < s->length; i++) {
        int rr = (s->orientation == 'V') ? r + i : r;
        int cc = (s->orientation == 'H') ? c + i : c;
        if (p->grid[rr][cc] != 0) return true;
    }
    return false;
}

bool place_ship(Player *p, const Ship *s, int rows, int cols) {
    if (!ship_fits(s, rows, cols)) return false;
    if (ship_overlaps(p, s)) return false;

    int r = s->start.row;
    int c = s->start.col;

    for (int i = 0; i < s->length; i++) {
        int rr = (s->orientation == 'V') ? r + i : r;
        int cc = (s->orientation == 'H') ? c + i : c;
        p->grid[rr][cc] = s->id;
    }

    return true;
}

bool ship_is_sunk(const Player *enemy, int rows, int cols, char ship_id) {
    for (int r = 1; r <= rows; r++)
        for (int c = 1; c <= cols; c++)
            if (enemy->grid[r][c] == ship_id)
                return false;
    return true;
}

bool all_ships_sunk(const Player *enemy, int rows, int cols) {
    for (int r = 1; r <= rows; r++)
        for (int c = 1; c <= cols; c++)
            if (enemy->grid[r][c] != 0 && enemy->grid[r][c] != 'x')
                return false;
    return true;
}

static void print_board(const Player *p, int rows, int cols, const char *label) {
    printf("%s %s:\n", label, p->name);
    for (int r = 1; r <= rows; r++) {
        for (int c = 1; c <= cols; c++) {
            char cell = p->grid[r][c];
            if (cell == 0) {
                printf(". ");
            } else if (cell == 'x') {
                printf("x ");
            } else {
                printf("%c ", cell);
            }
        }
        printf("\n");
    }
}

void game_print_boards(const Game *g) {
    print_board(&g->p1, g->rows, g->cols, "Harta jucatorului");
    print_board(&g->p2, g->rows, g->cols, "Harta jucatorului");
}

static bool read_ship(Ship *s) {
    char name_buf[128];
    if (scanf("%127s", name_buf) != 1) return false;

    s->name = xstrdup(name_buf);
    if (scanf(" %c", &s->id) != 1) return false;
    if (scanf("%d", &s->length) != 1) return false;

    int x, y;
    if (scanf("%d %d", &x, &y) != 2) return false;
    s->start.row = x;
    s->start.col = y;

    if (scanf(" %c", &s->orientation) != 1) return false;

    if (s->orientation == 'h') s->orientation = 'H';
    if (s->orientation == 'v') s->orientation = 'V';

    return true;
}

bool game_read_setup(Game *g) {
    if (scanf("%d %d", &g->rows, &g->cols) != 2) return false;

    player_init(&g->p1, "1", g->rows, g->cols);
    player_init(&g->p2, "2", g->rows, g->cols);

    for (int i = 0; i < g->p1.ship_count; i++) {
        Ship s = {0};
        if (!read_ship(&s)) return false;

        if (!place_ship(&g->p1, &s, g->rows, g->cols)) {
            fprintf(stderr,
                    "Invalid ship placement for player 1: %s (%c)\n",
                    s.name ? s.name : "(null)", s.id);
            g->p1.ships[i] = s;
            return false;
        }
        g->p1.ships[i] = s;
    }

    for (int i = 0; i < g->p2.ship_count; i++) {
        Ship s = {0};
        if (!read_ship(&s)) return false;

        if (!place_ship(&g->p2, &s, g->rows, g->cols)) {
            fprintf(stderr,
                    "Invalid ship placement for player 2: %s (%c)\n",
                    s.name ? s.name : "(null)", s.id);
            g->p2.ships[i] = s;
            return false;
        }
        g->p2.ships[i] = s;
    }

    return true;
}

static bool read_attack(Coord *c) {
    int x, y;
    if (scanf("%d %d", &x, &y) != 2) return false;
    c->row = x;
    c->col = y;
    return true;
}

static void record_attack(Player *attacker, Coord c) {
    if (attacker->attacks_len == attacker->attacks_cap) {
        attacker->attacks_cap *= 2;
        attacker->attacks = (Coord *)xrealloc(attacker->attacks, attacker->attacks_cap * sizeof(Coord));
    }
    attacker->attacks[attacker->attacks_len++] = c;
}

static void process_attack(Game *g, Player *attacker, Player *enemy, Coord c) {
    if (!in_bounds(c.row, c.col, g->rows, g->cols)) {
        printf("Jucatorul %s a dat o lovitura in afara tablei la coordonatele (%d, %d).\n",
               attacker->name, c.row, c.col);
        return;
    }

    if (attacker->attacked[c.row][c.col]) {
        printf("Jucatorul %s a mai atacat coordonatele (%d, %d).\n",
               attacker->name, c.row, c.col);
        return;
    }

    attacker->attacked[c.row][c.col] = true;
    record_attack(attacker, c);

    char cell = enemy->grid[c.row][c.col];

    if (cell == 0 || cell == 'x') {
        printf("Jucatorul %s a ratat o lovitura la coordonatele (%d, %d).\n",
               attacker->name, c.row, c.col);
        return;
    }

    // hit ship segment
    char ship_id = cell;
    enemy->grid[c.row][c.col] = 'x';
    printf("Jucatorul %s a lovit o nava la coordonatele (%d, %d).\n",
           attacker->name, c.row, c.col);

    if (ship_is_sunk(enemy, g->rows, g->cols, ship_id)) {
        printf("Jucatorul %s a scufundat o nava!\n", attacker->name);
    }
}

void game_play(Game *g) {
    bool win1 = false, win2 = false;
    int turn = 0;

    while (!win1 && !win2) {
        Player *attacker = (turn % 2 == 0) ? &g->p1 : &g->p2;
        Player *enemy = (turn % 2 == 0) ? &g->p2 : &g->p1;

        Coord c;
        if (!read_attack(&c)) {
            // input finished
            break;
        }

        process_attack(g, attacker, enemy, c);

        if (all_ships_sunk(enemy, g->rows, g->cols)) {
            printf("Jucatorul %s a castigat jocul!\n", attacker->name);
            if (turn % 2 == 0) win1 = true;
            else win2 = true;
        }

        turn++;
    }
}

void game_destroy(Game *g) {
    player_destroy(&g->p1, g->rows);
    player_destroy(&g->p2, g->rows);
    memset(g, 0, sizeof(*g));
}
