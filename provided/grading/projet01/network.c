/**
 * @file network.c
 * @brief Network implementation.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */
#include <string.h>

#include "custom_util.h"
#include "error.h"
#include "network.h"


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
 * @brief receive message from server
 * @param socket the client socket
 * @param buffer the message
 * @param length the length of the message
 * @return the size of the packet effectively received, -1 if error
 */
ssize_t recvfrom_server(int socket, void *restrict buffer, size_t length)
{
    //correcteur: ici il faudrait plutôt utiliser recvfrom et tester après l'appel la valeur de 
    //correcteur: src_addr et addrlen afin de s'assurer que le message vienne du bon endroit et soit complet (-1)
    return recv(socket, buffer, length, 0);
}



error_code network_get(client_t client, pps_key_t key, pps_value_t *value)
{
    M_EXIT_IF_TOO_LONG(key, MAX_MSG_SIZE, "network.c");
    char in_msg[MAX_MSG_SIZE];
    memset(in_msg, 0, sizeof(char) * MAX_MSG_SIZE);
    size_t in_msg_len = 0;

    if (*value != NULL) {
        free_const_ptr(*value);
        *value = NULL;
    }

    for (size_t i = 0; i < client.nodes->size; ++i) {
        //correcteur: il faut aussi tester que sendto_server == strlen(key) (-0.5)
        if (sendto_server(client.socket, client.nodes->nodes[i], key,
                          strlen(key)) == -1) {
            debug_print("%s", "Could not send to server.");
            //correcteur: recvfrom_server retourne un ssize_t
            //correcteur: il faut tester que in_msg_len == 1 avec in_msg[0] == '\0' (-0.5)
        } else if ((in_msg_len = recvfrom_server(client.socket, in_msg,
                                 MAX_MSG_SIZE)) == -1 || in_msg[0] == '\0') {
            debug_print("%s", "Did not get an answer or a correct answer from the server.");
        } else {
            *value = deep_str_copy(in_msg);
            M_EXIT_IF_NULL(*value, (int)strlen(in_msg), "network.c");
            return ERR_NONE;
        }
    }
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
        strncpy(packet, key, size_key);
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
    size_t size = 0;
    char* msg = create_cat_packet(key, value, &size);

    M_EXIT_IF_NULL(msg, (int)size, "network.c");

    // Send message to all servers
    // NB: put_error could be useful in the future to count the number of
    // failed communications
    unsigned int put_error = 0;
    unsigned int response = 0;
    for (size_t i = 0; i < client.nodes->size; ++i) {
        //correcteur: il faut aussi tester que sendto_server == strlen(key) (-0.5)
        if (sendto_server(client.socket, client.nodes->nodes[i], msg,
                          size) == -1) {
            debug_print("%s", "Could not send to server.");
            ++put_error;
            //correcteur: il faudrait plutôt utiliser NULL et 0 comme 2ème et 3ème paramètres de recvfrom_server
            //correcteur: il faut aussi tester que recvfrom_src() == 0 (-0.5)
        } else if (recvfrom_server(client.socket, &response,
                                   sizeof(response)) == -1) {
            debug_print("%s", "Did not get an answer from the server.");
            ++put_error;
        }
    }
    delete_cat_packet(msg);
    msg = NULL;
    size = 0;

    M_EXIT_IF(put_error > 0, ERR_NETWORK, "network.c", "%s", ", did not reach all servers.");

    return ERR_NONE;
}

