#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int sock_mcast_tx_open(unsigned char ttl)
{
    int sock;
    int ret;
    unsigned char one = 1;

    // open a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	perror("socket");
	return -1;
    }

    // Set multicast packet TTL
    ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
	    sizeof(unsigned char));
    if (ret < 0) {
	perror("setsockopt");
	goto fail;
    }

    // send multicast traffic to myself too
    ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
	    &one, sizeof(unsigned char));
    if (ret < 0) {
	perror("setsockopt");
	goto fail;
    }

    return sock;

fail:
    close(sock);
    return -1;
}

int sock_mcast_send(int sock, void *buf, int len, in_addr_t mcast_addr, int port)
{
    struct sockaddr_in saddr;
    int ret;

    memset(&saddr, 0, sizeof(struct sockaddr_in));

    // set destination multicast address
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = mcast_addr;
    saddr.sin_port = htons((short)port);

    ret = sendto(sock, buf, len, 0, (struct sockaddr *)&saddr, sizeof(saddr));

    return ret;
}
