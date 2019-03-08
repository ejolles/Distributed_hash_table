/**
 * @file test-servers.c
 * @brief performs tests using network.c
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for fork
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait

#include "client.h"
#include "error.h"
#include "network.h"
#include "node.h"
#include "system.h"

static void print_time(FILE* file,
                       const struct timespec* p_time_start,
                       const struct timespec* p_time_end)
{
    long nsec = p_time_end->tv_nsec - p_time_start->tv_nsec;
    while (nsec < 0) nsec += 1000000000;
    fprintf(file, "%ld%09ld\n", p_time_end->tv_sec - p_time_start->tv_sec, nsec);
}

int main(int argc, char* argv[])
{
    client_t client;
    client_init_args_t args = {&client, 1, TOTAL_SERVERS, (size_t) argc, &argv};
    if (client_init(args) != ERR_NONE) {
        printf("FAIL\n");
        debug_print("%s", "Client init failed.");
        return 1;
    }

    size_t tests_to_perform = 0;
    sscanf(argv[0], "%zu", &tests_to_perform);
    size_t zeros_in_a_row = 0;

    FILE* results = fopen("results.txt", "w");
    if (results == NULL) {
        client_end(&client);
        return 1;
    }

    fprintf(results, "N %zu tests %zu\n\n", client.args->N, tests_to_perform);
    struct timespec time_start, time_end;
    for (client.args->W = 1; client.args->W <= client.args->N; ++client.args->W) {
        fprintf(results, "W %zu\n", client.args->W);
        size_t suc = 0;
        for (size_t i = 0; i < tests_to_perform; ++i) {
            //fprintf(results, "%3zu: ", i);
            clock_gettime(CLOCK_MONOTONIC, &time_start);
            error_code err = network_put(client, (char *) &i, "this is a SourceCode test");
            clock_gettime(CLOCK_MONOTONIC, &time_end);
            if (err == ERR_NONE) {
                ++suc;
                fprintf(results, "SUCCESS ");
            } else {
                fprintf(results, "FAILURE ");
            }
            print_time(results, &time_start, &time_end);
        }
        fprintf(results, "ratio %zu %zu\n\n", suc, tests_to_perform);
        if (suc == 0) {
            ++zeros_in_a_row;
        } else {
            zeros_in_a_row = 0;
        }

        if (zeros_in_a_row == 3) {
            fprintf(results, "Network cannot handle a W this large, aborting.\n");
            break;
        }
        fflush(results);
    }

    fclose(results);
    client_end(&client);

    return 0;
}


