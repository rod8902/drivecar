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

int sock_rx_open(int port)
{
    int sock;
    int ret;
    struct sockaddr_in saddr;
    const unsigned int one = 1;
    int flags;

    // set content of struct saddr and imreq to zero
    memset(&saddr, 0, sizeof(struct sockaddr_in));

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
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);	// from any host
    ret = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret < 0) {
	perror("binding socket to interface");
	goto fail;
    }

    return sock;

fail:
    // close socket
    close(sock);

    return -1;
}

int sock_recvfrom(int sock, void *buf, int buflen, struct sockaddr *saddr)
{
    socklen_t socklen;
    int ret;

    socklen = sizeof(struct sockaddr_in);
    ret = recvfrom(sock, buf, buflen, 0,
	    (struct sockaddr *)&saddr, &socklen);

    return ret;
}
