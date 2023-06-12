#include <stdlib.h>
#include <string.h>

#include "bridge.h"

#define BUFFER_SIZE 2048

static void bridge_process_socket(bridge_t *bridge, int sock);

int bridge_init(bridge_t *bridge, uint32_t conn_max)
{
	bridge->conn_max = conn_max;
	bridge->conn_count = 0;
	bridge->sock_count = 0;

	bridge->conn = calloc(conn_max, sizeof(bridge_conn_t));
	if(!bridge->conn)
		return 0;

	bridge->sock = calloc(conn_max, sizeof(int));
	if(!bridge->sock) {
		free(bridge->conn);
		return 0;
	}

	return 1;
}


int bridge_add_conn(bridge_t *bridge, 
	const char *recv_from_ip, uint16_t recv_port,
	const char *forward_ip, uint16_t forward_port,
	conn_cb_t callback)
{

	bridge_conn_t *conn, *t;
	int sock_exists, *sock;

	if(bridge->conn_count >= bridge->conn_max)
		return 0;

	conn = &bridge->conn[bridge->conn_count];

	/* copy passed parameters */
	conn->callback = callback;
	conn->recv_from_ip = recv_from_ip;
	conn->recv_port = recv_port;
	conn->forward_ip = forward_ip;
	conn->forward_port = forward_port;
	conn->callback = callback;

	/* fill address information */
	conn->addr_recv.sin_family = AF_INET;
	conn->addr_recv.sin_port = htons(recv_port);
	conn->addr_recv.sin_addr.s_addr = INADDR_ANY;

	conn->addr_send.sin_family = AF_INET;
	conn->addr_send.sin_port = htons(forward_port);
	inet_aton(forward_ip, &conn->addr_send.sin_addr);

	/* check if socket was already created for this receive port */
	sock_exists = 0;
	for(int i = 0; i < bridge->conn_count; i++) {
		t = &bridge->conn[i];
		if(conn->recv_port == t->recv_port) {
			sock_exists = 1;
			conn->sock = t->sock;
		}
	}

	/* if not create and bind new socket */
	if(!sock_exists) {
		sock = &bridge->sock[bridge->sock_count];
		*sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(*sock == -1)
			return 0;

		if(bind(*sock, (const struct sockaddr *)&conn->addr_recv, sizeof(conn->addr_recv)) == -1)
			return 0;

		conn->sock = *sock;
		bridge->sock_count++;
	}

	bridge->conn_count++;

	return 1;

}

int bridge_start(bridge_t *bridge)
{
	while(1) {
		for(int i = 0; i < bridge->sock_count; i++) {
			bridge_process_socket(bridge, bridge->sock[i]);
		}
	}

}

static void bridge_process_socket(bridge_t *bridge, int sock)
{

	uint8_t recv_buffer[BUFFER_SIZE];
	uint8_t send_buffer[BUFFER_SIZE];

	int recv_size, send_size;
	struct sockaddr_in msg_addr;
	socklen_t msg_addr_len;

	bridge_conn_t *conn;

	/* try to receive message on socket */
	recv_size = recvfrom(sock, recv_buffer, sizeof(recv_buffer),
		MSG_DONTWAIT, (struct sockaddr *)&msg_addr, &msg_addr_len);
	if(recv_size <= 0)
		return;
	
	/* see if there are any connections for the ip-port pair*/
	for(int i = 0; i < bridge->conn_count; i++) {
		conn = &bridge->conn[i];
		if(conn->sock != sock)
			continue;
		if(strcmp(inet_ntoa(msg_addr.sin_addr), conn->recv_from_ip) != 0)
			continue;

		/* if callback is defined process buffer before sending */
		if(conn->callback) {
			int ret = conn->callback(recv_buffer, recv_size, send_buffer, &send_size);
			if(!ret)
				continue;

			sendto(sock, send_buffer, send_size, MSG_DONTWAIT,
				(const struct sockaddr *)&conn->addr_send, sizeof(conn->addr_send));
		/* else just forward received buffer */
		} else {
			sendto(sock, recv_buffer, recv_size, MSG_DONTWAIT, 
				(const struct sockaddr *)&conn->addr_send, sizeof(conn->addr_send));
		}		
	}

}
