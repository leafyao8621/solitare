#ifndef _MT19937_H_
#define _MT19937_H_

struct MT19937 {
    int index;
    unsigned int MT[624];
};

void mt19937_initialize(struct MT19937 *gen, unsigned int seed);
unsigned int mt19937_gen(struct MT19937 *gen);
#endif
