#include <time.h>
#include "core.h"

#define PRINCIPAL_LIMIT 100000000

static char val[13] = {
    0,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    10,
    10,
    10
};

static char name[13] = {
    'A',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'T',
    'J',
    'Q',
    'K'
};

struct Game game;

static void core_shuffle(void) {
    for (char *i = game.shoe; i != game.shoe_end; ++i) {
        unsigned idx = mt19937_gen(&game.gen) % game.len;
        char temp = *i;
        *i = game.shoe[idx];
        game.shoe[idx] = temp;
    }
}

char core_banker_val(void) {
    if (!game.reveal) {
        return val[*game.banker];
    }
    char out = val[game.banker_hidden];
    if (!out) {
        out &= SOFT;
        if (val[*game.banker] == 10) {
            return BLACKJACK | 21;
        }
    }
    for (char *i = game.banker; i != game.banker_end; ++i) {
        if (!*i) {
            out &= SOFT;
        }
        out += val[*i];
        if (out & 0x10 > 21) {
            out &= BUST;
        }
    }
    return out;
}

char core_player_val(void) {
    char out = val[*game.player];
    if (!out) {
        out &= SOFT;
        if (val[game.player[1]] == 10) {
            return BLACKJACK | 21;
        }
    }
    for (char *i = game.player + 1; i != game.player_end; ++i) {
        if (!*i) {
            out &= SOFT;
        }
        out += val[*i];
        if (out & 0x10 > 21) {
            out &= BUST;
        }
    }
    return out;
}

void core_initialize(char n_deck, char reshuffle) {
    if (n_deck > 8 || n_deck < 1) {
        return;
    }
    game.n_deck = n_deck;
    game.len = n_deck * 52;
    game.reshuffle = reshuffle;
    game.shoe_end = game.shoe + game.len;
    game.shoe_cur = game.shoe;
    game.player_end = game.player;
    game.banker_end = game.banker;
    game.money = 100000;
    game.principal = 0;
    game.interest = 0;
    game.gain = 0;
    game.reveal = 0;
    char *iter = game.shoe;
    for (int i = 0; i < n_deck; ++i) {
        for (int j = 0; j < 13; ++j) {
            for (int k = 0; k < 4; ++k, ++iter) {
                *iter = j;
            }
        }
    }
    mt19937_initialize(&game.gen, time(0));
    core_shuffle();
    *(game.player_end++) = *(game.shoe_cur++);
    *(game.banker_end++) = *(game.shoe_cur++);
    *(game.player_end++) = *(game.shoe_cur++);
    game.banker_hidden = *(game.shoe_cur++);
}

void core_log(FILE *fout) {
    fputs("Banker: ", fout);
    char banker = core_banker_val();
    if (banker & BLACKJACK) {
        fputs("BLACKJACK\n", fout);
    } else if (banker & BUST) {
        fputs("BUST", fout);
    } else {
        if (banker & SOFT) {
            fputs("SOFT ", fout);
        }
        fprintf(fout, "%hhd\n", banker & 0x1f);
    }
    for (char *i = game.banker; i != game.banker_end; ++i) {
        fprintf(fout, "%c val: %d\n", name[*i], val[*i]);
    }
    fputs("Banker Hidden:", fout);
    if (game.reveal) {
        fprintf(fout, " %c val: %d",
                name[game.banker_hidden], val[game.banker_hidden]);
    }
    fputc(10, fout);
    fputs("Player: ", fout);
    char player = core_player_val();
    if (player & BLACKJACK) {
        fputs("BLACKJACK\n", fout);
    } else if (player & BUST) {
        fputs("BUST", fout);
    } else {
        if (player & SOFT) {
            fputs("SOFT ", fout);
        }
        fprintf(fout, "%hhd\n", player & 0x1f);
    }
    for (char *i = game.player; i != game.player_end; ++i) {
        fprintf(fout, "%c val: %d\n", name[*i], val[*i]);
    }
}

char core_bet(long val) {
    if (game.money < val) {
        return 1;
    }
    game.money -= val;
    game.bet += val;
    return 0;
}

char core_borrow(long val) {
    if (game.principal >= PRINCIPAL_LIMIT) {
        return 1;
    }
    game.money += val;
    game.principal += val;
    return 0;
}

char core_get_val(char idx) {
    return val[idx];
}

char core_get_name(char idx) {
    return name[idx];
}
