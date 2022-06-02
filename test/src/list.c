#include <string.h>
#include "server.h"

client_t *addClient(struct list_head *head) {
    client_t *entry = malloc(sizeof(client_t));
    memset((char *)entry, 0, sizeof(client_t));
    list_add_tail(&(entry->list), head);
    return entry;
}

client_t *removeClient(client_t *entry) {
    list_del(&(entry->list));
    free(entry);
}

client_t *searchClient(struct list_head *head, int fd) {
    client_t *entry;
    list_for_each_entry(entry, head, list) {
        if(entry->fd == fd) {
            return entry;
        }
    }
    return NULL;
}

int checkName(struct list_head *head, char *name) {
    struct list_head *listptr;
    client_t *entry;
    list_for_each(listptr, head) {
        entry = list_entry(listptr, client_t, list);
        if(strcmp(entry->username, name) == 0) {
            return 0;
        }
    }
    return 1;
}

void freeList(struct list_head *head) {
    struct list_head *listptr;
    client_t *entry;
    list_for_each(listptr, head) {
        entry = list_entry(listptr, client_t, list);
        free(entry);
        entry = NULL;
    }
}