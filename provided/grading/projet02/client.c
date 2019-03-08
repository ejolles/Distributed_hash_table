/**
 * @file client.c
 * @brief Client implementation.
 */

#include "client.h"
#include "util.h"

#define min(x,y) ((x) < (y) ? (x) : (y))

void client_end(client_t *client)
{
    node_list_free(client->nodes);
    free(client->args);
}

error_code client_init(client_init_args_t args)
{
    // Initialisation
    memset(args.client, 0, sizeof(client_t));

    // Name
    args.client->name = *args.rem_argv[0];
    ++(*args.rem_argv);

    // Arguments
    M_EXIT_IF(args.args_req != SIZE_MAX && args.argc - 1 < args.args_req, ERR_BAD_PARAMETER, "client.c", "%s", ", more arguments required than given");

    args.client->args = parse_opt_args(args.args_kind, args.rem_argv);

    size_t args_left = argv_size(*args.rem_argv);
    if (args.client->args == NULL || (args_left != args.args_req && args.args_req != SIZE_MAX)) {
        client_end(args.client);
        return ERR_BAD_PARAMETER;
    }

    // Nodes
    args.client->nodes = get_nodes();
    if (args.client->nodes == NULL) {
        client_end(args.client);
        return ERR_NETWORK;
    }
    //correcteur: il faut tester que args.client->nodes->size == args.clinet->args->N (-0.5)

    args.client->args->N = min(args.client->args->N, args.client->nodes->size);
    args.client->args->R = min(args.client->args->R, args.client->args->N);
    args.client->args->W = min(args.client->args->W, args.client->args->N);

    // Socket
#define CLIENT_TIMEOUT 1
    args.client->socket = get_socket(CLIENT_TIMEOUT);
    if (args.client->socket == -1) {
        client_end(args.client);
        return ERR_NETWORK;
    }

    return ERR_NONE;
}
