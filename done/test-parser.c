/**
 * @file test-parser.c
 * @brief parses the results from test-servers
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "client.h"
#include "error.h"
#include "network.h"
#include "node.h"
#include "system.h"

int main(void)
{
    FILE* results = fopen("results.txt", "r");
    if (results == NULL) {
        printf("Failed to read file.\n");
        return 1;
    }
    FILE* parsed = fopen("parsed.txt", "w");
    if (parsed == NULL) {
        printf("Failed to write file.\n");
        fclose(results);
        return 1;
    }

    size_t N = 0;
    size_t tests = 0;
    size_t zeros_in_a_row = 0;
    fscanf(results, "%*c%zu%*s%zu", &N, &tests);
    while (getc(results) != '\n');
    while (getc(results) != '\n');

    double matrix[N][6];
    memset(matrix, 0, N*6*sizeof(size_t));
    size_t W = 0;

    for (W = 0; W < N && zeros_in_a_row < 3; ++W) {
        while (getc(results) != '\n');
        double s_times[tests];
        memset(s_times, 0, sizeof(size_t) * tests);
        size_t s_idx = 0;

        double f_times[tests];
        memset(f_times, 0, sizeof(size_t) * tests);
        size_t f_idx = 0;
        char status[8];

        for (size_t i = 0; i < tests; ++i) {
            fscanf(results, "%s", status);
            if (!strcmp(status, "SUCCESS")) {
                fscanf(results, "%lf", &s_times[s_idx]);
                matrix[W][1] += s_times[s_idx];
                matrix[W][2] += s_times[s_idx] * s_times[s_idx];
                ++s_idx;
            } else if (!strcmp(status, "FAILURE")) {
                fscanf(results, "%lf", &f_times[f_idx]);
                matrix[W][4] += f_times[f_idx];
                matrix[W][5] += f_times[f_idx] * f_times[f_idx];
                ++f_idx;
            }
        }


        fscanf(results, "%*s%*zu%*zu");
        if (s_idx != 0) {
            matrix[W][0] = (double) s_idx;
            matrix[W][1] /= (double) s_idx;
            matrix[W][2] /= (double) s_idx;
            matrix[W][2] = (s_idx == 1) ? 0 : sqrt( ((double) s_idx / ((double) s_idx - 1.0))
                                                    * (matrix[W][2] - matrix[W][1] * matrix[W][1]));
            matrix[W][1] /= 1e6;
            matrix[W][2] /= 1e6;
        }

        if (f_idx != 0) {
            matrix[W][3] = (double) f_idx;
            matrix[W][4] /= (double) f_idx;
            matrix[W][5] /= (double) f_idx;
            matrix[W][5] =  (f_idx == 1) ? 0 : sqrt( ((double) f_idx / ((double) f_idx - 1.0))
                            * (matrix[W][5] - matrix[W][4] * matrix[W][4]));
            matrix[W][4] /= 1e6;
            matrix[W][5] /= 1e6;
        }

        if (s_idx == 0) {
            ++zeros_in_a_row;
        } else {
            zeros_in_a_row = 0;
        }
        while (getc(results) != '\n');
        while (getc(results) != '\n');
    }

    fprintf(parsed, "SUCCESSES\n");
    for (size_t i = 0; i < W; ++i) {
        fprintf(parsed, "%3zu, %3.0lf, %6.1lf, %6.1lf\n", i+1, matrix[i][0], matrix[i][1], matrix[i][2]);
    }

    fprintf(parsed, "\n");

    fprintf(parsed, "FAILURES\n");
    for (size_t i = 0; i < W; ++i) {
        fprintf(parsed, "%3zu, %3.0lf, %6.1lf, %6.1lf\n", i+1, matrix[i][3], matrix[i][4], matrix[i][5]);
    }

    fclose(results);
    fclose(parsed);

    return 0;
}


