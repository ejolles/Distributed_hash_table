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
            size_t node_id = 0;
            //correcteur: si fscanf != 3 et != EOF c'est qu'il y a eu une erreur, il faut retourner une erreur (-0.5)
            while (fscanf(servers, "%15s %hu %lu", ip, &port, &node_id) == 3) {
                node_t node;
                memset(&node, 0, sizeof(node));
                for (size_t i = 1; i <= node_id; ++i) {
                    if (node_init(&node, ip, port, i) != ERR_NONE ||
                        node_list_add(list, node) != ERR_NONE) {
                        node_list_free(list);
                        node_end(&node);
                        fclose(servers);
                        return NULL;
                    }
                    node_end(&node);
                }
            }
            fclose(servers);
        }
    }
    return list;
}

error_code node_list_add(node_list_t *list, node_t node)
{
    M_EXIT_IF(list == NULL, ERR_BAD_PARAMETER, "node_list.c", "%s", ", node_list not initialised.");

    node_t* old_nodes = list->nodes;

    if((list->size + 1) > SIZE_MAX / sizeof(node_t) ||
       ((list->nodes = realloc(list->nodes,
                               (list->size + 1) * sizeof(node_t))) == NULL)) {
        list->nodes = old_nodes;
        return ERR_NOMEM;
    }

    node_init(&list->nodes[list->size], PPS_DEFAULT_IP, PPS_DEFAULT_PORT, 0);
    memcpy(list->nodes[list->size].address, node.address, sizeof(struct sockaddr_in));
    free_const_ptr(list->nodes[list->size].sha);

    unsigned char* temp = malloc(SHA_DIGEST_LENGTH);
    if (temp == NULL) {
        node_end(&list->nodes[list->size]);
        return ERR_NOMEM;
    }
    memcpy(temp, node.sha, SHA_DIGEST_LENGTH);
    list->nodes[list->size].sha = temp;
    list->size += 1;
    return ERR_NONE;
}

void node_list_sort(node_list_t *list, int (*comparator)(const node_t *, const node_t *))
{
    qsort(list->nodes, list->size, sizeof(node_t), (int (*)(const void*, const void*)) comparator);
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
