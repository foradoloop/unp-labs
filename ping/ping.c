#include "ping.h"

uint16_t in_cksum(uint16_t *addr, int len)
{
	uint32_t sum = 0;
	int nleft = len;
	uint16_t answer = 0;

	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(uint8_t *)(&answer) = *(uint8_t *)addr;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;
}

void make_echo_request(char *buffer, size_t len, uint16_t id, uint16_t seq)
{
	struct icmp *icmp = (struct icmp *)buffer;

	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seq);

	memset(icmp->icmp_data, 0xa5, len - 8);
	Gettimeofday((struct timeval *)icmp->icmp_data, NULL);

	icmp->icmp_cksum = in_cksum((uint16_t *)buffer, len);
}

void tv_sub(struct timeval *out, struct timeval *in, struct timeval *diff)
{
	diff->tv_sec = out->tv_sec - in->tv_sec;
	if (out->tv_usec < in->tv_usec) {
		diff->tv_usec = 1000000 + out->tv_usec - in->tv_usec;
		diff->tv_sec--;
	} else {
		diff->tv_usec = out->tv_usec - in->tv_usec;
	}
}

void proc_echo_reply(char *buffer, size_t len, uint16_t id)
{
	struct ip *ip = (struct ip *)buffer;
	struct icmp *icmp;
	struct timeval *tv_send;
	struct timeval tv_recv, tv_rtt;
	uint16_t ip_hl, ip_tl, icmp_len;
	char ip_addr_str[INET_ADDRSTRLEN];
	double rtt;

	if (len < sizeof(struct ip) + 8) {
		return;
	}

	ip_hl = ip->ip_hl << 2;
	ip_tl = ntohs(ip->ip_len);
	icmp = (struct icmp *)(buffer + ip_hl);
	icmp_len = ip_tl - ip_hl;


	if (ip->ip_p != IPPROTO_ICMP) {
		return;
	}

	if (in_cksum((uint16_t *)icmp, icmp_len)) {
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

	tv_send = (struct timeval *)icmp->icmp_data;
	Gettimeofday(&tv_recv, NULL);
	tv_sub(&tv_recv, tv_send, &tv_rtt);
	rtt = tv_rtt.tv_sec * 1000.0 + tv_rtt.tv_usec / 1000.0;

	Inet_ntop(AF_INET, &ip->ip_src, ip_addr_str, sizeof(ip_addr_str));

	printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.2lf ms\n",
	       ip_tl - ip_hl,
	       ip_addr_str,
	       ntohs(icmp->icmp_seq), 
	       ip->ip_ttl,
	       rtt);
}

