#include "unp-labs.h"
#include <net/ethernet.h>     
#include <netinet/if_ether.h> 
#include <net/if.h>           
#include <netpacket/packet.h>

#define BUFLEN 2048

int main(int argc, char **argv)
{
	int opt;
	char sendbuf[BUFLEN];
	int sockfd;
	
	struct ether_header *eth;
	struct ether_arp *arp;
	
	struct ifreq ifr;
	unsigned long suip; 
	unsigned char smac[ETHER_ADDR_LEN];
	struct sockaddr_ll sll;
	size_t len;
	char sipaddr[INET_ADDRSTRLEN];
	struct timeval to;

	if (argc != 5) {
		err_quit("usage: ./arp -i <interface> -a <address>");
	}

	memset(&ifr, 0, sizeof(ifr));

	while ((opt = getopt(argc, argv, "i:a:")) != -1) {
		switch (opt) {
			case 'i':
				strncpy(ifr.ifr_name, optarg, IFNAMSIZ);
				break;
			case 'a':
				strncpy(sipaddr, optarg, INET_ADDRSTRLEN);
				break;
			default:
				err_quit("usage: ./arp -i <interface> -a <address>");
		}
	}

	sockfd = Socket(PF_PACKET, SOCK_RAW, htons(ETHERTYPE_ARP));
	
	eth = (struct ether_header *)sendbuf;
	arp = (struct ether_arp *)(sendbuf + sizeof(struct ether_header));

	// Get interface IP
	Ioctl(sockfd, SIOCGIFADDR, &ifr);
	memcpy(&suip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, 4);

	// Get interface MAC address
	Ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	memcpy(smac, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

	// Get interface index
	Ioctl(sockfd, SIOCGIFINDEX, &ifr);
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;

	memset(sendbuf, 0, BUFLEN);

	memset(eth->ether_dhost, 0xFF, ETHER_ADDR_LEN);
	memcpy(eth->ether_shost, smac, ETHER_ADDR_LEN);
	eth->ether_type = htons(ETHERTYPE_ARP);

	arp->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
	arp->ea_hdr.ar_pro = htons(ETHERTYPE_IP);
	arp->ea_hdr.ar_hln = 0x06;
	arp->ea_hdr.ar_pln = 0x04;
	arp->ea_hdr.ar_op  = htons(ARPOP_REQUEST);
	
	memcpy(arp->arp_sha, eth->ether_shost, ETHER_ADDR_LEN);
	memcpy(arp->arp_spa, &suip, 4);
	memset(arp->arp_tha, 0, ETHER_ADDR_LEN);
	Inet_pton(AF_INET, sipaddr, arp->arp_tpa);

	to.tv_sec = 5;
	to.tv_usec = 0;
	Setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

	len = sizeof(struct ether_header) + sizeof(struct ether_arp);
	Sendto(sockfd, sendbuf, len, 0, (struct sockaddr *)&sll, sizeof(sll));

	for (;;) {
		char recvbuf[BUFLEN];
		ssize_t nrecv;
		
		struct ether_header *rec_eth;
		struct ether_arp *rec_arp;

again:
		nrecv = recvfrom(sockfd, recvbuf, BUFLEN, 0, NULL, NULL);
		if (nrecv <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				fprintf(stderr, "arp request timeout\n");
				break;
			} else if (errno == EINTR) {
				goto again;
			} else {
				err_sys("recvfrom");
			}
		}

		rec_eth = (struct ether_header *)recvbuf;
		rec_arp = (struct ether_arp *)(recvbuf + sizeof(struct ether_header));

		if (memcmp(smac, rec_eth->ether_dhost, ETHER_ADDR_LEN) != 0) {
			continue;
		} else if (rec_eth->ether_type != htons(ETHERTYPE_ARP)) {
			continue;
		} else if (rec_arp->ea_hdr.ar_hrd != htons(ARPHRD_ETHER)) {
			continue;
		} else if (rec_arp->ea_hdr.ar_op != htons(ARPOP_REPLY)) {
			continue;
		} else if (memcmp(rec_arp->arp_tpa, &suip, 4) != 0) {
			continue;
		} else {
			unsigned char *dmac;
			dmac = rec_arp->arp_sha;
			printf("%s\t%02x:%02x:%02x:%02x:%02x:%02x\n", sipaddr, dmac[0], dmac[1], dmac[2], dmac[3], dmac[4], dmac[5]);
			break;
		}
	}

	close(sockfd);

	return 0;
}
