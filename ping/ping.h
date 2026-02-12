#ifndef PING_H
#define PING_H

#include "unp-labs.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>

typedef uint16_t u16;
typedef uint32_t u32;

#define PING_TL 64
#define PING_HDR_LEN 8
#define PING_PL_LEN (PING_TL - PING_HDR_LEN)
#define BUFFERLEN (PING_TL + 100)

void make_echo_request(char *buffer, size_t len, u16 id, u16 seq);
void proc_echo_reply(char *buffer, size_t len, u16 id);

#endif

