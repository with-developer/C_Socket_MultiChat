#ifndef _CLIENT_UI_H_
#define _CLIENT_UI_H_

#include <ncurses.h>
#include <time.h>

#define LINE_MIN    20
#define COLUMN_MIN  70

#define MAIN_TITLE  " Mumi Chat "

// Box is "container" to contain Win, interact with Win actually
struct client_ui
{
    WINDOW *main_win;
    WINDOW *chat_box;
    WINDOW *chat_win;
    WINDOW *input_box;
    WINDOW *input_win;
};

void initCurses(struct client_ui*);
void initColors();
void drawTermTooSmall(struct client_ui*);
void drawMainUI(struct client_ui*);
void wprinttime(WINDOW* win, time_t timestamp);
void wprinterr(WINDOW* win, time_t timestamp, char* buf);
void wprintnotify(WINDOW* win, time_t timestamp,  char* buf);
void wprintmsg(WINDOW* win, time_t timestamp, char* username, char* buf);

#endif