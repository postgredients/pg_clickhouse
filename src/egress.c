#include "postgres.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "egress.h"

char *ch_egress_interface;

static bool
ch_egress_address_allowed(const struct sockaddr *address,
						  socklen_t address_len)
{
	if (address == NULL)
		return false;

	if (address->sa_family == AF_INET &&
		address_len >= sizeof(struct sockaddr_in))
	{
		const struct sockaddr_in *address4 =
			(const struct sockaddr_in *) address;
		uint32		addr = ntohl(address4->sin_addr.s_addr);

		return (addr & 0xff000000U) != 0 &&
			(addr & 0xff000000U) != 0x7f000000U &&
			(addr & 0xffff0000U) != 0xa9fe0000U;
	}

	if (address->sa_family == AF_INET6 &&
		address_len >= sizeof(struct sockaddr_in6))
	{
		const struct in6_addr *addr =
			&((const struct sockaddr_in6 *) address)->sin6_addr;

		if (IN6_IS_ADDR_UNSPECIFIED(addr) || IN6_IS_ADDR_LOOPBACK(addr) ||
			IN6_IS_ADDR_LINKLOCAL(addr))
			return false;
		if (IN6_IS_ADDR_V4MAPPED(addr) || IN6_IS_ADDR_V4COMPAT(addr))
		{
			struct sockaddr_in mapped = {.sin_family = AF_INET};

			memcpy(&mapped.sin_addr, &addr->s6_addr[12], sizeof(mapped.sin_addr));
			return ch_egress_address_allowed((const struct sockaddr *) &mapped,
										 sizeof(mapped));
		}
		return true;
	}

	return false;
}

int
ch_egress_socket(int domain, int type, int protocol,
				 const struct sockaddr *address, socklen_t address_len)
{
	int			fd;

	if (ch_egress_interface == NULL || ch_egress_interface[0] == '\0')
		return socket(domain, type, protocol);
	if (!ch_egress_address_allowed(address, address_len))
	{
		errno = EACCES;
		return -1;
	}

	fd = socket(domain, type, protocol);
	if (fd < 0)
		return -1;

#ifdef SO_BINDTODEVICE
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ch_egress_interface,
				   strlen(ch_egress_interface) + 1) == 0)
		return fd;
#else
	errno = ENOTSUP;
#endif

	int			save_errno = errno;

	close(fd);
	errno = save_errno;
	return -1;
}

curl_socket_t
ch_egress_curl_socket(void *clientp, curlsocktype purpose,
					  struct curl_sockaddr *address)
{
	(void) clientp;
	(void) purpose;

	return ch_egress_socket(address->family, address->socktype,
							address->protocol, &address->addr,
							address->addrlen);
}
