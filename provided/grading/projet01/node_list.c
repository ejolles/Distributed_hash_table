/**
 * @file node_list.c
 * @brief Parsing and handling of a list of nodes.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */
#include "custom_util.h"
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
    node_list_t* list = node_list_new();

    if(list != NULL) {
        char** ips = NULL;
        //correcteur: int16_t != uint16_t !
        int16_t* ports = NULL;

        // Parse the IPs and port numbers written into servers.txt
        //correcteur: c'est bien de modulariser mais là c'est un peu compliqué et la liste est parcourue deux fois,
        //correcteur: une 1ère pour prendre les ip et port et une 2ème fois pour ajouter à node_list
        size_t nbr_servers = parse_server_file(&ips, &ports);

        for (size_t i = 0; i < nbr_servers; ++i) {
            node_t node;
            if (node_init(&node, ips[i], ports[i], 0) != ERR_NONE ||
                node_list_add(list, node)) {
                delete_parsed_data(&ips, &ports);
                node_list_free(list);
                return NULL;
            }
        }

        delete_parsed_data(&ips, &ports);
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
