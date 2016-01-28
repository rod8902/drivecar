#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef PALS_DEBUG
#include <assert.h>
#endif

int sock_mcast_rx_open(in_addr_t mcast_addr, int port)
{
    int sock;
    int ret;
    struct sockaddr_in saddr;
    struct ip_mreq imreq;
    const unsigned int one = 1;
    int flags;

    // set content of struct saddr and imreq to zero
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&imreq, 0, sizeof(struct ip_mreq));

    // open a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
	perror("Error creating socket");
	return -1;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (ret < 0) {
	perror("set socket option of reuse addr");
	goto fail;
    }

    // set non blocking
    flags = fcntl(sock,F_GETFL,0);
#ifdef PALS_DEBUG
    assert(flags != -1);
#endif
    ret = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
	perror("fcntl");
	goto fail;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = mcast_addr;
    //saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
    ret = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret < 0) {
	perror("binding socket to interface");
	goto fail;
    }

    imreq.imr_multiaddr.s_addr = mcast_addr;
    imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface

    // JOIN multicast group on default interface
    ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
	    (const void *)&imreq, sizeof(struct ip_mreq));
    if (ret < 0) {
	perror("IP_ADD_MEMBERSHIP");
	goto fail;
    }
    //printf("joined to mcast group %s\n", MCAST_GRP);

    return sock;

fail:
    // close socket
    close(sock);

    return -1;
}
