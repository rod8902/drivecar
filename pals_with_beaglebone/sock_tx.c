#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int sock_tx_open()
{
    int sock;

    // open a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	perror("socket");
	return -1;
    }

    return sock;
}

int sock_sendto(int sock, void *buf, int len, in_addr_t addr, int port)
{
    struct sockaddr_in saddr;
    int ret;

    memset(&saddr, 0, sizeof(struct sockaddr_in));

    // set destination multicast address
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = addr;
    saddr.sin_port = htons((short)port);

    ret = sendto(sock, buf, len, 0, (struct sockaddr *)&saddr, sizeof(saddr));

    return ret;
}
