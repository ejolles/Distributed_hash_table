/**
 * @file network.c
 * @brief Network implementation.
 */
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "error.h"
#include "hashtable.h"
#include "network.h"
#include "util.h"


/**
 * @brief send message to server
 * @param socket the client socket
 * @param node the node to which the message sould be sent
 * @param msg the message to send
 * @param length the length of the message
 * @return the size of the packet effectively sent, -1 if error
 */
ssize_t sendto_server(int socket, node_t node, const void* msg, size_t length)
{
    return sendto(socket, msg, length, 0, (struct sockaddr*)node.address,
                  sizeof(*node.address));
}

/**
 * @brief receive message from a specific server
 * @param socket the client socket
 * @param buffer the message
 * @param length the length of the message
 * @return the size of the packet effectively received, -1 if error
 */
ssize_t recvfrom_server(int socket, void *restrict buffer, size_t length)
{
    return recv(socket, buffer, length, 0);
}

/**
 * @brief using a local Htable, counts the number of occurrences of a specified key and returns the former
 * @param table the local hash table
 * @param key the key to add and count
 * @return the number of occurrences of the key
 */
unsigned int increment_and_get(Htable_t table, pps_key_t key)
{
    char* value = (char*) get_Htable_value(table, key);
    if (value != NULL) {
        unsigned int count = (unsigned int) (++value[0]);
        add_Htable_value(table, key, value);
        free(value);
        return count;
    }
    add_Htable_value(table, key, "\x01");
    free(value);
    return 1;
}

error_code create_socket(int* socket)
{
#define CLIENT_TIMEOUT 1
    *socket = get_socket(CLIENT_TIMEOUT);
    if (*socket == -1) {
        debug_print("%s", "Could not create socket.");
        return ERR_NETWORK;
    }
    debug_print("%s", "Socket creation successful.");
    return ERR_NONE;
}

error_code network_get(client_t client, pps_key_t key, pps_value_t *value)
{
    M_REQUIRE_NON_NULL(key);
    M_REQUIRE_NON_NULL(value);
    M_EXIT_IF_TOO_LONG(key, MAX_MSG_SIZE, "network.c");

    if (*value != NULL) {
        free_const_ptr(*value);
        *value = NULL;
    }

    int socket = 0;
    if (create_socket(&socket) != ERR_NONE) {
        return ERR_NETWORK;
    }

    // Local Htable where the keys are the received values and the values are the linked counters
    Htable_t table = construct_Htable(client.args->N);
    if (table.buckets == NULL) {
        close(socket);
        return ERR_NOMEM;
    }

    node_list_t* nodes = ring_get_nodes_for_key(client.nodes, client.args->N, key);
    if (nodes == NULL) {
        delete_Htable_and_content(&table);
        close(socket);
        return ERR_NOMEM;
    }

    for (size_t i = 0; i < nodes->size; ++i) {
        sendto_server(socket, nodes->nodes[i], key, strlen(key));
    }

    for (size_t i = 0; i < nodes->size; ++i) {
        char in_msg[MAX_MSG_SIZE];
        ssize_t in_msg_len = 0;
        memset(in_msg, 0, sizeof(char) * MAX_MSG_SIZE);
        if ((in_msg_len = recvfrom_server(socket, in_msg, MAX_MSG_SIZE)) != -1
            && !(in_msg[0] == 0 && in_msg_len == 1)) {
            unsigned int count = increment_and_get(table, in_msg);
            if (count >= client.args->R) {
                delete_Htable_and_content(&table);
                ring_free(nodes);
                *value = strdup(in_msg);
                M_EXIT_IF_NULL(*value, (int)strlen(in_msg), "network.c");
                close(socket);
                return ERR_NONE;
            }
        } else {
            debug_print("%s", "Could not contact servers.");
            //correcteur: il faut faire continue pas break, sinon si dès qu'un serveur répond qu'il n'a pas la clef
            //correcteur: cela retourne FAIL (-1)
            break;
        }
    }

    ring_free(nodes);
    delete_Htable_and_content(&table);
    close(socket);
    return ERR_NETWORK;
}

/**
 * @brief creates network packet with key and value
 * @param key the key
 * @param val the value
 * @param size the variable into which the size of the packet should be written
 * @return the packet
 */
char* create_cat_packet(pps_key_t key, pps_value_t val, size_t* size)
{
    size_t size_key = strlen(key);
    size_t size_val = strlen(val);
    *size = size_key + 1 + size_val;
    char* packet = calloc(*size, sizeof(char));
    if (packet != NULL) {
        strcpy(packet, key);
        strncpy(&packet[size_key + 1], val, size_val);
    }
    return packet;
}

error_code network_put(client_t client, pps_key_t key, pps_value_t value)
{
    M_REQUIRE_NON_NULL(key);
    M_REQUIRE_NON_NULL(value);
    M_EXIT_IF_TOO_LONG(key, MAX_MSG_SIZE, "network.c");
    M_EXIT_IF_TOO_LONG(value, MAX_MSG_SIZE, "network.c");

    int socket = 0;
    M_EXIT_IF_ERR(create_socket(&socket), "socket creation failed");

    // Create packet
    size_t size = 0;
    char* msg = create_cat_packet(key, value, &size);

    if (msg == NULL) {
        close(socket);
        return ERR_NOMEM;
    }

    // Send message to all servers
    // for-loop only iterates on N nodes
    node_list_t* nodes = ring_get_nodes_for_key(client.nodes, client.args->N, key);
    if (nodes == NULL) {
        free(msg);
        close(socket);
        return ERR_NOMEM;
    }

    for (size_t i = 0; i < nodes->size; ++i) {
        if (sendto_server(socket, nodes->nodes[i], msg, size) != -1) {
            debug_print("%s", "Successfuly sent msg.");
        }
    }

    size_t nodes_contacted = 0;
    for (size_t i = 0; i < nodes->size && recvfrom_server(socket, NULL, 0) == 0; ++i) {
        ++nodes_contacted;
        debug_print("Successfuly contacted server, #%zu success.", nodes_contacted);
        if (nodes_contacted >= client.args->W) {
            debug_print("Enough servers contacted: %zu", nodes_contacted);
            break;
        }
    }

    free(msg);
    node_list_free(nodes);
    close(socket);

    M_EXIT_IF(nodes_contacted < client.args->W, ERR_NETWORK, "network.c", ", W quorum %zu not reached (only contacted %zu servers), put failed.", client.args->W, nodes_contacted);

    return ERR_NONE;
}

