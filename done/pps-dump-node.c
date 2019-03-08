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
#define CLIENT_TIMEOUT 1
    int socket = get_socket(CLIENT_TIMEOUT);
    if (socket == -1) {
        return 1;
    }

    node_t node;
    uint16_t port = 0;
    if (sscanf(argv[2], "%hu", &port) != 1
        || node_init(&node, argv[1], port, 0) != ERR_NONE) {
        printf("FAIL\n");
        return 1;
    }

    if (sendto(socket, "", 1, 0, (struct sockaddr*) node.address, sizeof(*node.address)) == -1) {
        printf("FAIL\n");
        return 1;
    }

    // '+ 1' so that it never points to nothing
    char ans[MAX_MSG_SIZE + 1];
    char** keys = NULL;
    char** values = NULL;
    uint32_t nbr_pairs = 0;
    ssize_t count = -1;
    while (count < nbr_pairs) {
        memset(ans, 0, MAX_MSG_SIZE);
        ssize_t ans_len = recv(socket, &ans, MAX_MSG_SIZE, 0);
        if (ans_len == -1) {
            printf("FAIL\n");
            node_end(&node);
            free(values);
            free(keys);
            return 1;
        }

        size_t current_idx = 0;

        if (count == -1) {
            memcpy(&nbr_pairs, ans, sizeof(uint32_t));
            nbr_pairs = ntohl(nbr_pairs);
            current_idx = sizeof(uint32_t);
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
            char** old_key = keys;
            keys = realloc(keys, sizeof(char*) * (size_t) (count + 1));
            if (keys == NULL) {
                free(values);
                free(old_key);
                return 1;
            }
            keys[count] = strdup(key);

            char** old_val = values;
            values = realloc(values, sizeof(char*) * (size_t) (count + 1));
            if (values == NULL) {
                free(keys);
                free(old_val);
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
    node_end(&node);

    return 0;
}
