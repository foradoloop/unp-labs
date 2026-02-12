#include "ping.h"

int main(int argc, char **argv)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct timeval recv_timeout = {1, 0};
	struct timeval sleep_time;
	char ip_addr[INET_ADDRSTRLEN];
	char buffer[BUFFERLEN];
	int sockfd;
	pid_t pid;
	uint16_t nsent = 0;
	ssize_t nrecv;

	if (argc != 2) {
		err_quit("usage: ./ping <ipaddress>");
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;

	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
		err_sys("getaddrinfo");
	}

	sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	Setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, (socklen_t)sizeof(recv_timeout));

	Inet_ntop(AF_INET, &(((struct sockaddr_in *)res->ai_addr)->sin_addr), ip_addr, sizeof(ip_addr));

	printf("PING %s (%s): %d data bytes\n",
	       res->ai_canonname ? res->ai_canonname : ip_addr,
	       ip_addr, PING_PL_LEN);

	pid = getpid() & 0xFFFF;

	for (;;) {
		make_echo_request(buffer, PING_TL, pid, nsent++);
		
		Sendto(sockfd, buffer, PING_TL, 0, res->ai_addr, res->ai_addrlen);

again:
		nrecv = recvfrom(sockfd, buffer, BUFFERLEN, 0, NULL, NULL);
		
		if (nrecv < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				printf("Request timeout for icmp_seq %u\n", nsent - 1);
				continue;
			} else if (errno == EINTR) {
				goto again;
			} else {
				err_sys("recvfrom");
			}
		} else {
			proc_echo_reply(buffer, nrecv, pid);
		}


		sleep_time.tv_sec = 1;
		sleep_time.tv_usec = 0;
		select(0, NULL, NULL, NULL, &sleep_time);
	}


	freeaddrinfo(res);
	return 0;
}
