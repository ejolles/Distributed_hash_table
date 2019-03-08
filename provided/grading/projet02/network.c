/**
 * @file network.c
 * @brief Network implementation.
 */
#include <string.h>
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
    return sendto(socket, msg, length, 0, (struct sockaddr*)&node,
                  sizeof(node));
}

/**
 * @brief checks if the two given sockaddr are the same
 * @param a first sockaddr
 * @param b second sockaddr
 * @param len_a first socklen
 * @param len_b second socklen
 * @return 1 if they are the same server, 0 if not
 */
int same_sockaddr(struct sockaddr_in* a, struct sockaddr_in* b, socklen_t len_a, socklen_t len_b)
{
    //correcteur: on peut utiliser memcmp(a,b) et len_a == len_b
    char a_decr[len_a];
    inet_ntop(AF_INET, a, a_decr, len_a);
    char b_decr[len_b];
    inet_ntop(AF_INET, b, b_decr, len_b);

    if (len_a != len_b
        || a->sin_port != b->sin_port
        || strncmp(a_decr, b_decr, len_a) != 0) {
        return 0;
    }

    return 1;
}

/**
 * @brief receive message from server
 * @param socket the client socket
 * @param buffer the message
 * @param length the length of the message
 * @return the size of the packet effectively received, -1 if error
 */
ssize_t recvfrom_server(int socket, void *restrict buffer, size_t length, node_t node)
{
    struct sockaddr_in* node_addr_ptr = (struct sockaddr_in*)&node;
    socklen_t node_len = sizeof(node);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    socklen_t addr_len = sizeof(serv_addr);

    ssize_t size = recvfrom(socket, buffer, length, 0, (struct sockaddr*)&serv_addr, &addr_len);

    if (size != -1 && same_sockaddr(node_addr_ptr, &serv_addr, node_len, addr_len)) {
        return size;
    }

    return -1;
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
        return count;
    }
    add_Htable_value(table, key, "\x01");
    return 1;
}

error_code network_get(client_t client, pps_key_t key, pps_value_t *value)
{
    M_EXIT_IF_TOO_LONG(key, MAX_MSG_SIZE, "network.c");
    char in_msg[MAX_MSG_SIZE];
    ssize_t in_msg_len = 0;

    if (*value != NULL) {
        free_const_ptr(*value);
        *value = NULL;
    }
    // Local Htable where the keys are the received values and the values are the linked counters
    //correcteur: on peut utiliser client.args->N comme taille pour la Htable
    Htable_t table = construct_Htable(HTABLE_SIZE);

    // for-loop only iterates on N nodes
    for (size_t i = 0; i < client.args->N; ++i) {
        memset(in_msg, 0, sizeof(char) * MAX_MSG_SIZE);
        ssize_t size = 0;
        if ((size = sendto_server(client.socket, client.nodes->nodes[i], key,
                                  strlen(key))) == -1
            || (size_t) size != strlen(key)) {
            debug_print("%s", "Could not send to server.");
        } else if ((in_msg_len = recvfrom_server(client.socket, in_msg,
                                 MAX_MSG_SIZE, client.nodes->nodes[i])) == -1 || (in_msg[0] == '\0' && in_msg_len == 1)) {
            debug_print("%s", "Did not get an answer or a correct answer from the server.");
        } else {
            // Count is the number of occurrences of the key
            unsigned int count = increment_and_get(table, in_msg);
            if (count >= client.args->R) {
                delete_Htable_and_content(&table);
                *value = strdup(in_msg);
                M_EXIT_IF_NULL(*value, (int)strlen(in_msg), "network.c");
                return ERR_NONE;
            }
        }
    }

    delete_Htable_and_content(&table);
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

/**
 * @brief deletes the created packet
 * @param packet the packet to delete
 */
void delete_cat_packet(char* packet)
{
    free(packet);
}

error_code network_put(client_t client, pps_key_t key, pps_value_t value)
{
    M_EXIT_IF_TOO_LONG(key, MAX_MSG_SIZE, "network.c");
    M_EXIT_IF_TOO_LONG(value, MAX_MSG_SIZE, "network.c");
    // Create packet
    size_t nodes_contacted = 0;
    size_t size = 0;
    char* msg = create_cat_packet(key, value, &size);

    M_EXIT_IF_NULL(msg, (int)size, "network.c");

    // Send message to all servers
    // for-loop only iterates on N nodes
    for (size_t i = 0; i < client.args->N; ++i) {
        ssize_t size_rec = 0;
        if ((size_rec = sendto_server(client.socket, client.nodes->nodes[i], msg,
                                      size)) == -1
            || (size_t) size_rec != size) {
            debug_print("%s", "Could not send to server.");
        } else if (recvfrom_server(client.socket, NULL, 0, client.nodes->nodes[i]) != 0) {
            debug_print("%s", "Did not get an answer from the server.");
        } else {
            ++nodes_contacted;
            //correcteur: il faut quitter si le quorum est atteint (-1) sinon on envoie trop de put
        }
    }
    delete_cat_packet(msg);
    msg = NULL;

    M_EXIT_IF(nodes_contacted < client.args->W, ERR_NETWORK, "network.c", "%s", ", did not reach all servers.");

    return ERR_NONE;
}

