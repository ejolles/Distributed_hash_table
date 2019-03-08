/**
 * @file pps-client-put.c
 * @brief Client's put implementation.
 */

#include <stdio.h>

#include "client.h"
#include "error.h"
#include "network.h"
#include "node.h"
#include "system.h"

int main(int argc, char* argv[])
{
    // Initialization of the client
    client_t client;
    client_init_args_t args = {&client, 2, TOTAL_SERVERS | PUT_NEEDED, (size_t) argc, &argv};
    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    if (network_put(client, argv[0],argv[1]) != ERR_NONE) {
        printf("FAIL\n");
        debug_print("%s", "Failed to reach network.");
    } else {
        printf("OK\n");
    }

    client_end(&client);
    return 0;
}

