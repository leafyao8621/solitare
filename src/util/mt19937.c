#include <stdio.h>
#include "mt19937.h"

#define w 32
#define n 624
#define m 397
#define r 31
#define a 0x9908b0df
#define u 11
#define d 0xffffffff
#define s 7
#define b 0x9d2c5680
#define t 15
#define c 0xefc60000
#define l 18
#define f 1812433253

#define upper_mask 0x80000000
#define lower_mask 0x7fffffff

void mt19937_initialize(struct MT19937 *gen, unsigned int seed) {
    gen->index = n;
    *(gen->MT) = seed;
    for (unsigned int i = 1, *ii = gen->MT + 1;
         i < n;
         *ii = (f * (ii[-1] ^ (ii[-1] >> 30)) + i),
        //  printf("0x%08x 0x%08x\n", ii[-1], *ii),
         ++ii, ++i);
}

static void twist(struct MT19937 *gen) {
    unsigned int x, xA;
    for (unsigned int i = 0, *ii = gen->MT; i < n; ++i, ++ii) {
        x = i < n - 1 ?
            ((*ii & upper_mask) | (ii[1] & lower_mask)) :
            ((*ii & upper_mask) | (*(gen->MT) & lower_mask));
        // printf("0x%08x\n", x);
        xA = x >> 1;
        if (x & 1) {
            xA ^= a;
        }
        *ii = gen->MT[(i + m) % n] ^ xA;
    }
    gen->index = 0;
}

unsigned int mt19937_gen(struct MT19937 *gen) {
    if (gen->index == n) {
        twist(gen);
    }

    unsigned int y = gen->MT[gen->index++];
    // printf("%u\n", y);
    y ^= (y >> u);
    y ^= (y << s) & b;
    y ^= (y << t) & c;
    y ^= y >> l;
    return y;
}
