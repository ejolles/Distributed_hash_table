/**
 * @file node_list.c
 * @brief Parsing and handling of a list of nodes.
 */
#include "node_list.h"

node_list_t *node_list_new()
{
    node_list_t* list = malloc(sizeof(node_list_t));
    if (list != NULL) {
        memset(list, 0, sizeof(node_list_t));
    }
    return list;
}

node_list_t *get_nodes()
{
#define IP_LENGTH 15
    node_list_t* list = node_list_new();

    if(list != NULL) {
        FILE* servers = fopen(PPS_SERVERS_LIST_FILENAME, "r");

        if (servers != NULL) {
            char ip[IP_LENGTH + 1] = PPS_DEFAULT_IP;
            uint16_t port = PPS_DEFAULT_PORT;

            while (fscanf(servers, "%15s %hu", ip, &port) == 2) {
                node_t node;
                if (node_init(&node, ip, port, 0) != ERR_NONE ||
                    node_list_add(list, node)) {
                    node_list_free(list);
                    fclose(servers);
                    return NULL;
                }
            }
            fclose(servers);
        }
    }
    return list;
}

error_code node_list_add(node_list_t *list, node_t node)
{
    if (list == NULL) {
        return ERR_BAD_PARAMETER;
    }

    node_t* old_nodes = list->nodes;

    if((list->size + 1) > SIZE_MAX / sizeof(node_t) ||
       ((list->nodes = realloc(list->nodes,
                               (list->size + 1) * sizeof(node_t))) == NULL)) {
        list->nodes = old_nodes;
        return ERR_NOMEM;
    }

    list->nodes[list->size] = node;
    list->size += 1;
    return ERR_NONE;
}

void node_list_free(node_list_t *list)
{
    if(list != NULL) {
        if(list->nodes != NULL) {
            for (size_t i = 0; i < list->size; ++i) {
                node_end(&list->nodes[i]);
            }
            free(list->nodes);
        }
        free(list);
    }
}
