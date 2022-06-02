#include <stdlib.h>
#include <string.h>
#include "client_ui.h"

void initCurses(struct client_ui *ui) {
    if((ui->main_win = initscr()) == NULL) {
        perror("Error: Create Curses Window Failed: ");
        exit(EXIT_FAILURE);
    }
    // No echo key press, handle by ourselves
    noecho();
    // Read input one char at a time
    cbreak();
    // Capture Spectial Key
    keypad(ui->main_win, TRUE);
    initColors();

    if(LINES < LINE_MIN || COLS < COLUMN_MIN) {
        drawTermTooSmall(ui);
    }
    else {
        drawMainUI(ui);
    }
}

void initColors() {
    /* Use 8 colors, this is also the most common color scheme */
    start_color();
    // Start using the default terminal colors
    use_default_colors();

    // Initialize color pairs
    init_pair(1, -1, -1); // Default
    init_pair(2, COLOR_CYAN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_RED, -1);
    init_pair(5, COLOR_BLUE, -1);
    init_pair(6, COLOR_MAGENTA, -1);
    init_pair(7, COLOR_GREEN, -1);
    init_pair(8, COLOR_RED, COLOR_WHITE);
}

void drawTermTooSmall(struct client_ui *ui) {
    wbkgd(ui->main_win, COLOR_PAIR(8));
    wattron(ui->main_win, A_BOLD);
    mvwaddstr(ui->main_win, (LINES * 0.5) - 1, (COLS * 0.5) - 3, "TERMINAL");
    mvwaddstr(ui->main_win, (LINES * 0.5), (COLS * 0.5) - 4, "TOO SMALL!");
    wattroff(ui->main_win, A_BOLD);
    wrefresh(ui->main_win);
    wbkgd(ui->main_win, COLOR_PAIR(1));
}

void drawMainUI(struct client_ui *ui) {
    int chat_lines = (LINES >> 1) + (LINES >> 2);
    ui->chat_box = subwin(ui->main_win, chat_lines, COLS, 0, 0);
    box(ui->chat_box, 0, 0);

    // Draw Main Title on Chat Box
    int title_half_len = (strlen(MAIN_TITLE) >> 1) + (strlen(MAIN_TITLE) & 1);
    mvwaddch(ui->chat_box, 0, (COLS * 0.5) - title_half_len - 1, ACS_RTEE);
    wattron(ui->chat_box, COLOR_PAIR(3));
    mvwaddstr(ui->chat_box, 0, (COLS * 0.5) - title_half_len, MAIN_TITLE);
    wattroff(ui->chat_box, COLOR_PAIR(3));
    mvwaddch(ui->chat_box, 0, (COLS * 0.5) + title_half_len - 1, ACS_LTEE );

    ui->chat_win = derwin(ui->chat_box, chat_lines - 2, COLS - 2, 1, 1);
    // Enable Text Scrolling
    scrollok(ui->chat_win, TRUE);
    wrefresh(ui->chat_win);
    // Create Input Box under Chat Box
    ui->input_box = subwin(ui->main_win, (LINES - chat_lines), COLS, chat_lines, 0);
    box(ui->input_box, 0, 0);
    ui->input_win = derwin(ui->input_box, (LINES - chat_lines) - 2, COLS - 2, 1, 1);
    wrefresh(ui->input_win);
}

void wprinttime(WINDOW* win, time_t timestamp) {
    struct tm *tm_print = localtime(&timestamp);
    // Print HH:MM:SS
    wattron(win, COLOR_PAIR(1));
    wprintw(win, "%02d:%02d:%02d", tm_print->tm_hour, tm_print->tm_min, tm_print->tm_sec);
    wattroff(win, COLOR_PAIR(1));

    // Print vertical line
    wattron(win, COLOR_PAIR(3));
    wprintw(win, " ");
    waddch(win, ACS_VLINE);
    wprintw(win, " ");
    wattroff(win, COLOR_PAIR(3));
}

void wprinterr(WINDOW* win, time_t timestamp, char* buf) {
    wprinttime(win, timestamp);

    wattron(win, A_BOLD);
    wattron(win, COLOR_PAIR(8));
    wprintw(win, "[Error]");
    wattroff(win, COLOR_PAIR(8));
    wattroff(win, A_BOLD);
    // Print error message
    wattron(win, COLOR_PAIR(8));
    wprintw(win, " %s\n", buf);
    wattroff(win, COLOR_PAIR(8));
}

void wprintnotify(WINDOW* win, time_t timestamp, char* buf) {
    wprinttime(win, timestamp);

    wprintw(win, "[");
    wattron(win, COLOR_PAIR(6));
    wprintw(win, "SERVER");
    wattroff(win, COLOR_PAIR(6));
    wprintw(win, "]");
    // Print notify message
    wattron(win, COLOR_PAIR(1));
    wprintw(win, " %s\n", buf);
    wattroff(win, COLOR_PAIR(1));
}

void wprintmsg(WINDOW* win, time_t timestamp, char* username, char* buf) {
    wprinttime(win, timestamp);
    // Print message sender
    wprintw(win, "[");
    wattron(win, COLOR_PAIR(2));
    wprintw(win, username);
    wattroff(win, COLOR_PAIR(2));
    wprintw(win, "]");
    // Print normal message
    wattron(win, COLOR_PAIR(1));
    wprintw(win, " %s\n", buf);
    wattroff(win, COLOR_PAIR(1));
}