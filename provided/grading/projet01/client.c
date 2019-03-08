/**
 * @file client.c
 * @brief Client implementation.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */

#include "client.h"

void client_end(client_t *client)
{
    node_list_free(client->nodes);
}


error_code client_init(client_init_args_t args)
{
    args.client->name = args.name;
    args.client->nodes = get_nodes();
    M_EXIT_IF(args.client->nodes == NULL, ERR_NETWORK, "client.c", "%s", ", error with the nodes: they don't exist");

#define CLIENT_TIMEOUT 1
    args.client->socket = get_socket(CLIENT_TIMEOUT);
    //correcteur: il faut appeler node_list_free (ou appeler get_node après) (-0.5)
    M_EXIT_IF(args.client->socket == -1, ERR_NETWORK, "client.c", "%s", ", error with the socket: it doesn't exist");

    return ERR_NONE;
}
