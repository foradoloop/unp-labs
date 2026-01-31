#include "ping.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./ping <ipaddr>\n");
		exit(1);
	}

	struct addrinfo hints;
	struct addrinfo *res;
	int sockfd;
	struct timeval recvto = { 1, 0 };
	char ipaddr[INET_ADDRSTRLEN];
	u16 nsent = 0;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;

	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
		err_sys("getaddrinfo");
	}

	sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvto, (socklen_t)sizeof(recvto));

	Inet_ntop(AF_INET, &(((struct sockaddr_in *)res->ai_addr)->sin_addr), ipaddr, sizeof(ipaddr));

	printf("PING %s (%s): %d data bytes\n",
			res->ai_canonname ? res->ai_canonname : ipaddr,
			ipaddr,
			PING_PL_LEN);

	for (;;) {
		char buffer[BUFFERLEN];
		pid_t pid = getpid() & 0xFFFF;
		ssize_t nrecv;
		struct timeval sleep = { 1 , 0 };

		make_echo_request(buffer, PING_TL, pid, nsent++);
		Sendto(sockfd, buffer, PING_TL, 0, res->ai_addr, res->ai_addrlen);
receive_again:
		nrecv = recvfrom(sockfd, buffer, BUFFERLEN, 0, NULL, NULL);
		if (nrecv < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				printf("Request timeout for %u\n", nsent - 1);
				continue;
			} else if (errno == EINTR) {
				goto receive_again;
			} else {
				err_sys("recvfrom");
			}
		} else {
			proc_echo_reply(buffer, nrecv, pid);
		}

		Select(0, NULL, NULL, NULL, &sleep);
	}

	freeaddrinfo(res);

	return 0;
}

