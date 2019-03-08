/**
 * @file pps-lauch-server.c
 * @brief Server implementation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "hashtable.h"
#include "system.h"


int main(void)
{
#define IP_LENGTH 15
    int socket = get_socket(0);
    if (socket == -1) {
        return 1;
    }
    char ip[IP_LENGTH + 1] = PPS_DEFAULT_IP;
    uint16_t port = PPS_DEFAULT_PORT;

    // Ask for the IP address
    do {
        printf("IP port? ");
    } while (fscanf(stdin, "%15s %hu", ip, &port) != 2);

    // Bind the server with an IP address and a PORT number
    if (bind_server(socket, ip, port)) {
        debug_print("%s", "Bind server failed.");
        return 1;
    }

    // Initialize a hash-table
    Htable_t table = construct_Htable(HTABLE_SIZE);
    // NB: the value also has to finish with \0 when passed to the Htable
    // -- cf implementation below
    char in_msg[MAX_MSG_SIZE + 1];
    struct sockaddr_in cli_addr;
    socklen_t addr_len;

    // Loop forever
    while (1) {
        // Reset values
        memset(in_msg, 0, MAX_MSG_SIZE + 1);
        memset(&cli_addr, 0, addr_len);
        addr_len = sizeof(cli_addr);

        ssize_t in_msg_len = recvfrom(socket, in_msg, MAX_MSG_SIZE, 0, (struct sockaddr *) &cli_addr, &addr_len);

        if (in_msg_len == -1) {
            debug_print("%s", "SERVER: The input message could not be read.");
            continue;
        }

        debug_print("%s", "A message was received.");

        char* key = in_msg;
        char* val = memchr(in_msg, '\0', (size_t)in_msg_len);

        size_t val_len = 0;

        // Point to the first character of the value if existing
        if (val != NULL) {
            if (strlen(key) + 1 == (size_t)in_msg_len) {
                val_len = 1;
            } else {
                val = &val[1];
                val_len = strlen(val);
            }
        }

        debug_print("SERVER: received %s %s", key, val);

        // It is a check request from pps-list_node
        if (in_msg_len == 0) {
            sendto(socket, NULL, 0, 0, (struct sockaddr *) &cli_addr, addr_len);
        } // It is a dump-node request
        else if (in_msg_len == 1 && in_msg[0] == '\0') {
            kv_list_t* list = get_Htable_content(table);

            // Send number of pairs which will be sent
            uint32_t size_to_send = htonl((uint32_t)list->size);

            // Send pairs
            //correcteur: macro MAX_MSG_SIZE existe aussi si jamais
#define MAX_UDP_SIZE 65507
            char* msg = NULL;
            msg = calloc(MAX_UDP_SIZE, sizeof(char));
            if (msg == NULL) {
                kv_list_free(list);
                continue;
            }
            //correcteur: on peut utiliser memset
            for (size_t i = 0; i < sizeof(int32_t); ++i) {
                msg[i] = (char) (size_to_send >> (8 * (3-i)));
            }
            size_t current_size = sizeof(int32_t);
            for (size_t i = 0; i < list->size; ++i) {
                key = (char*) list->pairs[i].key;
                pps_value_t value = list->pairs[i].value;
                size_t key_len = strlen(key) + 1;
                size_t value_len = strlen(value) + 1;
                //correcteur: attention, il ne faut pas mettre de \0 après value si c'est la dernière du message (-0.5)
                if (current_size + key_len + value_len > MAX_UDP_SIZE) {
                    // send to client
                    sendto(socket, msg, current_size * sizeof(char), 0, (struct sockaddr *) &cli_addr, addr_len);
                    memset(msg, 0, MAX_UDP_SIZE * sizeof(char));
                    current_size = 0;
                }
                strcpy(&msg[current_size], key);
                current_size += key_len;
                strcpy(&msg[current_size], value);
                current_size += value_len;
            }
            sendto(socket, msg, current_size * sizeof(char), 0, (struct sockaddr *) &cli_addr, addr_len);
            free(msg);
            kv_list_free(list);
            list = NULL;
        }
        // ---- It is a get request ----
        else if (val == NULL || val_len == 0) {
            val = (char*) get_Htable_value(table, key);

            debug_print("SERVER: retrieved %s with key %s", val, key);

            if (val == NULL) {
                char m = '\0';
                sendto(socket, &m, 1, 0, (struct sockaddr *) &cli_addr, addr_len);
            } else {
                sendto(socket, val, strlen(val), 0, (struct sockaddr *) &cli_addr, addr_len);
            }
            free(val);
        }
        // ---- It is a put request ----
        else {
            add_Htable_value(table, key, val);
            sendto(socket, NULL, 0, 0, (struct sockaddr *) &cli_addr, addr_len);
        }
        debug_print("%s", "Finished cycle.");
    }

    delete_Htable_and_content(&table);
    return 0;
}
