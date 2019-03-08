/**
 * @file ring.c
 * @brief Ring parsing and handling. This is required only from week 11.
 */

#include <stddef.h>
#include <stdint.h>

#include "ring.h"
#include "error.h"
#include "hashtable.h"
#include "node_list.h"

ring_t *ring_alloc()
{
    return node_list_new();
}

error_code ring_init(ring_t *ring)
{
#define IP_LENGTH 15
    if(ring != NULL) {
        node_list_t* list = get_nodes();
        if (list == NULL) {
            return ERR_NOMEM;
        }
        node_list_sort(list, node_cmp_sha);
        *ring = *list;
        free(list);
    }
    return ERR_NONE;
}

void ring_free(ring_t *ring)
{
    node_list_free(ring);
}

/**
 * @brief returns the index of the nearest node
 * @param ring the ring of nodes
 * @param sha the SHA1 string of the key
 * @return the described index
 */
size_t index_of_first_node(const ring_t* ring, unsigned char sha[SHA_DIGEST_LENGTH])
{
    for (size_t i = 0; i < ring->size; ++i) {
        if (strncmp((const char*) ring->nodes[i].sha, (const char*) sha, SHA_DIGEST_LENGTH) >= 0) {
            return i;
        }
    }
    return 0;
}

/**
 * @brief checks whether a node is already present in a node list
 * @param list the list of nodes
 * @param node the node
 * @return 1 if the node is already present, 0 otherwise
 */
int already_present_in_list(node_list_t* list, node_t node)
{
    for (size_t i = 0; i < list->size; ++i) {
        if (!node_cmp_server_addr(&list->nodes[i], &node)) {
            return 1;
        }
    }
    return 0;
}

node_list_t *ring_get_nodes_for_key(const ring_t *ring, size_t wanted_list_size, pps_key_t key)
{
    unsigned char key_sha[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*) key, strlen(key), key_sha);
    size_t index = index_of_first_node(ring, key_sha);
    node_list_t* list = node_list_new();
    if (list == NULL) {
        return NULL;
    }
    while (list->size < wanted_list_size) {
        node_t node = ring->nodes[index];
        if (!already_present_in_list(list, node)) {
            node_list_add(list, node);
        }
        index = (index + 1) % ring->size;
    }
    return list;
}
