/**
 * @file custom_util.c
 * @brief Custom tools for the project.
 *
 * //correcteur: pas d'identifiant personnel svp!
 */
#include <stdio.h>
#include <stdlib.h>

#include "custom_util.h"
#include "config.h"
#include "error.h"

//correcteur: pourquoi ne pas utiliser directement strdup (util.h)
const char* deep_str_copy(const char* to_copy)
{
    if (to_copy == NULL) {
        return NULL;
    }
    size_t str_len = strlen(to_copy);
    char* str_copy = calloc(str_len + 1, sizeof(char));
    if (str_copy == NULL) {
        return NULL;
    }
    strncpy(str_copy, to_copy, str_len);
    return str_copy;
}

size_t parse_server_file(char*** ips, int16_t** ports)
{
    if (ips == NULL || ports == NULL) {
        debug_print("%s", "Invalid pointer(s) given to the function.");
    }

    FILE* servers = fopen(PPS_SERVERS_LIST_FILENAME, "r");
    size_t nbr_elements = 0;

    if (servers != NULL) {
        //correcteur: doit êter 15 + 1 pour le \0 (-0.5)
        char ip[15] = PPS_DEFAULT_IP;
        //correcteur: int16_t != uint16_t (-0.5)
        int16_t port = PPS_DEFAULT_PORT;
        while (fscanf(servers, "%15s%hd", ip, &port) == 2) {
            //correcteur: comme port est un int16_t ici, il ne peut pas être plus grand que 65535
            if (port <= 0 || 65535 < port) {
                if (*ips != NULL) {
                    free(*ips);
                }
                if (*ports != NULL) {
                    free(*ports);
                }
                fclose(servers);
                delete_parsed_data(ips, ports);
                //correcteur: attention le type de retour est size_t, pas int
                return -1;
            }
            *ips = realloc(*ips, (nbr_elements + 1) * sizeof(char*));
            (*ips)[nbr_elements] = calloc(15, sizeof(char));
            strcpy((*ips)[nbr_elements], ip);
            *ports = realloc(*ports, (nbr_elements + 1) * sizeof(int16_t));
            (*ports)[nbr_elements] = port;
            ++nbr_elements;
            while (getc(servers) != '\n');
        }
        fclose(servers);
    } else {
        debug_print("Error: unable to read the file %s\n", PPS_SERVERS_LIST_FILENAME);
        //correcteur: attention le type de retour est size_t, pas int
        return -1;
    }

    return nbr_elements;
}

void delete_parsed_data(char*** ips, int16_t** ports)
{
    if (ips != NULL && *ips != NULL) {
        if (**ips != NULL) {
            free(**ips);
        }
        free(*ips);
    }

    if (ports != NULL && *ports != NULL) {
        free(*ports);
    }
}
