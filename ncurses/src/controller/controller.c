#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "../core/core.h"

#define BET 0
#define ENTRY_BET 1
#define PLAY 2
#define ENTRY_PLAY 3
#define BANKER 4

static char mode;
static signed char cur_menu;
static signed char cur_entry;
static char entry_buf[8];

static void render_basic(void) {
    mvprintw(0, 0, "Money:     $%8ld.%02ld",
             game.money / 100, game.money % 100);
    mvprintw(1, 0, "Principle: $%8ld.%02ld",
             game.principle / 100, game.principle % 100);
    mvprintw(2, 0, "Interest:  $%8ld.%02ld",
             game.interest / 100, game.interest % 100);
    mvprintw(3, 0, "Gain:      $%8ld.%02ld",
             game.gain / 100, ((game.gain % 100) + 100) % 100);
    mvprintw(4, 0, "Bet:       $%8ld.%02ld",
             game.bet / 100, game.bet % 100);
    mvaddstr(5, 0, "Banker:");
    char banker = core_banker_val();
    if (banker & BLACKJACK) {
        mvprintw(5, 7, "%13s", "BLACKJACK");
    } else if ((banker & 0x3f) > 21) {
        mvprintw(5, 7, "%13s", "BUST");
    } else {
        if (banker & SOFT) {
            mvaddstr(5, 8, "SOFT");
        } else {
            mvaddstr(5, 8, "    ");
        }
        mvprintw(5, 12, "%8hhd", banker & 0x3f);
    }
    int j = 0;
    for (char *i = game.banker; i != game.banker_end; ++i, j += 2) {
        mvprintw(6, j, "%c", core_get_name(*i));
    }
    mvaddstr(7, 0, "Banker Hidden:");
    if (game.reveal) {
        mvprintw(7, 14, "%6c",
                 core_get_name(game.banker_hidden));
    }
    mvaddstr(8, 0, "Player:");
    char player = core_player_val();
    if (player & BLACKJACK) {
        mvprintw(8, 7, "%13s", "BLACKJACK");
    } else if ((player & 0x3f) > 21) {
        mvprintw(8, 7, "%13s", "BUST");
    } else {
        if (player & SOFT) {
            mvaddstr(8, 8, "SOFT");
        } else {
            mvaddstr(8, 8, "    ");
        }
        mvprintw(8, 12, "%8hhd", player & 0x3f);
    }
    j = 0;
    for (char *i = game.player; i != game.player_end; ++i, j += 2) {
        mvprintw(9, j, "%c", core_get_name(*i));
    }
}

static void render_bet(void) {
    mvaddstr(11, 0, "Bet ");
    mvaddstr(12, 0, "Borrow");
    mvaddstr(13, 0, "Pay");
    move(11, 0);
}

static void render_play(void) {
    mvaddstr(11, 0, "Hit");
    mvaddstr(12, 0, "Stand ");
    mvaddstr(13, 0, "Split");
    mvaddstr(14, 0, "Double");
    mvaddstr(15, 0, "Borrow");
    move(11, 0);
}

static void render_banker(void) {
    mvaddstr(11, 0, "Next");
    mvaddstr(12, 0, "      ");
    mvaddstr(13, 0, "     ");
    mvaddstr(14, 0, "      ");
    mvaddstr(15, 0, "      ");
    move(11, 0);
}

static void render_entry(void) {
    mvprintw(11 + cur_menu, 11, "$ %s", entry_buf);
    move(11 + cur_menu, 13 + cur_entry);
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
    cur_menu = 0;
    cur_entry = 0;
    memset(entry_buf, '0', 7);
    entry_buf[7] = 0;
    render_basic();
    render_bet();
    game.shoe[0] = 10;
    game.shoe[2] = 0;
    // game.shoe[4] = 0;
    // game.shoe[5] = 5;
    // game.shoe[6] = 8;
}

char controller_handle(void) {
    int in = getch();
    char ret = 0;
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
            mode = ENTRY_BET;
            render_entry();
            break;
        }
        break;
    case ENTRY_BET:
        switch (in) {
        case KEY_LEFT:
        case 'A':
        case 'a':
            cur_entry = (((cur_entry - 1) % 7) + 7) % 7;
            move(11 + cur_menu, 13 + cur_entry);
            break;
        case KEY_RIGHT:
        case 'D':
        case 'd':
            cur_entry = (cur_entry + 1) % 7;
            move(11 + cur_menu, 13 + cur_entry);
            break;
        case KEY_UP:
        case 'W':
        case 'w':
            if (entry_buf[cur_entry] == '9') {
                entry_buf[cur_entry] = '0';
            } else {
                ++entry_buf[cur_entry];
            }
            render_entry();
            break;
        case KEY_DOWN:
        case 'S':
        case 's':
            if (entry_buf[cur_entry] == '0') {
                entry_buf[cur_entry] = '9';
            } else {
                --entry_buf[cur_entry];
            }
            render_entry();
            break;
        case 'X':
        case 'x':
        case 'K':
        case 'k':
            mode = BET;
            remove_entry();
            break;
        case 'Z':
        case 'z':
        case 'J':
        case 'j':
            switch (cur_menu) {
            case 0:
                if (!core_bet(atoi(entry_buf) * 100)) {
                    memset(entry_buf, '0', 7);
                    cur_entry = 0;
                    if (!core_start_game()) {
                        mode = PLAY;
                        remove_entry();
                        render_basic();
                        render_play();
                    } else {
                        mode = BANKER;
                        render_basic();
                        remove_entry();
                        render_banker();
                    }
                }
                break;
            case 1:
                if (!core_borrow(atoi(entry_buf) * 100)) {
                    mode = BET;
                    memset(entry_buf, '0', 7);
                    cur_entry = 0;
                    render_basic();
                    remove_entry();
                }
                break;
            case 2:
                if (!core_pay(atoi(entry_buf) * 100)) {
                    mode = BET;
                    memset(entry_buf, '0', 7);
                    cur_entry = 0;
                    render_basic();
                    remove_entry();
                }
                break;
            }
            break;
        }
        break;
    case PLAY:
        switch (in) {
        case KEY_UP:
        case 'W':
        case 'w':
            cur_menu = (((cur_menu - 1) % 5) + 5) % 5;
            move(11 + cur_menu, 0);
            break;
        case KEY_DOWN:
        case 'S':
        case 's':
            cur_menu = (cur_menu + 1) % 5;
            move(11 + cur_menu, 0);
            break;
        case 'Z':
        case 'z':
        case 'J':
        case 'j':
            switch (cur_menu) {
            case 0:
                if (!core_hit()) {
                    render_basic();
                    move(11, 0);
                } else {
                    mode = BANKER;
                    render_basic();
                    render_banker();
                }
                break;
            case 1:
                mode = BANKER;
                render_basic();
                render_banker();
                break;
            case 2:
                break;
            case 3:
                if (!core_double()) {
                    core_hit();
                    mode = BANKER;
                    render_basic();
                    render_banker();
                }
                break;
            case 4:
                mode = ENTRY_PLAY;
                render_entry();
                break;
            }
            break;
        }
        break;
    case ENTRY_PLAY:
        switch (in) {
        case KEY_LEFT:
        case 'A':
        case 'a':
            cur_entry = (((cur_entry - 1) % 7) + 7) % 7;
            move(11 + cur_menu, 13 + cur_entry);
            break;
        case KEY_RIGHT:
        case 'D':
        case 'd':
            cur_entry = (cur_entry + 1) % 7;
            move(11 + cur_menu, 13 + cur_entry);
            break;
        case KEY_UP:
        case 'W':
        case 'w':
            if (entry_buf[cur_entry] == '9') {
                entry_buf[cur_entry] = '0';
            } else {
                ++entry_buf[cur_entry];
            }
            render_entry();
            break;
        case KEY_DOWN:
        case 'S':
        case 's':
            if (entry_buf[cur_entry] == '0') {
                entry_buf[cur_entry] = '9';
            } else {
                --entry_buf[cur_entry];
            }
            render_entry();
            break;
        case 'X':
        case 'x':
        case 'K':
        case 'k':
            mode = PLAY;
            remove_entry();
            break;
        case 'Z':
        case 'z':
        case 'J':
        case 'j':
            switch (cur_menu) {
            case 2:
                break;
            case 4:
                if (!core_borrow(atoi(entry_buf) * 100)) {
                    mode = PLAY;
                    memset(entry_buf, '0', 7);
                    cur_entry = 0;
                    render_basic();
                    remove_entry();
                }
                break;
            }
            break;
        }
        break;
    case BANKER:
        switch (in) {
        case 'Z':
        case 'z':
        case 'J':
        case 'j':
            ret = core_banker();
            render_basic();
            move(11, 0);
            switch (ret) {
            case PUSH:
                mvaddstr(10, 0, "PUSH");
                move(11, 0);
                getch();
                mvaddstr(10, 0, "    ");
                core_reset();
                core_accrue_interest();
                mode = BET;
                mvprintw(6, 0, "%40c", ' ');
                mvprintw(9, 0, "%40c", ' ');
                mvprintw(7, 14, "%6c", ' ');
                render_basic();
                render_bet();
                cur_menu = 0;
                cur_entry = 0;
                break;
            case WIN:
                mvaddstr(10, 0, "WIN");
                move(11, 0);
                getch();
                mvaddstr(10, 0, "   ");
                core_reset();
                core_accrue_interest();
                mode = BET;
                mvprintw(6, 0, "%40c", ' ');
                mvprintw(9, 0, "%40c", ' ');
                mvprintw(7, 14, "%6c", ' ');
                render_basic();
                render_bet();
                cur_menu = 0;
                cur_entry = 0;
                break;
            case LOSS:
                mvaddstr(10, 0, "LOSS");
                move(11, 0);
                getch();
                mvaddstr(10, 0, "    ");
                core_reset();
                core_accrue_interest();
                mode = BET;
                mvprintw(6, 0, "%40c", ' ');
                mvprintw(9, 0, "%40c", ' ');
                mvprintw(7, 14, "%6c", ' ');
                render_basic();
                render_bet();
                cur_menu = 0;
                cur_entry = 0;
                break;
            }
            break;
        }
        break;
    }
    return 0;
}

void controller_finalize(void) {
    endwin();
}
