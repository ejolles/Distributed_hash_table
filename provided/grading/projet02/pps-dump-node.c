/**
 * @file pps-dump-node.c
 * @brief Prompt all key-value pairs of a specific server
 */

#include <stdio.h>

#include "client.h"
#include "error.h"
#include "node.h"
#include "config.h"
#include "system.h"


int main(int argc, char* argv[])
{
    client_t client;
    //correcteur: il ne faut pas faire de client ici, c'est un utilitaire pour le debug
    client_init_args_t args = {&client, 2, 0, (size_t) argc, &argv};

    if (client_init(args) != ERR_NONE) {
        return 1;
    }

    node_t node;
    uint16_t port = 0;
    if (sscanf(argv[1], "%hu", &port) != 1
        || node_init(&node, argv[0], port, 0) != ERR_NONE) {
        printf("FAIL\n");
        client_end(&client);
        return 1;
    }
    //correcteur: attention "\0" != '\0' ~ ""
    sendto(client.socket, "\0", 1, 0, (struct sockaddr*)&(node), sizeof(node));


#define MAX_UDP_SIZE 65507 //correcteur: la macro MAX_MSG_SIZE existe
    // '+ 1' so that it never points to nothing
    char ans[MAX_UDP_SIZE + 1];
    char** keys = NULL;
    char** values = NULL;
    uint32_t nbr_pairs = 0;
    ssize_t count = -1;
    while (count < nbr_pairs) {
        memset(ans, 0, MAX_UDP_SIZE * sizeof(char));
        ssize_t ans_len = recv(client.socket, &ans, MAX_UDP_SIZE * sizeof(char), 0);
        if (ans_len == -1) {
            printf("FAIL\n");
            client_end(&client);
            return 1;
        }

        size_t current_idx = 0;

        if (count == -1) {
            for (size_t i = 0; i < sizeof(int32_t); ++i) {
                nbr_pairs = nbr_pairs << 8;
                nbr_pairs += ans[i];
            }
            nbr_pairs = ntohl(nbr_pairs);
            current_idx = 4;
            ++count;
        }

        // These lengths take into account the \0
        size_t key_len = 0;
        size_t value_len = 0;

        while ((key_len = strlen(&ans[current_idx]) + 1) > 1) {
            char* key = &ans[current_idx];
            current_idx += key_len;
            value_len = strlen(&ans[current_idx]) + 1;
            char* value = &ans[current_idx];
            current_idx += value_len;

            // Lets copy the keys and values
            keys = realloc(keys, sizeof(char*) * (size_t) (count + 1));
            if (keys == NULL) {
                debug_print("%s", "KEYS NULL.");
                return 1;
            }
            keys[count] = strdup(key);

            values = realloc(values, sizeof(char*) * (size_t) (count + 1));
            if (values == NULL) {
                debug_print("%s", "VALUES NULL.");
                return 1;
            }
            values[count] = strdup(value);

            ++count;
        }
    }

    // PROMPT ALL KEYS AND VALUES ACCORDING TO PDF
    for (size_t i = 0; i < (size_t) count; ++i) {
        printf("%s = %s\n", keys[i], values[i]);
        free(keys[i]);
        free(values[i]);
    }
    free(keys);
    free(values);

    client_end(&client);

    return 0;
}
