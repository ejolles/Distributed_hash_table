/**
 * @file pps-lauch-server.c
 * @brief Server implementation.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "hashtable.h"
#include "system.h"


int main(void)
{
    int socket = get_socket(0);
    //correcteur: il faut tester la valeur de retour de get_socket() (-1)
    char ip[15] = PPS_DEFAULT_IP; //correcteur: il faut utiliser 15 + 1 pour le \0 (-0.5)
    //correcteur: int16_t != uint16_t ! (-0.5)
    int16_t port = PPS_DEFAULT_PORT;

    // Ask for the IP address
    do {
        printf("IP port? ");
    } while (fscanf(stdin, "%15s %hd", ip, &port) != 2);

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

        // The key and value just point to the in_msg. We needed to
        // add 1 to the size so that the value terminates with \0
        char* key = in_msg;
        char* val = memchr(in_msg, '\0', in_msg_len);

        // Point to the first character of the value if exisitng
        if (val != NULL) {
            val = &val[1];
        }

        debug_print("SERVER: received %s %s", key, val);

        // ---- It is a get request ----
        if (val == NULL || strlen(val) == 0) {
            //correcteur: val a déjà été défini plus haut
            pps_value_t val = get_Htable_value(table, key);

            debug_print("SERVER: retrieved %s with key %s", val, key);

            if (val == NULL) {
                //correcteur: attention '\0' != "\0"
                sendto(socket, "\0", 1, 0, (struct sockaddr *) &cli_addr, addr_len);
            } else {
                sendto(socket, val, strlen(val), 0, (struct sockaddr *) &cli_addr, addr_len);
            }
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
