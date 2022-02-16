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

static void render_foundation(void) {
    for (char i = 0, *ii = game.foundations; i < 4; ++i, ++ii) {
        if (*ii != -1) {
            mvprintw(1, 12 + (i << 2), "%c%2s",
                     suite_lookup[(unsigned long)(*ii >> 4)],
                     rank_lookup[(unsigned long)(*ii & 0xf)]);
        } else {
            mvaddstr(1, 12 + (i << 2), "---");
        }
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
    mvaddstr(0, 0, "Solitare");
    mvaddstr(1, 0, "Stock");
    render_talon();
    render_foundation();
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
    switch (group) {
    case GROUP_STOCK:
        switch (in) {
        case 'Z':
        case 'z':
            core_draw(&game);
            render_talon();
            move(1, 0);
            break;
        case KEY_RIGHT:
            group = GROUP_TALON;
            move(1, 8);
            break;
        case KEY_DOWN:
            group = GROUP_TABLEAU;
            position = 0;
            move(2, 0);
            break;
        }
        break;
    case GROUP_TALON:
        switch (in) {
        case KEY_LEFT:
            group = GROUP_STOCK;
            move(1, 0);
            break;
        case KEY_RIGHT:
            group = GROUP_FOUNDATION;
            position = 0;
            move(1, 12);
            break;
        case KEY_DOWN:
            group = GROUP_TABLEAU;
            position = 0x20;
            move(2, 8);
            break;
        }
        break;
    case GROUP_FOUNDATION:
        switch (in) {
        case KEY_LEFT:
            if (!position) {
                group = GROUP_TALON;
                move(1, 8);
            } else {
                --position;
                move(1, 12 + (position << 2));
            }
            break;
        case KEY_RIGHT:
            if (position < 3) {
                ++position;
                move(1, 12 + (position << 2));
            }
            break;
        case KEY_DOWN:
            group = GROUP_TABLEAU;
            position += 3;
            position <<= 4;
            move(2, (position >> 4) << 2);
            break;
        }
        break;
    case GROUP_TABLEAU:
        switch (in) {
        case KEY_UP:
            if (position & 0xf) {
                --position;
                move(2 + (position & 0xf), position >> 4 << 2);
            } else {
                if ((position >> 4) < 2) {
                    group = GROUP_STOCK;
                    move(1, 0);
                } else if ((position >> 4) == 2) {
                    group = GROUP_TALON;
                    move(1, 8);
                } else {
                    group = GROUP_FOUNDATION;
                    position >>= 4;
                    position -= 3;
                    move(1, 12 + (position << 2));
                }
            }
            break;
        case KEY_RIGHT:
            if ((position >> 4) < 6) {
                position += 0x10;
                for (; (position & 0xf) &&
                       game.tableau[(unsigned long)(position >> 4)]
                                   [(unsigned long)(position & 0xf)] == 0xff;
                     --position);
                move(2 + (position & 0xf), position >> 4 << 2);
            }
            break;
        case KEY_LEFT:
            if (position >> 4) {
                position -= 0x10;
                for (; (position & 0xf) &&
                       game.tableau[(unsigned long)(position >> 4)]
                                   [(unsigned long)(position & 0xf)] == 0xff;
                     --position);
                move(2 + (position & 0xf), position >> 4 << 2);
            }
            break;
        case KEY_DOWN:
            if (position < 12 &&
                game.tableau[(unsigned long)((position + 1) >> 4)]
                            [(unsigned long)((position + 1) & 0xf)] != 0xff) {
                ++position;
                move(2 + (position & 0xf), position >> 4 << 2);
            }
        }
        break;
    }
    return 0;
}

void controller_finalize(void) {
    endwin();
}
