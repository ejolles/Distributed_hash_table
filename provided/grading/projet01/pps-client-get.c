/**
 * @file pps-client-get.c
 * @brief Client's get implementation.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */

#include <stdio.h>

#include "client.h"
#include "error.h"
#include "network.h"
#include "node.h"
#include "system.h"

int main(void)
{
    // Initialization of the client
    client_t client;
    client_init_args_t args = {&client, "Get-client"};
    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    // Is initialised to a big value, the check is made in network.c
    char key[MAX_MSG_SIZE + 1];
    pps_value_t val = NULL;

    // Infinite loop
    while (1) {
        if (val != NULL) {
            free_const_ptr(val);
            val = NULL;
        }

        memset(key, 0, MAX_MSG_SIZE + 1);

        // Ask for a key
        scanf("%s", key);
        //correcteur:il faut tester la valeur de retour de scanf (-0.5)

        // Let's make sure that the last character is \0
        key[MAX_MSG_SIZE] = '\0';

        // If Ctrl+D, then shut client down
        if (feof(stdin)) {
            break;
        }
        while (!ferror(stdin) && getc(stdin) != '\n');

        if (network_get(client, key, &val) != ERR_NONE || val == NULL) {
            printf("FAIL\n");
            debug_print("%s", "Failed to reach network.");
        } else if (val[0] == '\0') {
            printf("FAIL\n");
            debug_print("No existing value for %s", key);
        } else {
            printf("OK %s\n", val);
        }
    }

    client_end(&client);
    return 0;
}

