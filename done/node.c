/**
 * @file node.c
 * @brief Node implementation.
 */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#include "node.h"


error_code node_init(node_t *node, const char *ip, uint16_t port, size_t _unused node_id)
{
    M_REQUIRE_NON_NULL(node);
    M_REQUIRE_NON_NULL(ip);
#define MAX_LINE_SIZE 100
    char input[MAX_LINE_SIZE];
    memset(input, 0, MAX_LINE_SIZE);
    sprintf(input, "%s %hu %zu", ip, port, node_id);

    unsigned char* sha = calloc(SHA_DIGEST_LENGTH, sizeof(unsigned char));
    M_EXIT_IF_NULL(sha, SHA_DIGEST_LENGTH, "node.c");
    SHA1((unsigned char*) input, strlen(input), sha);
    node->sha = sha;
    node->address = malloc(sizeof(struct sockaddr_in));
    if (node->address == NULL) {
        node_end(node);
        return ERR_NOMEM;
    }

    error_code err = get_server_addr(ip, port, node->address);

    if (err != ERR_NONE) {
        node_end(node);
        return err;
    }

    return ERR_NONE;
}

void node_end(node_t* node)
{
    free_const_ptr(node->sha);
    free(node->address);
}

int node_cmp_sha(const node_t *first, const node_t *second)
{
    M_REQUIRE_NON_NULL(first);
    M_REQUIRE_NON_NULL(second);
    return strncmp((const char*) first->sha, (const char*) second->sha, SHA_DIGEST_LENGTH);
}

int node_cmp_server_addr(const node_t *first, const node_t *second)
{
    M_REQUIRE_NON_NULL(first);
    M_REQUIRE_NON_NULL(second);
    int diff_ip = memcmp(&first->address->sin_addr, &second->address->sin_addr, sizeof(struct in_addr));
    return diff_ip ? diff_ip : memcmp(&first->address->sin_port, &second->address->sin_port, sizeof(in_port_t));
}
