/**
 * @file pps-client-cat.c
 * @brief Client that allows concatenation of values.
 */

#include <stdio.h>

#include "args.h"
#include "client.h"
#include "error.h"
#include "network.h"
#include "node.h"
#include "system.h"
#include "util.h"

int main(int argc, char* argv[])
{
    // Initialization of the client
    client_t client;
    client_init_args_t args = {&client, SIZE_MAX, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    // Updating remaining elements
    argc = (int) argv_size(argv);

    char new_value[MAX_MSG_SIZE + 1];
    memset(new_value, 0, MAX_MSG_SIZE + 1);
    pps_value_t val = NULL;

    while (argc > 1) {
        free_const_ptr(val);
        val = NULL;
        if (network_get(client, argv[0], &val) != ERR_NONE || val == NULL) {
            memset(new_value, 0, MAX_MSG_SIZE + 1);
            break;
        }
        size_t max_size = MAX_MSG_SIZE - strlen(new_value);
        strncat(new_value, val, max_size);
        --argc;
        ++argv;
    }

    new_value[MAX_MSG_SIZE] = '\0';

    if (strlen(new_value) != 0 && strlen(argv[0]) != 0
        && network_put(client, argv[0], new_value) == ERR_NONE) {
        printf("OK\n");
    } else {
        printf("FAIL\n");
    }

    free_const_ptr(val);
    val = NULL;
    client_end(&client);
    return 0;
}

