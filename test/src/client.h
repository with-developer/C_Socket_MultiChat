#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "packet.h"
#include "client_ui.h"

void askUsername(struct client_ui*, packet_t*);
int userInput(struct client_ui*, char*, int);

#endif