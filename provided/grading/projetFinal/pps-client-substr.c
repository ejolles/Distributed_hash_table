/**
 * @file pps-client-substr.c
 * @brief Client that allows extraction of substrings.
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
    client_init_args_t args = {&client, 4, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    pps_value_t val = NULL;
    ssize_t position_s = 0;
    int length_test = 0;

    if (sscanf(argv[1], "%ld", &position_s) != 1
        || sscanf(argv[2], "%d", &length_test) != 1
        || length_test < 0
        || network_get(client, argv[0], &val) != ERR_NONE || val == NULL) {
        printf("FAIL\n");
        free_const_ptr(val);
        client_end(&client);
        return 1;
    }

    size_t length = (size_t) length_test;
    size_t val_len = strlen(val);

    // if position is negative
    size_t position = position_s < 0 ? val_len + (size_t) position_s : (size_t) position_s;

    if (position + length > val_len) {
        printf("FAIL\n");
    } else {
        char new_value[MAX_MSG_SIZE];
        strncpy(new_value, &val[position], length);

        if (network_put(client, argv[3], new_value) == ERR_NONE) {
            printf("OK\n");
        } else {
            printf("FAIL\n");
        }
    }

    free_const_ptr(val);
    client_end(&client);

    return 0;
}

