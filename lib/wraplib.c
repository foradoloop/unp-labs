#include "unp-labs.h"

const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const char *ptr;

	if (!strptr) {
		err_sys("NULL 3rd arg for inet_ntop");
	}
	ptr = inet_ntop(family, addrptr, strptr, len);
	if (ptr == NULL) {
		err_sys("inet_ntop error");
	}
	return ptr;
}

void Inet_pton(int family, const char *strptr, void *addrptr)
{
	int n;

	n = inet_pton(family, strptr, addrptr);

	if (n < 0) {
		err_sys("inet_pton error for %s", strptr);
	} else if (n == 0) {
		err_sys("inet_pton error for %s", strptr);
	}
}

