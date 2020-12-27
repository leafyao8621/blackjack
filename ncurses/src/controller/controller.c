#include <stdlib.h>
#include <ncurses.h>
#include "../core/core.h"

#define BET 0
#define ENTRY 1

static char mode;
static signed char cur_menu;
static signed char cur_entry;
static char entry_buf[8];

static void render_basic(void) {
    mvprintw(0, 0, "Money:     $%8ld.%02ld",
             game.money / 100, game.money % 100);
    mvprintw(1, 0, "Principal: $%8ld.%02ld",
             game.principal / 100, game.principal % 100);
    mvprintw(2, 0, "Interest:  $%8ld.%02ld",
             game.interest / 100, game.interest % 100);
    mvprintw(3, 0, "Gain:      $%8ld.%02ld",
             game.gain / 100, game.gain % 100);
    mvprintw(4, 0, "Bet:       $%8ld.%02ld",
             game.bet / 100, game.bet % 100);
    mvaddstr(5, 0, "Banker:");
    char banker = core_banker_val();
    if (banker & BLACKJACK) {
        mvaddstr(5, 8, "BLACKJACK");
    } else if (banker & BUST) {
        mvaddstr(5, 8, "BUST");
    } else {
        if (banker & SOFT) {
            mvaddstr(5, 8, "SOFT ");
        }
        mvprintw(5, 14, "%hhd", banker & 0x1f);
    }
    int j = 0;
    for (char *i = game.banker; i != game.banker_end; ++i, j += 2) {
        mvprintw(6, j, "%c", core_get_name(*i));
    }
    mvaddstr(7, 0, "Banker Hidden:");
    if (game.reveal) {
        mvprintw(7, 15, " %c val: %d",
                 core_get_name(game.banker_hidden));
    }
    mvaddstr(8, 0, "Player:");
    char player = core_player_val();
    if (player & BLACKJACK) {
        mvaddstr(8, 8, "BLACKJACK");
    } else if (player & BUST) {
        mvaddstr(8, 8, "BUST");
    } else {
        if (player & SOFT) {
            mvaddstr(8, 8, "SOFT ");
        }
        mvprintw(8, 14, "%hhd", player & 0x1f);
    }
    j = 0;
    for (char *i = game.player; i != game.player_end; ++i, j += 2) {
        mvprintw(9, j, "%c", core_get_name(*i));
    }
    cur_menu = 0;
    cur_entry = 0;
}

static void render_bet(void) {
    mvaddstr(11, 0, "Bet");
    mvaddstr(12, 0, "Borrow");
    mvaddstr(13, 0, "Pay");
    move(11, 0);
}

static void render_entry(void) {
    mvprintw(11 + cur_menu, 11, "$%08d", 0, 0);
    move(11 + cur_menu, 12 + cur_entry);
}

static void remove_entry(void) {
    mvprintw(11 + cur_menu, 11, "%9c", ' ');
    move(11 + cur_menu, 0);
}
void controller_initialize(void) {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    core_initialize(1, 0);
    render_basic();
    render_bet();
}

char controller_handle(void) {
    int in = getch();
    switch (in) {
    case 'Q':
    case 'q':
        return 1;
    }
    switch (mode) {
    case BET:
        switch (in) {
        case KEY_UP:
        case 'W':
        case 'w':
            cur_menu = (((cur_menu - 1) % 3) + 3) % 3;
            move(11 + cur_menu, 0);
            break;
        case KEY_DOWN:
        case 'S':
        case 's':
            cur_menu = (cur_menu + 1) % 3;
            move(11 + cur_menu, 0);
            break;
        case 'Z':
        case 'z':
        case 'J':
        case 'j':
            mode = ENTRY;
            render_entry();
            break;
        }
        break;
    case ENTRY:
        switch (in) {
        case KEY_LEFT:
        case 'A':
        case 'a':
            cur_entry = (cur_entry - 1) & 7;
            move(11 + cur_menu, 12 + cur_entry);
            break;
        case KEY_RIGHT:
        case 'D':
        case 'd':
            cur_entry = (cur_entry + 1) & 7;
            move(11 + cur_menu, 12 + cur_entry);
            break;
        case 'X':
        case 'x':
            mode = BET;
            remove_entry();
            break;
        }
    }
    return 0;
}

void controller_finalize(void) {
    endwin();
}
