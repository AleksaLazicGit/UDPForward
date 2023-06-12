#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bridge.h"

int callback(const uint8_t *old_buffer, uint32_t old_buffer_size, uint8_t *new_buffer, uint32_t *new_buffer_size)
{
	memcpy(new_buffer, old_buffer, old_buffer_size);
	*new_buffer_size = old_buffer_size;

	printf("%i\n", old_buffer_size);

	return 1;
}


int main(int argc, char **argv)
{

	bridge_t bridge;

	bridge_init(&bridge, 32);

	bridge_add_conn(&bridge, "192.168.1.150", 1024, "192.168.1.151", 1025, NULL);
	bridge_add_conn(&bridge, "192.168.1.150", 1024, "192.168.1.152", 1025, NULL);
	bridge_add_conn(&bridge, "192.168.1.150", 1024, "192.168.1.153", 1025, NULL);
	bridge_add_conn(&bridge, "192.168.1.150", 1024, "192.168.1.154", 1025, NULL);
//	bridge_add_conn(&bridge, "192.168.1.150", 1025, "192.168.1.155", 1025, NULL);

	bridge_add_conn(&bridge, "192.168.1.151", 1026, "192.168.1.150", 1024, NULL);
	bridge_add_conn(&bridge, "192.168.1.152", 1026, "192.168.1.150", 1024, NULL);
//	bridge_add_conn(&bridge, "192.168.1.153", 1026, "192.168.1.150", 1024, NULL);
//	bridge_add_conn(&bridge, "192.168.1.154", 1026, "192.168.1.150", 1024, NULL);
//	bridge_add_conn(&bridge, "192.168.1.155", 1026, "192.168.1.150", 1024, NULL);


	bridge_start(&bridge);

}
