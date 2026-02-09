#include "unp-labs.h"

#define SERV_PORT 8080
#define LISTENQ 5
#define MAXLINE 100

int main(void)
{
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int listenfd;
	int connfd;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for (;;) {
		bzero(&cliaddr, sizeof(cliaddr));
		
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

		for (;;) {
			char buffer[MAXLINE];
			ssize_t nread;
			
			nread = Read(connfd, buffer, MAXLINE);

			if (nread <= 0) {
				Close(connfd);
				break;
			}
			Writen(connfd, buffer, nread);
		}
	}

	return 0;
}
