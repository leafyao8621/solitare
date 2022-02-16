#include <time.h>

#include <ncurses.h>

#include "../core/core.h"

#define GROUP_STOCK 0
#define GROUP_TALON 1
#define GROUP_FOUNDATION 2
#define GROUP_TABLEAU 3

static struct Game game;
static char group, position;

static void render_talon(void) {
    if (game.talon_ptr != game.talon) {
        mvprintw(1, 8, "%c%2s",
                 suite_lookup[(unsigned long)(game.talon_ptr[-1] >> 4)],
                 rank_lookup[(unsigned long)(game.talon_ptr[-1] & 0xf)]);
    } else {
        mvaddstr(1, 8, "---");
    }
}

static void render_tableau(void) {
    for (char i = 0; i < 7; ++i) {
        for (char j = 0, *jj = game.tableau[(unsigned long)i];
             j < 13;
             ++j, ++jj) {
            if (*jj != 0xff) {
                if (*jj & 0x80) {
                    mvaddstr(2 + j, i << 2, "---");
                } else {
                    mvprintw(2 + j, i << 2, "%c%2s",
                             suite_lookup[(unsigned long)((*jj & 0x7f) >> 4)],
                             rank_lookup[(unsigned long)((*jj & 0x7f) & 0xf)]);
                }
            }
        }
    }
}

void controller_initialize(void) {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    core_initialize(&game, 1000);
    for (int i = 0; i < 20; ++i) {
        core_draw(&game);
    }
    mvaddstr(0, 0, "Solitare");
    mvaddstr(1, 0, "Stock");
    render_talon();
    render_tableau();
    move(1, 0);
    group = GROUP_STOCK;
    position = 0;
}

char controller_handle(void) {
    int in = getch();
    switch (in) {
    case 'Q':
    case 'q':
        return 1;
    }
    return 0;
}

void controller_finalize(void) {
    endwin();
}
