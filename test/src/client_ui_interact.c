#include <string.h>
#include "client.h"

void askUsername(struct client_ui *ui, packet_t *tx_pkt) {
    int name_len = 0;
    mvwaddstr(ui->input_box, 0, 2, " Enter User Name (Max 64): ");
    touchwin(ui->input_box);
    do {
        name_len = userInput(ui, tx_pkt->username, NAME_MAX);
    } while(name_len == 0);
    box(ui->input_box, 0, 0);
    wrefresh(ui->input_box);
}

int userInput(struct client_ui *ui, char *buf, int buf_size) {
    int i = 0;
    int ch;
    wmove(ui->input_win, 0, 0);
    wrefresh(ui->input_win);
    while ((ch = getch()) != '\n') {
        // Backspace
        if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == KEY_LEFT || ch == 8 || ch == 127) {
            if (i > 0) {
                wprintw(ui->input_win, "\b \b");
                buf[--i] = '\0';
                wrefresh(ui->input_win);
            }
            else {
                wprintw(ui->input_win, "\b");
            }
        }
        else if (ch == KEY_RESIZE) {
            continue;
        }
        // Otherwise put in buffer
        else if (ch != ERR) {
            if (i < BUFFER_MAX - 1) {
                strcat(buf, (char *)&ch);
                i++;
                wprintw(ui->input_win, "%c", ch);
                wrefresh(ui->input_win);
            }
            // Unless buffer is full
            else {
                wprintw(ui->input_win, "\b%c", ch);
                buf[(i - 1)] = '\0';
                strcat(buf, (char *)&ch);
                wrefresh(ui->input_win);
            }
        }
    }
    buf[i] = '\0';
    wclear(ui->input_win);
    wrefresh(ui->input_win);
    return i;
}