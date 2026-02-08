#include "unp-labs.h"

int Socket(int family, int type, int protocol)
{
	int n = socket(family, type, protocol);
	if (n < 0) {
		err_sys("socket error");
	}
	return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0) {
		err_sys("bind error");
	}
}

void Listen(int fd, int backlog)
{
	if (listen(fd, backlog) < 0) {
		err_sys("listen error");
	}
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salen)
{
	int n;
again:
	n = accept(fd, sa, salen);
	if (n < 0) {
#ifdef EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			err_sys("accept error");
	}
	return n;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int n = select(nfds, readfds, writefds, exceptfds, timeout);

	if (n < 0) {
		err_sys("select error");
	}
	return n;
}

void Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	if (sendto(sockfd, buf, len, flags, dest_addr, addrlen) != (ssize_t)len) {
		err_sys("sendto error");
	}
}

void Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0) {
		err_sys("setsockopt error");
	}
}

