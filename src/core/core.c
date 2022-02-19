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

static void swap(unsigned char *a, unsigned char *b) {
    unsigned char c = *a;
    *a = *b;
    *b = c;
}

int core_initialize(struct Game *game, unsigned seed) {
    if (!game) {
        return 1;
    }
    mt19937_initialize(&game->gen, seed);
    static unsigned char cards[52];
    for (unsigned char i = 0, *ii = cards; i < 4; ++i) {
        for (unsigned char j = 0; j < 13; ++j, ++ii) {
            *ii = (i << 4) | j;
        }
    }
    for (unsigned char i = 0, *ii = cards; i < 52; ++i, ++ii) {
        swap(ii, cards + mt19937_gen(&game->gen) % 52);
    }
    memcpy(game->stock, cards, 24);
    game->stock_ptr = game->stock;
    memset(game->talon, -1, 24);
    game->talon_ptr = game->talon;
    memset(game->foundations, -1, 4);
    for (unsigned char i = 0, *ii = cards + 24; i < 7; ++i) {
        for (unsigned char j = 0, *jj = game->tableau[i];
             j < 20;
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
    if (game->stock_ptr - game->stock < 24 && *game->stock_ptr != 0xff) {
        *(game->talon_ptr++) = *game->stock_ptr;
        *(game->stock_ptr++) = 0xff;
    } else {
        memcpy(game->stock, game->talon, 24);
        memset(game->talon, -1, 24);
        game->stock_ptr = game->stock;
        game->talon_ptr = game->talon;
    }
    return 0;
}

int core_tableau_to_foundation(struct Game *game,
                               unsigned char oidx1,
                               unsigned char oidx2,
                               unsigned char didx) {
    if (!game) {
        return 1;
    }
    if (oidx1 > 6 ||
        oidx2 > 19 ||
        game->tableau[oidx1][oidx2] & 0x80 ||
        didx > 3) {
        return 2;
    }
    if (game->foundations[didx] != 0xff) {
        if (game->tableau[oidx1][oidx2] !=
            game->foundations[didx] + 1) {
            return 2;
        }
    } else {
        if ((game->tableau[oidx1][oidx2] >> 4) !=
            didx ||
            game->tableau[oidx1][oidx2] & 0xf) {
            return 2;
        }
    }
    game->foundations[didx] = game->tableau[oidx1][oidx2];
    game->tableau[oidx1][oidx2] = 0xff;
    if (oidx2) {
        if (game->tableau[oidx1][oidx2 - 1] & 0x80) {
            game->tableau[oidx1][oidx2 - 1] &= 0x7f;
        }
    }
    return 0;
}

int core_talon_to_foundation(struct Game *game,
                             unsigned char didx) {
    if (!game) {
        return 1;
    }
    if (game->talon_ptr == game->talon ||
        didx > 3) {
        return 2;
    }
    if (game->foundations[didx] != 0xff) {
        if (game->talon_ptr[-1] !=
            game->foundations[didx] + 1) {
            return 2;
        }
    } else {
        if ((game->talon_ptr[-1] >> 4) !=
            didx ||
            game->talon_ptr[-1] & 0xf) {
            return 2;
        }
    }
    game->foundations[didx] = game->talon_ptr[-1];
    *(--game->talon_ptr) = 0xff;
    return 0;
}

int core_talon_to_tableau(struct Game *game,
                          unsigned char didx1,
                          unsigned char didx2) {
    if (!game ||
        didx1 > 6 ||
        didx2 > 19) {
        return 1;
    }
    if (game->talon_ptr == game->talon) {
        return 2;
    }
    if (game->tableau[didx1][didx2] == 0xff &&
        (game->talon_ptr[-1] & 0xf) != 12) {
        return 2;
    }
    if ((game->tableau[didx1][didx2] != 0xff) &&
        (!((game->tableau[didx1][didx2] ^
            game->talon_ptr[-1]) & 0x20) ||
         ((game->tableau[didx1][didx2] & 0xf) !=
          ((game->talon_ptr[-1] & 0xf) + 1)))) {
        return 2;
    }
    if (game->tableau[didx1][didx2] != 0xff) {
        game->tableau[didx1][didx2 + 1] = *(--game->talon_ptr);
    } else {
        game->tableau[didx1][didx2] = *(--game->talon_ptr);
    }
    *game->talon_ptr = 0xff;
    return 0;
}

int core_tableau_to_tableau(struct Game *game,
                            unsigned char oidx1,
                            unsigned char oidx2,
                            unsigned char didx1,
                            unsigned char didx2) {
    if (!game ||
        oidx1 > 6 ||
        oidx2 > 19 ||
        didx1 > 6 ||
        didx2 > 19) {
        return 1;
    }
    if (game->tableau[oidx1][oidx2] & 0x80) {
        return 2;
    }
    if (((game->tableau[didx1][didx2] != 0xff) &&
         (!((game->tableau[didx1][didx2] ^
             game->tableau[oidx1][oidx2]) & 0x20) ||
          ((game->tableau[didx1][didx2] & 0xf) !=
           ((game->tableau[oidx1][oidx2] & 0xf) + 1)))) ||
        ((game->tableau[didx1][didx2] == 0xff) &&
         ((game->tableau[oidx1][oidx2] & 0xf) != 12))) {
        return 2;
    }
    if (game->tableau[didx1][didx2] & 0x80) {
        for (unsigned char i = didx2, ii = oidx2;
             i < 19 && ii < 19 &&
             !(game->tableau[oidx1][ii] & 0x80);
             ++i, ++ii) {
            game->tableau[didx1][i] = game->tableau[oidx1][ii];
            game->tableau[oidx1][ii] = 0xff;
        }

    } else {
        for (unsigned char i = didx2 + 1, ii = oidx2;
             i < 12 && ii < 12 &&
             !(game->tableau[oidx1][ii] & 0x80);
             ++i, ++ii) {
            game->tableau[didx1][i] = game->tableau[oidx1][ii];
            game->tableau[oidx1][ii] = 0xff;
        }
    }
    if (oidx2) {
        if (game->tableau[oidx1][oidx2 - 1] & 0x80) {
            game->tableau[oidx1][oidx2 - 1] &= 0x7f;
        }
    }
    return 0;
}

int core_foundation_to_tableau(struct Game *game,
                               unsigned char oidx,
                               unsigned char didx1,
                               unsigned char didx2) {
    if (!game ||
        oidx > 3 ||
        didx1 > 6 ||
        didx2 > 19) {
        return 1;
    }
    if (game->tableau[didx1][didx2] & 0x80 ||
        game->foundations[oidx] == 0xff) {
        return 2;
    }
    if (!((game->tableau[didx1][didx2] ^
           game->foundations[oidx]) & 0x20) ||
        ((game->tableau[didx1][didx2] & 0xf) !=
         ((game->foundations[oidx] & 0xf) + 1))) {
        return 2;
    }
    game->tableau[didx1][didx2 + 1] = game->foundations[oidx];
    if (!(game->foundations[oidx] & 0xf)) {
        game->foundations[oidx] = 0xff;
    } else {
        --game->foundations[oidx];
    }
    return 0;
}

int core_log(struct Game *game, FILE *fout) {
    if (!fout) {
        return 1;
    }
    puts("Stock:");
    for (unsigned char i = 0, *ii = game->stock; i < 24; ++i, ++ii) {
        if (*ii != 0xff) {
            printf("%c%2s\n",
                   suite_lookup[(unsigned long)(*ii >> 4)],
                   rank_lookup[(unsigned long)(*ii & 0xf)]);
        } else {
            puts("Empty");
        }
    }
    puts("Talon:");
    for (unsigned char i = 0, *ii = game->talon; i < 24; ++i, ++ii) {
        if (*ii != 0xff) {
            printf("%c%2s\n",
                   suite_lookup[(unsigned long)(*ii >> 4)],
                   rank_lookup[(unsigned long)(*ii & 0xf)]);
        } else {
            puts("Empty");
        }
    }
    puts("Tableau:");
    for (unsigned char i = 0; i < 7; ++i) {
        printf("%hhd:\n", i);
        for (unsigned char j = 0, *jj = game->tableau[(unsigned long)i];
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
