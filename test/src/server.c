#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "server.h"

#define DEFAULT_SERVER_PORT 9487
#define MAX_CONNECT 20
#define MAX_PENDING 10

#define MESSAGE_STORE_FILE "chatmsg.txt"

int main(int argc, char* argv[]) {
    
    // Set Server Info
    struct sockaddr_in sa_local;
    memset(&sa_local, 0, sizeof(sa_local));
    sa_local.sin_family = AF_INET;
    sa_local.sin_addr.s_addr = INADDR_ANY;
    sa_local.sin_port = htons(DEFAULT_SERVER_PORT);

    int local_sd;
    int enable = 1, i = 0, j = 0;
    if((local_sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[Chatroom] Creating socket failed: ");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(local_sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("[Chatroom] Setsockopt failed: ");
        exit(EXIT_FAILURE);
    }
    if(bind(local_sd, (struct sockaddr*)&sa_local, sizeof(sa_local)) < 0) {
        perror("[Chatroom] Binding socket failed: ");
        exit(EXIT_FAILURE);
    }
    if(listen(local_sd, MAX_PENDING) < 0) {
        perror("[Chatroom] Listening failed: ");
        exit(EXIT_FAILURE);
    }

    int client_sd;
    uint32_t addrlen, recvbytes;
    struct sockaddr_in sa_client;

    // Prepare data to use select() later
    int fd_max;
    struct timeval timeout;
    fd_set master_fds, read_fds;
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);

    FD_SET(local_sd, &master_fds);
    fd_max = local_sd;

    // Prepare log file
    FILE *msg_file;
    if(!(msg_file = fopen(MESSAGE_STORE_FILE, "w"))) {
        perror("[Chatroom] Cannot open file: ");
        exit(EXIT_FAILURE);
    }
    LIST_HEAD(client_list);
    packet_t uni_pkt;
    memset((char *)&uni_pkt, 0, sizeof(uni_pkt));
    struct tm *rxtm;
    printf("[Chatroom] Start Mumi Chatroom Server on Port %d\n", DEFAULT_SERVER_PORT);
    while(1) {
        read_fds = master_fds;

        // Setting select() timeout to 1s
        // Since select() will modify timeout to the timeout left since last call,
        // we need to reassign the value before every call of select()
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        // Using select to determine which fd has event
        // Then we can do server in single-thread
        if(select(fd_max+1, &read_fds, NULL, NULL, &timeout) < 0) {
            perror("[Chatroom] Select Failed: ");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i <= fd_max; i++) {
            // Check which fd has read event
            if(FD_ISSET(i, &read_fds)) {
                // If local socket has read event => New client connected
                if(i == local_sd) {
                    addrlen = sizeof(sa_client);
                    if((client_sd = accept(local_sd, (struct sockaddr *)&sa_client, &addrlen)) < 0) {
                        perror("[Chatroom] Accept Failed: ");
                    }
                    else {
                        FD_SET(client_sd, &master_fds);
                        client_t *client = addClient(&client_list);
                        client->fd = client_sd;
                        client->state = UNSETNAME;
                        if(client_sd > fd_max) {
                            fd_max = client_sd;
                        }
                        printf("[Chatroom] New Client: %d\n", client_sd);
                    }
                }
                // Otherwise, receicing connected socket's packet
                else {
                    if((recvbytes = recv(i, &uni_pkt, sizeof(uni_pkt), 0)) <= 0) {
                        client_t *client = searchClient(&client_list, i);
                        if(recvbytes == 0) {
                            // Broadcast leave message
                            sprintf(uni_pkt.buf, "<%s> has left the chatroom", client->username);
                            // Setting packet and timestamp
                            uni_pkt.timestamp = time(NULL);
                            uni_pkt.opt = SENDNOTIFY;
                            removeClient(client);
                            list_for_each_entry(client, &client_list, list) {
                                if(client->state == NORMAL_CHAT) {
                                    if((send(client->fd, &uni_pkt, sizeof(uni_pkt), 0)) < 0) {
                                        perror("[Chatroom] send Failed: ");
                                    }
                                }
                            }
                            printf("[Chatroom] Client Disconnected: %d\n", i);
                        }
                        else {
                            removeClient(client);
                            perror("[Chatroom] recv Failed: ");
                        }
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                    else {
                        // Determine which action to do
                        switch(uni_pkt.opt) {
                            case SETNAME: {
                                uni_pkt.timestamp = time(NULL);
                                // Check if the name is used
                                if(!(checkName(&client_list, uni_pkt.username))) {
                                    // If is used, reject the request
                                    // Maintain this client in UNSETNAME state
                                    printf("[Chatroom] Client %d name duplicate... Reject\n", i);
                                    // Return failed message to the client
                                    uni_pkt.opt = NAMEERR;
                                    if((send(i, &uni_pkt, sizeof(uni_pkt), 0)) < 0) {
                                        perror("[Chatroom] send Failed: ");
                                    }
                                }
                                else {
                                    client_t *client = searchClient(&client_list, i);
                                    strcpy(client->username, uni_pkt.username);
                                    printf("[Chatroom] Client %d set name: %s\n", i, client->username);
                                    // Return success message to the client
                                    uni_pkt.opt = NAMESUC;
                                    if((send(i, &uni_pkt, sizeof(uni_pkt), 0)) < 0) {
                                        perror("[Chatroom] send Failed: ");
                                    }
                                    // Change client's state to normal
                                    client->state = NORMAL_CHAT;
                                    // Broadcast welcome message
                                    sprintf(uni_pkt.buf, "<%s> has come into the chatroom", client->username);
                                    uni_pkt.timestamp = time(NULL);
                                    uni_pkt.opt = SENDNOTIFY;
                                    list_for_each_entry(client, &client_list, list) {
                                        if((send(client->fd, &uni_pkt, sizeof(uni_pkt), 0)) < 0) {
                                            perror("[Chatroom] send Failed: ");
                                        }
                                    }
                                }
                                break;
                            }
                            case SENDMSG: {
                                printf("[New Mesg] <%s> %s\n", uni_pkt.username, uni_pkt.buf);
                                uni_pkt.timestamp = time(NULL);
                                rxtm = localtime(&(uni_pkt.timestamp));
                                // Save to log file
                                fprintf(msg_file, "%02d:%02d:%02d | [%s] %s\n", 
                                    rxtm->tm_hour, rxtm->tm_min, rxtm->tm_sec, uni_pkt.username, uni_pkt.buf);
                                fflush(msg_file);
                                // Send to each client
                                client_t* client;
                                list_for_each_entry(client, &client_list, list) {
                                    // Only client that have name set can got the message
                                    if(client->state == NORMAL_CHAT) {
                                        if((send(client->fd, &uni_pkt, sizeof(uni_pkt), 0)) < 0) {
                                            perror("[Chatroom] send Failed: ");
                                        }
                                    }
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
    fclose(msg_file);
    return 0;
}
