/**
 * @file custom_util.h
 * @brief Custom tools for the project
 *
 * @author Eric Jollès & Robin Mamie
 */
#include <stdint.h>
#include <stdlib.h>


/**
 * @brief performs a deep string copy
 * @param to_copy the string to copy
 * @return the deep string copy
 */
const char* deep_str_copy(const char* to_copy);

/**
 * @brief parses the servers.txt file and saves the ips and ports
 * @param ips the array of strings into which the ips should be saved
 * @param ports the array of integers into which the ports should be saved
 * @return the number of servers
 */
size_t parse_server_file(char*** ips, int16_t** ports);

/**
 * @brief deletes the array of ips and ports created in parse_server_file
 * @param ips the array of strings which should be deleted
 * @param ports the array of ports which should be deleted
 */
void delete_parsed_data(char*** ips, int16_t** ports);

