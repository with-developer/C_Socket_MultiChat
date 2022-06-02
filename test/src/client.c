#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include "packet.h"
#include "client.h"

#define DEFAULT_SERVER_PORT 9487

int local_sd;
struct client_ui ui;

void resizeHandler(int signum);
void terminateHandler(int signum);

void* recvHandle() {
    int local_msglen;
    packet_t rx_pkt;
    struct tm *rxtm;
    memset((char *)&rx_pkt, 0, sizeof(rx_pkt));
    // Main Receiving Loop
    while ((local_msglen = recv(local_sd, &rx_pkt, sizeof(rx_pkt), 0))) {
        if(rx_pkt.opt == SENDNOTIFY) {
            wprintnotify(ui.chat_win, rx_pkt.timestamp, rx_pkt.buf);
        }
        else {
            wprintmsg(ui.chat_win, rx_pkt.timestamp, rx_pkt.username, rx_pkt.buf);
        }
        wrefresh(ui.chat_win);
        wcursyncup(ui.input_win);
        wrefresh(ui.input_win);
        memset((char *)&rx_pkt, 0, sizeof(rx_pkt));
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct hostent *hp;
    struct sockaddr_in sin;
    packet_t tx_pkt;
    memset((char *)&tx_pkt, 0, sizeof(tx_pkt));

    if(argc != 2) {
        fprintf(stderr, "Error: Usage: %s Server-IP\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    hp = gethostbyname(argv[1]);
    if (!hp) {
        fprintf(stderr, "[Chatroom] unknown host: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // Set Socket Server Info
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(DEFAULT_SERVER_PORT);

    if ((local_sd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        perror("[Chatroom] Creating socket failed: ");
        exit(EXIT_FAILURE);
    }
    if (connect(local_sd, (struct sockaddr *) &sin, sizeof(sin))  < 0 ) {
        perror("[Chatroom] Connect failed: "); 
        exit(EXIT_FAILURE);
    }

    initCurses(&ui);
    wcursyncup(ui.input_win);

    // Handle Ctrl-C Event Manually
    signal(SIGINT, terminateHandler);
    // Handle Screen Resize Event
    signal(SIGWINCH, resizeHandler);

    // Sending Packet Contains Username And Check
    while(1) {
        memset(tx_pkt.username, 0, sizeof(tx_pkt.username));
        askUsername(&ui, &tx_pkt);
        // Setting Packet
        tx_pkt.timestamp = time(NULL);
        tx_pkt.pkt_type = SINGLE_PKT;
        tx_pkt.opt = SETNAME;
        if((send(local_sd, &tx_pkt, sizeof(tx_pkt), 0)) < 0) {
            endwin();
            perror("[Chatroom] Recv failed: ");
            exit(EXIT_FAILURE);
        }
        packet_t tmp_pkt;
        do {
            if(recv(local_sd, &tmp_pkt, sizeof(tmp_pkt), 0) < 0) {
                endwin();
                perror("[Chatroom] Recv failed: ");
                exit(EXIT_FAILURE);
            }
        } while(tmp_pkt.opt == SENDMSG);
        if(tmp_pkt.opt == NAMEERR) {
            wprinterr(ui.chat_win, tmp_pkt.timestamp, "Username Duplicate! Please Choose other names!");
            wrefresh(ui.chat_win);
            wcursyncup(ui.input_win);
            wrefresh(ui.input_win);
        }
        else {
            break;
        }
    } 

    // Create Thread to Recieve Messages from Server
    pthread_t recvs;
    if(pthread_create(&recvs, NULL, recvHandle, NULL) != 0) {
        endwin();
        perror("[Chatroom] Create Thread failed: ");
        exit(EXIT_FAILURE);
    }
    pthread_detach(recvs);

    int msg_len = 0;
    tx_pkt.pkt_type = SINGLE_PKT;
    tx_pkt.opt = SENDMSG;
    // Main Message Sending Loop
    while (1) {
        do {
            memset(tx_pkt.buf, 0, sizeof(tx_pkt.buf));
            msg_len = userInput(&ui, tx_pkt.buf, sizeof(tx_pkt.buf));
        } while(msg_len == 0);
        tx_pkt.timestamp = time(NULL);
        send(local_sd, &tx_pkt, sizeof(tx_pkt), 0 );
    }
    terminateHandler(SIGINT);
    return 0;
}

void terminateHandler(int signum) {
    endwin();
    close(local_sd);
    exit(EXIT_SUCCESS);
}

void resizeHandler(int sig) {
    // End current windows
    endwin();
    refresh();
    clear();

    if (LINES < LINE_MIN || COLS < COLUMN_MIN) {
        drawTermTooSmall(&ui);
    }
    else {
        // Redraw windows
        drawMainUI(&ui);

        // Refresh and move cursor to input window
        wrefresh(ui.main_win);
        wcursyncup(ui.input_win);
        wrefresh(ui.input_win);
    }
}