#include "unp-labs.h"

#define SERV_PORT	8080
#define LISTENQ		5
#define MAXLINE		100

int main(void)
{
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	fd_set allset;
	fd_set rset;
	int listenfd;
	int connfd;
	int clisock;
	int maxfd;
	int maxi;
	int nready;
	int client[FD_SETSIZE];
	char buffer[MAXLINE];
	ssize_t nread;

	memset(&servaddr, 0, sizeof(servaddr));
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
		rset = allset;
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (nready > 0) {
			if (FD_ISSET(listenfd, &rset)) {
				memset(&cliaddr, 0, sizeof(cliaddr));
				clilen = sizeof(cliaddr);
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
					if (connfd > maxfd) {
						maxfd = connfd;
					}
					if (i > maxi) {
						maxi = i;
					}
				}

				nready--;
			}

			for (int i = 0; nready > 0 && i <= maxi; i++) {
				if (client[i] < 0) {
					continue;
				}

				clisock = client[i];

				if (FD_ISSET(clisock, &rset)) {
					nread = Read(clisock, buffer, MAXLINE);

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

					nready--;
				}
			}
		}
	}

	return 0;
}

