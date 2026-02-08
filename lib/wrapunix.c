#include "unp-labs.h"

ssize_t writen(int fd, const void *buffer, size_t n)
{
	const char *ptr = buffer;
	ssize_t nleft = n;

	while (nleft > 0) {
		ssize_t nwritten = write(fd, ptr, nleft);

		if (nwritten <= 0) {
			if (nwritten < 0 && errno == EINTR) {
				nwritten = 0;
			} else {
				return -1;
			}
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) == -1) {
		err_sys("writen error");
	}
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
again:
	ssize_t nread = read(fd, ptr, nbytes);

	if (nread < 0) {
		if (errno == EINTR) {
			goto again;
		} else {
			return -1;
		}
	}
	return nread;
}

void Close(int fd)
{
	if (close(fd) == -1) {
		err_sys("close error");
	}
}

void Gettimeofday(struct timeval *tv, void *foo)
{
	if (gettimeofday(tv, foo) < 0) {
		err_sys("gettimeofday error");
	}
}

int Ioctl(int fd, int request, void *arg)
{
	int n;

	n = ioctl(fd, request, arg);
	if (n == -1) {
		err_sys("ioctl error");
	}
	return n;
}

