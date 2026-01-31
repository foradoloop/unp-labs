#include "ping.h"

u16 in_cksum(u16 *addr, int len)
{
	u32 sum = 0;
	int nleft = len;

	while (nleft > 1) {
		sum += *addr++;
		nleft = nleft - 2;
	}

	if (nleft == 1) {
		sum += *((uint8_t *)addr);
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += sum >> 16;

	return (u16)~sum;
}

void make_echo_request(char *buffer, size_t len, u16 id, u16 seq)
{
	struct icmp *icmp = (struct icmp *)buffer;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seq);

	memset(icmp->icmp_data, 0xa5, len - 8);
	Gettimeofday((struct timeval *)icmp->icmp_data, NULL);

	icmp->icmp_cksum = in_cksum((u16 *)buffer, len);
}

void tv_sub(struct timeval *a, struct timeval *b, struct timeval *c)
{
	c->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec < b->tv_usec) {
		c->tv_usec = 1000000 + a->tv_usec - b->tv_usec;
		c->tv_sec--;
	} else {
		c->tv_usec = a->tv_usec - b->tv_usec;
	}
}

void proc_echo_reply(char *buffer, size_t len, u16 id)
{
	if (len < sizeof(struct ip) + sizeof(struct icmp)) {
		return;
	}

	struct ip *ip = (struct ip *)buffer;
	u16 ip_tl = ntohs(ip->ip_len);
	u16 ip_hl = ip->ip_hl << 2;
	struct icmp *icmp = (struct icmp *)(buffer + ip_hl);
	u16 icmp_len = ip_tl - ip_hl;
	struct timeval *tvsend;
	struct timeval tvrecv, tvrtt;
	char ipaddr[INET_ADDRSTRLEN];
	double rtt;

	if (ip->ip_p != IPPROTO_ICMP) {
		return;
	}
	if (in_cksum((u16 *)icmp, icmp_len)) {
			return;
	}
	if (icmp->icmp_type != ICMP_ECHOREPLY) {
		return;
	}	
	if (icmp->icmp_id != htons(id)) {
		return;
	}
	if (icmp_len < 8 + sizeof(struct timeval)) {
		return;
	}

	tvsend = (struct timeval *)icmp->icmp_data;
	Gettimeofday(&tvrecv, NULL);
	tv_sub(&tvrecv, tvsend, &tvrtt);
	rtt = tvrtt.tv_sec * 1000.0 + tvrtt.tv_usec / 1000.0;

	Inet_ntop(AF_INET, &ip->ip_src, ipaddr, sizeof(ipaddr));

	printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.2lf ms\n",
			ip_tl - ip_hl, ipaddr, ntohs(icmp->icmp_seq), ip->ip_ttl, rtt);
}

