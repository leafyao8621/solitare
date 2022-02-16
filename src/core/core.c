#include <string.h>

#include "core.h"

#define SUITE_HEART 0
#define SUITE_DIAMOND 1
#define SUITE_SPADE 2
#define SUITE_CLUB 4

const char suite_lookup[4] = {
    '!',
    '@',
    '#',
    '$'
};

const char *rank_lookup[13] = {
    "A",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "J",
    "Q",
    "K"
};

static void swap(char *a, char *b) {
    char c = *a;
    *a = *b;
    *b = c;
}

int core_initialize(struct Game *game, unsigned seed) {
    if (!game) {
        return 1;
    }
    mt19937_initialize(&game->gen, seed);
    static char cards[52];
    for (char i = 0, *ii = cards; i < 4; ++i) {
        for (char j = 0; j < 13; ++j, ++ii) {
            *ii = (i << 4) | j;
        }
    }
    for (char i = 0, *ii = cards; i < 52; ++i, ++ii) {
        swap(ii, cards + mt19937_gen(&game->gen) % 52);
    }
    memcpy(game->stock, cards, 24);
    game->stock_ptr = game->stock;
    memset(game->talon, 0xff, 24);
    game->talon_ptr = game->talon;
    memset(game->foundations, -1, 4);
    for (char i = 0, *ii = cards + 24; i < 7; ++i) {
        for (char j = 0, *jj = game->tableau[(unsigned long)i];
             j < 13;
             ++j, ++jj) {
            if (j < i) {
                *jj = 0x80 | *ii;
                ++ii;
            } else if (j < i + 1) {
                *jj = *ii;
                ++ii;
            } else {
                *jj = 0xff;
            }

        }
    }
    return 0;
}

int core_draw(struct Game *game) {
    if (!game) {
        return 1;
    }
    if (game->stock == game->stock_ptr &&
        *game->stock_ptr == 0xff) {
        return 2;
    }
    if (game->stock_ptr - game->stock < 24 && *game->stock_ptr != -1) {
        *(game->talon_ptr++) = *game->stock_ptr;
        *(game->stock_ptr++) = -1;
    } else {
        memcpy(game->stock, game->talon, 24);
        memset(game->talon, -1, 24);
        game->stock_ptr = game->stock;
        game->talon_ptr = game->talon;
    }
    return 0;
}

int core_log(struct Game *game, FILE *fout) {
    if (!fout) {
        return 1;
    }
    puts("Stock:");
    for (char i = 0, *ii = game->stock; i < 24; ++i, ++ii) {
        if (*ii != 0xff) {
            printf("%c%2s\n",
                   suite_lookup[(unsigned long)(*ii >> 4)],
                   rank_lookup[(unsigned long)(*ii & 0xf)]);
        } else {
            puts("Empty");
        }
    }
    puts("Talon:");
    for (char i = 0, *ii = game->talon; i < 24; ++i, ++ii) {
        if (*ii != 0xff) {
            printf("%c%2s\n",
                   suite_lookup[(unsigned long)(*ii >> 4)],
                   rank_lookup[(unsigned long)(*ii & 0xf)]);
        } else {
            puts("Empty");
        }
    }
    puts("Tableau:");
    for (char i = 0; i < 7; ++i) {
        printf("%hhd:\n", i);
        for (char j = 0, *jj = game->tableau[(unsigned long)i];
             j < 13;
             ++j, ++jj) {
            if (*jj != 0xff) {
                printf("%c%2s",
                       suite_lookup[(unsigned long)((*jj & 0x7f) >> 4)],
                       rank_lookup[(unsigned long)((*jj & 0x7f) & 0xf)]);
                if (*jj & 0x80) {
                    puts(" Hidden");
                } else {
                    putchar(10);
                }
            } else {
                puts("Empty");
            }
        }
    }
    return 0;
}
