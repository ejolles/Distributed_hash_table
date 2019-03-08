/**
 * @file client.c
 * @brief Client implementation.
 */

#include "client.h"
#include "util.h"
#include "ring.h"

void client_end(client_t *client)
{
    ring_free(client->nodes);
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
    /// Count number of servers
    size_t nb_servers = 0;
    FILE* servers = fopen("servers.txt", "r");
    if (servers == NULL) {
        client_end(args.client);
        return ERR_IO;
    }
    char c;
    while (EOF != (c = (char) getc(servers))) {
        if (c == '\n') {
            ++nb_servers;
        }
    }
    fclose(servers);

    /// Parse arguments if necessary
    if (args.args_kind != 0) {
        args.client->args = parse_opt_args(args.args_kind, args.rem_argv);
        if (args.client->args == NULL
            || nb_servers < args.client->args->N) {
            client_end(args.client);
            return ERR_BAD_PARAMETER;
        }
    }

    size_t args_left = argv_size(*args.rem_argv);
    if (args_left != args.args_req && args.args_req != SIZE_MAX) {
        client_end(args.client);
        return ERR_BAD_PARAMETER;
    }

    // Nodes
    args.client->nodes = ring_alloc();
    if (args.client->nodes == NULL
        || ring_init(args.client->nodes) != ERR_NONE) {
        client_end(args.client);
        return ERR_NETWORK;
    }

    return ERR_NONE;
}
