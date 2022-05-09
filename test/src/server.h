#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdlib.h>
#include "packet.h"
#include "list.h"

#define UNSETNAME   0
#define NORMAL_CHAT 1

struct Client {
    int fd;
    char state;
    char username[NAME_MAX];
    struct list_head list;
};
typedef struct Client client_t;

client_t* addClient(struct list_head*);
client_t* removeClient(client_t*);
client_t *searchClient(struct list_head*, int);
int checkName(struct list_head*, char*);
void freeList(struct list_head*);

#endif