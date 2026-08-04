#ifndef PG_CLICKHOUSE_EGRESS_H
#define PG_CLICKHOUSE_EGRESS_H

#include <curl/curl.h>
#include <sys/socket.h>

extern char *ch_egress_interface;
extern int ch_egress_socket(int domain, int type, int protocol,
							const struct sockaddr *address,
							socklen_t address_len);
extern curl_socket_t ch_egress_curl_socket(void *clientp,
										  curlsocktype purpose,
										  struct curl_sockaddr *address);

#endif
