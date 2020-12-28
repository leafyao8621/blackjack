#include <time.h>
#include "core.h"

#define PRINCIPLE_LIMIT 100000000

static char val[13] = {
    11,
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
    if (game.banker_end == game.banker) {
        return 0;
    }
    if (!game.reveal) {
        return val[*game.banker];
    }
    char out = val[game.banker_hidden];
    if (val[*game.banker] + out == 21) {
        return BLACKJACK | 21;
    }
    char ace_cnt = 0;
    for (char *i = game.banker; i != game.banker_end; ++i) {
        if (!*i) {
            ++ace_cnt;
        }
        out += val[*i];
    }
    for (; ace_cnt > 0 && (out & 0x1f) > 21; --ace_cnt, out -= 10);
    if ((out & 0x1f) > 21) {
        out |= BUST;
    } else if (ace_cnt) {
        out |= SOFT;
    }
    return out;
}

char core_player_val(void) {
    if (game.player_end == game.player) {
        return 0;
    }
    char out = val[*game.player];
    if (out + val[game.player[1]] == 21) {
        return BLACKJACK | 21;
    }
    char ace_cnt = !*game.player;
    for (char *i = game.player + 1; i != game.player_end; ++i) {
        if (!*i) {
            ++ace_cnt;
        }
        out += val[*i];
    }
    for (; ace_cnt > 0 && (out & 0x1f) > 21; --ace_cnt, out -= 10);
    if ((out & 0x1f) > 21) {
        out |= BUST;
    } else if (ace_cnt) {
        out |= SOFT;
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
    game.principle = 0;
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
}

void core_start_game(void) {
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    *(game.player_end++) = *(game.shoe_cur++);
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    *(game.banker_end++) = *(game.shoe_cur++);
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    *(game.player_end++) = *(game.shoe_cur++);
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    game.banker_hidden = *(game.shoe_cur++);
}

char core_bet(long val) {
    if (!val || game.money < val) {
        return 1;
    }
    game.money -= val;
    game.bet += val;
    return 0;
}

char core_borrow(long val) {
    if (game.principle + val > PRINCIPLE_LIMIT) {
        return 1;
    }
    game.money += val;
    game.principle += val;
    return 0;
}

char core_pay(long val) {
    if (game.money < val) {
        return 1;
    }
    game.money -= val;
    if (val > game.interest) {
        game.interest = 0;
        val -= game.interest;
        game.principle -= val;
    } else {
        game.interest -= val;
    }
    return 0;
}

void core_accrue_interest(void) {
    game.interest += game.principle * 0.01;
    game.gain -= game.principle * 0.01;
}

char core_get_val(char idx) {
    return val[idx];
}

char core_get_name(char idx) {
    return name[idx];
}
