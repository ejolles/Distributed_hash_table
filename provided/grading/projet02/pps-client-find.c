/**
 * @file pps-client-find.c
 * @brief Client that allows value finding.
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
    client_init_args_t args = {&client, 2, TOTAL_SERVERS | GET_NEEDED | PUT_NEEDED, (size_t) argc, &argv};

    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    // Is initialised to a big value, the check is made in network.c
    pps_value_t val = NULL;
    pps_value_t sub_val = NULL;

    if (network_get(client, argv[0], &val) == ERR_NONE && val != NULL &&
        network_get(client, argv[1], &sub_val) == ERR_NONE && sub_val != NULL) {

        char* occ = strstr(val, sub_val);
        ssize_t index = -1;

        if (occ != NULL) {
            index = occ - val;
        }

        printf("OK %zd\n", index);

    } else {
        printf("FAIL\n");
    }

    free_const_ptr(val);
    val = NULL;
    free_const_ptr(sub_val);
    sub_val = NULL;

    client_end(&client);
    return 0;
}

