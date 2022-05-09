#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdint.h>
#include <sys/types.h>

/* Buffer Max Length Definition */
#define BUFFER_MAX 256
#define NAME_MAX 64

/* Packet Types */
#define SINGLE_PKT 0
#define CONTINUOUS_PKT 1

/* Packet Options */
#define SETNAME     1
#define SENDMSG     2
#define SENDNOTIFY  3

/* Server Response */
#define SUCCESS 100
#define NAMESUC 101
#define NAMEERR 102

struct Packet
{
    time_t timestamp;
    char buf[BUFFER_MAX];
    char username[NAME_MAX];
    uint8_t pkt_type;
    uint8_t opt;
};
typedef struct Packet packet_t;


#endif