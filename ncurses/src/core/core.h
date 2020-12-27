#ifndef _CORE_H_
#define _CORE_H_

#define BLACKJACK 0x80
#define BUST 0x40
#define SOFT 0x20

#include <stdio.h>
#include "../util/mt19937.h"

struct Game {
    char banker[21];
    char banker_hidden, reveal;
    char *banker_end;
    char player[21];
    char *player_end;
    char split[8];
    char *split_cur, *split_end;
    char shoe[416];
    char *shoe_cur, *shoe_end;
    char n_deck;
    unsigned len;
    char reshuffle;
    long money;
    long principal;
    long interest;
    long gain;
    long bet;
    struct MT19937 gen;
};

extern struct Game game;

void core_initialize(char n_deck, char reshuffle);
void core_log(FILE *fout);
char core_bet(long val);
char core_borrow(long val);
char core_banker_val(void);
char core_player_val(void);
char core_get_val(char idx);
char core_get_name(char idx);

#endif
