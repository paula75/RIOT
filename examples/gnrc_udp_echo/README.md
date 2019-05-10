# gnrc_udp_echo example
This example implements a simple udp echo server. It replies any UDP packet the server receives, the server port is specified in the Makefile of this example, if you do not specify the default value is 8808. You can use this example on native or connecting a board.

## Connecting two RIOT instances

When using native (i.e. when you're trying this on your Linux machine),
you first need to set up two tap devices and a bridge that connects
them. This constitutes a virtual network that the RIOT instances can
use to communicate.

    ./../../dist/tools/tapsetup/tapsetup --create 2

Then, make sure you've compiled the application by calling `make` and
start the first RIOT instance by invoking `make term`. In the RIOT
shell, get to know the IP address of this node:

      > ifconfig
          Iface  6  HWaddr: 52:59:AE:9B:95:7A
            L2-PDU:1500 MTU:1500  HL:64  RTR  
            inet6 addr: fe80::5059:aeff:fe9b:957a  scope: local  VAL
            inet6 group: ff02::2
            inet6 group: ff02::1

The example already has a server initialize, so it is ready to receive a message.


In a second terminal, compile [gnrc_networking example](https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_networking) and listen to `tap1`

    PORT=tap1 make term

In the RIOT shell on `tap1`, you can start a udp server on the same port:

    > udp server start 8808

Now you can send a message to the first RIOT instance:

    > udp send fe80::5059:aeff:fe9b:957a 8808 testmessage

In your first terminal, you should now see

   > Received message: testmessage
      Reply sent

Then, the first instances replies to this message to the second, you can add the od_string module in the gnrc_networking Makefile to see the string representation, So in the second RIOT instance you should see:
    > PKTDUMP: data received:
    ~~ SNIP  0 - size:  11 byte, type: NETTYPE_UNDEF (0)
    00000000  74  65  73  74  6D  65  73  73  61  67  65                      testmessage
    ~~ SNIP  1 - size:   8 byte, type: NETTYPE_UDP (3)
       src-port:  8888  dst-port:  8888
       length: 19  cksum: 0x3b88
    ~~ SNIP  2 - size:  40 byte, type: NETTYPE_IPV6 (1)
    traffic class: 0x00 (ECN: 0x0, DSCP: 0x00)
    flow label: 0x00000
    length: 19  next header: 17  hop limit: 64
    source address: fe80::5059:aeff:fe9b:957a
    destination address: fe80::e847:82ff:feac:f54f
    ~~ SNIP  3 - size:  20 byte, type: NETTYPE_NETIF (-1)
    if_pid: 6  rssi: 0  lqi: 0
    flags: 0x0
    src_l2addr: 52:59:AE:9B:95:7A
    dst_l2addr: EA:47:82:AC:F5:4F
    ~~ PKT    -  4 snips, total size:  79 byte

You can change the server port and the delay(in seconds) of the reply in the Makefile
