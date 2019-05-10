/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 *
 * @author      Martine Lenders <m.lenders@fu-berlin.de>
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/sock/udp.h"
#include "net/gnrc/pktdump.h"
#include "thread.h"
#include "xtimer.h"

#define SERVER_MSG_QUEUE_SIZE   (8U)
#define SERVER_BUFFER_SIZE      (32)

static char server_stack[THREAD_STACKSIZE_DEFAULT];
uint8_t buf[128];
sock_udp_t sock;
sock_udp_ep_t local = SOCK_IPV6_EP_ANY;

static void *_server_thread(void *args)
{
    int delay;
    delay = (int) args;

    while (1) {

        sock_udp_ep_t remote;
        ssize_t res;

        if ((res = sock_udp_recv(&sock, buf, sizeof(buf), SOCK_NO_TIMEOUT,
                                 &remote)) >= 0) {
            printf("Received message: ");
            for(int i = 0; i < res; i++ ){
              printf("%c", buf[i]);
            }
            printf("\n");

            xtimer_sleep(delay);
            /* Form reply */
            if (sock_udp_send(&sock, buf, res, &remote) < 0) {
                puts("Error sending reply");
            }
            else{
              printf("Reply sent after %i seconds\n", delay);
            }
        }
    }
    return NULL;
}

static void send(char *addr_str, char *port_str, char *data, unsigned int num,
                 unsigned int delay)
{
    int iface;
    uint16_t port;
    ipv6_addr_t addr;

    /* get interface, if available */
    iface = ipv6_addr_split_iface(addr_str);
    if ((iface < 0) && (gnrc_netif_numof() == 1)) {
        iface = gnrc_netif_iter(NULL)->pid;
    }
    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("Error: unable to parse destination address");
        return;
    }
    /* parse port */
    port = atoi(port_str);
    if (port == 0) {
        puts("Error: unable to parse destination port");
        return;
    }

    for (unsigned int i = 0; i < num; i++) {
        gnrc_pktsnip_t *payload, *udp, *ip;
        unsigned payload_size;
        /* allocate payload */
        payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            puts("Error: unable to copy data to packet buffer");
            return;
        }
        /* store size for output */
        payload_size = (unsigned)payload->size;
        /* allocate UDP header, set source port := destination port */
        udp = gnrc_udp_hdr_build(payload, port, port);
        if (udp == NULL) {
            puts("Error: unable to allocate UDP header");
            gnrc_pktbuf_release(payload);
            return;
        }
        /* allocate IPv6 header */
        ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
        if (ip == NULL) {
            puts("Error: unable to allocate IPv6 header");
            gnrc_pktbuf_release(udp);
            return;
        }
        /* add netif header, if interface was given */
        if (iface > 0) {
            gnrc_pktsnip_t *netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

            ((gnrc_netif_hdr_t *)netif->data)->if_pid = (kernel_pid_t)iface;
            LL_PREPEND(ip, netif);
        }
        /* send packet */
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return;
        }
        /* access to `payload` was implicitly given up with the send operation above
         * => use temporary variable for output */
        printf("Success: sent %u byte(s) to [%s]:%u\n", payload_size, addr_str,
               port);
        xtimer_usleep(delay);
    }
}

int udp_start_server(int udp_echo_port, int *delay)
{
    local.port = udp_echo_port;

    /* Create new UDP socket object*/
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return -1;
    }
    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST,
                      _server_thread, delay, "UDP server") <= KERNEL_PID_UNDEF)
    {
        puts("error initializing server thread");
        return -1;
    }
    printf("UDP sock created on port: %i\n", udp_echo_port);

    return 1;
}


int udp_cmd(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s [send]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "send") == 0) {
        uint32_t num = 1;
        uint32_t delay = 1000000;
        if (argc < 5) {
            printf("usage: %s send <addr> <port> <bytes> [<num> [<delay in us>]]\n",
                   argv[0]);
            return 1;
        }
        if (argc > 5) {
            num = atoi(argv[5]);
        }
        if (argc > 6) {
            delay = atoi(argv[6]);
        }
        send(argv[2], argv[3], argv[4], num, delay);
    }
    else {
        puts("error: invalid command");
    }
    return 0;
}
