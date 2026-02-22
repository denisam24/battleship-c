#include "battleship.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    Game g;
    memset(&g, 0, sizeof(g));

    if (scanf("%d", &g.matches) != 1) return 0;

    for (int k = 0; k < g.matches; k++) {
        if (!game_read_setup(&g)) {
            game_destroy(&g);
            return 1;
        }

        game_print_boards(&g);
        game_play(&g);
        game_destroy(&g);
    }

    return 0;
}
