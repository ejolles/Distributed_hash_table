/**
 * @file pps-client-put.c
 * @brief Client's put implementation.
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
    client_init_args_t args = {&client, "Put-client"};
    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    // Are initialised to a big value, the check is made in network.c
    char key[MAX_MSG_SIZE + 1];
    char value[MAX_MSG_SIZE + 1];

    // Infinite loop
    while(1) {
        memset(key, 0, MAX_MSG_SIZE + 1);
        memset(value, 0, MAX_MSG_SIZE + 1);

        // Ask for the pair key/value
        scanf("%s %s", key, value);
        //correcteur: il faut tester la valeur de retour de scanf (-0.5)
        // Let's make sure that the last character is \0
        key[MAX_MSG_SIZE] = '\0';
        value[MAX_MSG_SIZE] = '\0';

        // If Ctrl+D, then shut client down
        if (feof(stdin)) {
            break;
        }
        while (!ferror(stdin) && getc(stdin) !=  '\n');

        if (network_put(client, key, value) != ERR_NONE) {
            printf("FAIL\n");
            debug_print("%s", "Failed to reach network.");
        } else {
            printf("OK\n");
        }
    }

    client_end(&client);
    return 0;
}

