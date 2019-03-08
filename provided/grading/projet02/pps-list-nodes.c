/*
 * @file pps-list-nodes.c
 * @brief Tests all nodes present in servers.txt
 */

#include <stdio.h>

#include "client.h"
#include "config.h"
#include "error.h"
#include "node.h"
#include "system.h"


int main(int argc, char* argv[])
{
    client_t client;
    client_init_args_t args = {&client, 0, 0, (size_t)argc, &argv};
    if (client_init(args) != ERR_NONE) {
        debug_print("%s", "Client could not be initialized.");
        //correcteur: il faut print FAIL (-1)
        return 1;
    }
#define MAX_LINE_SIZE 100
    FILE* servers = fopen(PPS_SERVERS_LIST_FILENAME, "r");
    //correcteur: client_init a déjà initialiser une node_list via get_nodes (-2)
    if (servers != NULL) {
        char line[MAX_LINE_SIZE];
        int i = 0;
        while (fgets(line, MAX_LINE_SIZE, servers) != NULL && strlen(line) > 1) {
            line[strcspn(line, "\n")] = '\0';
            printf("%s ", line);
            node_t node = client.nodes->nodes[i];
            sendto(client.socket, NULL, 0, 0, (struct sockaddr*)&(node), sizeof(node));
            if (recv(client.socket, NULL, 0, 0) == 0) {
                printf("OK");
            } else {
                printf("FAIL");
            }
            putchar('\n');
            ++i;
        }
    }

    fclose(servers);
    client_end(&client);

    return 0;
}
