#ifndef PING_H
#define PING_H

#include "unp-labs.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>

#define PING_TOTAL_LEN 64
#define PING_HEADER_LEN 8
#define PING_PAYLOAD_LEN (PING_TOTAL_LEN - PING_HEADER_LEN)
#define BUFFERLEN (PING_TOTAL_LEN + 512)

void make_echo_request(char *buffer, size_t len, uint16_t id, uint16_t seq);
void proc_echo_reply(char *buffer, size_t len, uint16_t id);

#endif

