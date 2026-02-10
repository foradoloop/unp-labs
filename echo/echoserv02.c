#include "unp-labs.h"

#define SERV_PORT 8080
#define LISTENQ 5
#define MAXLINE 100

void sig_chld(int);
void echo_handler(int connfd);

int main(void)
{
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int listenfd;
	int connfd;
	pid_t clipid;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);

	for (;;) {
		bzero(&cliaddr, sizeof(cliaddr));

		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		clipid = fork();

		if (clipid == 0) {
			Close(listenfd);
			echo_handler(connfd);
			Close(connfd);
			exit(0);
		} else {
			Close(connfd);
		}
	}

	return 0;
}

void sig_chld(int signo)
{
        pid_t pid;
        int stat;

        do {
                pid = waitpid(-1, &stat, WNOHANG);
        } while (pid > 0);
}

void echo_handler(int connfd)
{
	char buffer[MAXLINE];
	ssize_t nread;

again:
	nread = Read(connfd, buffer, MAXLINE);
	if (nread <= 0) {
		return;
	}
	Writen(connfd, buffer, nread);
	goto again;
}
