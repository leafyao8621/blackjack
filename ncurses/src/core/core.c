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
    game.shoe_cur = game.shoe;
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
    for (; ace_cnt > 0 && (out & 0x3f) > 21; --ace_cnt, out -= 10);
    if (ace_cnt) {
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
    for (; ace_cnt > 0 && (out & 0x3f) > 21; --ace_cnt, out -= 10);
    if (ace_cnt) {
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
    game.split_cur = game.split;
    game.split_end = game.split;
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

char core_start_game(void) {
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
    char player = core_player_val();
    if (player & BLACKJACK) {
        return 1;
    }
    return 0;
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
        val -= game.interest;
        game.interest = 0;
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

char core_hit(void) {
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    *(game.player_end++) = *(game.shoe_cur++);
    char player = core_player_val();
    if ((player & 0x3f) >= 21) {
        if (game.split_cur == game.split_end) {
            *(game.split_cur++) = player;
            ++game.split_end;
            return 1;
        } else {
            *(game.split_cur++) = player;
            game.player_end = game.player + 1;
            if (game.shoe_cur == game.shoe_end) {
                core_shuffle();
            }
            *(game.player_end++) = *(game.shoe_cur++);
            return 2;
        }
    }
    return 0;
}

char core_stand(void) {
    char player = core_player_val();
    if (game.split_cur != game.split_end) {
        *(game.split_cur++) = player;
        game.player_end = game.player + 1;
        if (game.shoe_cur == game.shoe_end) {
            core_shuffle();
        }
        *(game.player_end++) = *(game.shoe_cur++);
        return 1;
    } else {
        *(game.split_cur++) = player;
        ++game.split_end;
        return 0;
    }
}

char core_banker(void) {
    char init = 0;
    char player = game.split_cur[-1];
    if ((player & 0x3f) > 21) {
        game.gain -= game.bet;
        if (game.split_cur > game.split + 1) {
            --game.split_cur;
            return CONT | LOSS;
        } else {
            return LOSS;
        }
    }
    if (!game.reveal) {
        init = 1;
        game.reveal = 1;
    }
    char banker = core_banker_val();
    if (player & BLACKJACK) {
        if (banker & BLACKJACK) {
            game.money += game.bet;
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | PUSH;
            } else {
                return PUSH;
            }
        } else {
            game.money += (game.bet << 1) + (game.bet >> 1);
            game.gain += game.bet + (game.bet >> 1);
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | WIN;
            } else {
                return WIN;
            }
        }
    }
    banker = core_banker_val();
    if ((banker & 0x3f) > 17) {
        if ((banker & 0x3f) > 21) {
            game.money += game.bet << 1;
            game.gain += game.bet;
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | WIN;
            } else {
                return WIN;
            }
        }
        if ((player & 0x3f) > (banker & 0x3f)) {
            game.money += game.bet << 1;
            game.gain += game.bet;
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | WIN;
            } else {
                return WIN;
            }
        } else if ((player & 0x3f) == (banker & 0x3f)) {
            game.money += game.bet;
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | PUSH;
            } else {
                return PUSH;
            }
        } else {
            game.gain -= game.bet;
            if (game.split_cur > game.split + 1) {
                --game.split_cur;
                return CONT | LOSS;
            } else {
                return LOSS;
            }
        }
    }
    if (!init) {
        if (game.shoe_cur == game.shoe_end) {
            core_shuffle();
        }
        *(game.banker_end++) = *(game.shoe_cur++);
    }
    return CONT;
}

void core_reset(void) {
    game.bet = 0;
    game.reveal = 0;
    game.player_end = game.player;
    game.banker_end = game.banker;
    game.split_cur = game.split;
    game.split_end = game.split;
}

char core_double(void) {
    if (game.money < game.bet) {
        return 1;
    }
    game.money -= game.bet;
    game.bet += game.bet;
    return 0;
}

char core_split(void) {
    if (!(game.player_end - game.player == 2 &&
          *(game.player) == game.player[1])) {
        return 1;
    }
    if (game.money < game.bet) {
        return 2;
    }
    game.money -= game.bet;
    if (game.shoe_cur == game.shoe_end) {
        core_shuffle();
    }
    game.player_end[-1] = *(game.shoe_cur++);
    game.split_end++;
    return 0;
}
