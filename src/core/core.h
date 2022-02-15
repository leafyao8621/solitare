#ifndef _CORE_H_
#define _CORE_H_

#include <stdio.h>

#include "../util/mt19937.h"

extern const char suite_lookup[4];
extern const char *rank_lookup[13];

struct Game {
    struct MT19937 gen;
    char stock[24], *stock_ptr,
         talon[24], *talon_ptr,
         foundations[4],
         tableau[7][13];
};

int core_initialize(struct Game *game, unsigned seed);
int core_draw(struct Game *game);
int core_log(struct Game *game, FILE *fout);

#endif
