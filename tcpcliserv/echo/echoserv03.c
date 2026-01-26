#include "unp-labs.h"

#define SERV_PORT 8080
#define LISTENQ 5
#define MAXLINE 100

int main(void)
{
	struct sockaddr_in servaddr;
	int listenfd;
	int maxfd;
	int maxi;
	int client[FD_SETSIZE];
	fd_set allset;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;
	maxi = -1;
	for (int i = 0; i < FD_SETSIZE; i++) {
		client[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for (;;) {
		fd_set rset;
		int n;

		rset = allset;
		n = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (n > 0) {
			struct sockaddr_in cliaddr;
			socklen_t clilen;
			int connfd;

			bzero(&cliaddr, sizeof(cliaddr));
			clilen = sizeof(cliaddr);
			if (FD_ISSET(listenfd, &rset)) {
				connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
				int i;

				for (i = 0; i < FD_SETSIZE; i++) {
					if (client[i] < 0) {
						client[i] = connfd;
						break;
					}
				}
				if (i == FD_SETSIZE) {
					err_quit("too many clients");
				} else {
					FD_SET(connfd, &allset);
					maxfd = (connfd > maxfd) ? connfd : maxfd;
					maxi = (i > maxi) ? i : maxi;
				}

				--n;
			}

			for (int i = 0; n > 0 && i <= maxi; i++) {
				if (client[i] < 0) {
					continue;
				}

				int clisock = client[i];

				if (FD_ISSET(clisock, &rset)) {
					char buffer[MAXLINE];
					ssize_t nread = Read(clisock, buffer, MAXLINE);

					if (nread <= 0) {
						Close(clisock);
						FD_CLR(clisock, &allset);
						client[i] = -1;

						if (i == maxi) {
							while (maxi >= 0 && client[maxi] < 0) {
								maxi--;
							}
						}
						if (clisock == maxfd) {
							maxfd = listenfd;
							for (int j = 0; j <= maxi; j++) {
								if (client[j] > maxfd) {
									maxfd = client[j];
								}
							}
						}
					} else {
						Writen(clisock, buffer, nread);
					}

					--n;
				}
			}
		}
	}

	return 0;
}
