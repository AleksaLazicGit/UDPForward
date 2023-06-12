#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdint.h>
#include <arpa/inet.h>

typedef int (*conn_cb_t)(const uint8_t *, uint32_t, uint8_t *, uint32_t *);

typedef struct bridge_conn_t {
	const char *recv_from_ip;
	uint16_t recv_port;
	const char *forward_ip;
	uint16_t forward_port;
	conn_cb_t callback;

	struct sockaddr_in addr_recv;
	struct sockaddr_in addr_send;
	int sock;
} bridge_conn_t;

typedef struct bridge_t {
	bridge_conn_t *conn;
	uint32_t conn_count;
	uint32_t conn_max;
	int *sock;
	int sock_count;
} bridge_t;


int bridge_init(bridge_t *bridge, uint32_t conn_max);

int bridge_add_conn(bridge_t *bridge, 
	const char *recv_from_ip, uint16_t recv_port,
	const char *forward_ip, uint16_t forward_port,
	conn_cb_t callback);

int bridge_start(bridge_t *bridge);

#endif