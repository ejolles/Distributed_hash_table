/**
 * @file pps-client-get.c
 * @brief Client's get implementation.
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
    client_init_args_t args = {&client, 1, TOTAL_SERVERS | GET_NEEDED, (size_t) argc, &argv};

    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    pps_value_t val = NULL;

    if (network_get(client, argv[0], &val) != ERR_NONE || val == NULL) {
        printf("FAIL\n");
        debug_print("%s", "Failed to reach network.");
    } else {
        printf("OK %s\n", val);
    }

    free_const_ptr(val);
    val = NULL;
    client_end(&client);
    return 0;
}

