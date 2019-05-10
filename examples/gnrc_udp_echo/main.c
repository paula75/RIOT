/*
 * Copyright (c)
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
 * @brief       udp echo example
 *
 * @}
 */


#include <stdio.h>
#include "shell.h"

extern int udp_cmd(int argc, char **argv);
extern void udp_start_server(int udp_echo_port, int delay);

const shell_command_t shell_commands[] = {
   { "udp", "send data over UDP and listen on UDP ports", udp_cmd },
   { NULL, NULL, NULL }
};

int main(void)
{
    udp_start_server(UDP_ECHO_PORT, UDP_ECHO_DELAY);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
