/*
 * @file pps-list-nodes.c
 * @brief Tests all nodes present in servers.txt
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"
#include "config.h"
#include "error.h"
#include "node.h"
#include "system.h"

/**
 * @brief checks whether a node is present in a node_list or not
 * @param list the node_list
 * @param node the node
 *
 * @return 1 if the node is present in the list, 0 if not
 */
int is_node_present_in_list(node_list_t* list, node_t node)
{
    for (size_t i = 0; i < list->size; ++i) {
        if (!node_cmp_server_addr(&list->nodes[i], &node)) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
#define CLIENT_TIMEOUT 1
    int socket = get_socket(CLIENT_TIMEOUT);
    if (socket == -1) {
        debug_print("%s", "Could not get socket.");
        printf("FAIL\n");
        return 1;
    }

    node_list_t* base_nodes = get_nodes();
    node_list_sort(base_nodes, node_cmp_server_addr);

    for (size_t i = 0; i < base_nodes->size; ++i) {
        node_t node = base_nodes->nodes[i];
        sendto(socket, NULL, 0, 0, (struct sockaddr*) node.address, sizeof(*node.address));
    }

    node_list_t* list = node_list_new();
    node_t node;
    if (node_init(&node, PPS_DEFAULT_IP, PPS_DEFAULT_PORT, 0) != ERR_NONE) {
        node_list_free(list);
        node_list_free(base_nodes);
    }
    socklen_t len = sizeof(struct sockaddr_in);
    while (recvfrom(socket, NULL, 0, 0, (struct sockaddr *) node.address, &len)
           == 0) {
        if (node_list_add(list, node) != ERR_NONE) {
            node_list_free(list);
            node_list_free(base_nodes);
            return 1;
        }
    }
    node_end(&node);
    memset(&node, 0, sizeof(node_t));
    node_list_sort(list, node_cmp_server_addr);

    for (size_t i = 0; i < base_nodes->size; ++i) {
        node = base_nodes->nodes[i];
        char ip[15+1];
        inet_ntop(AF_INET, &node.address->sin_addr, ip, len);
        uint16_t port = ntohs(node.address->sin_port);

        printf("%s %hu (", ip, port);
        for (size_t j = 0; j < SHA_DIGEST_LENGTH; ++j) {
            printf("%02x", node.sha[j]);
        }
        printf(") ");
        if (is_node_present_in_list(list, node)) {
            printf("OK");
        } else {
            printf("FAIL");
        }
        putchar('\n');
    }
    node_list_free(list);
    node_list_free(base_nodes);
    return 0;
}
