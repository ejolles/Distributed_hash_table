/**
 * @file args.c
 * @brief Parsing argv options. This is required only from week 10.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "args.h"
#include "error.h"

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

#define BASE_N 3
#define BASE_W 2
#define BASE_R 2


/**
 * @brief extracts the number (if it exists) associated to the first element of a list of string
 * @param str the list of string, the first element should be either -n,-r or -w and then a positive number
 * @param num the number associated to the first element of the list of string
 * @return an error code
 */
error_code go_fwd_and_parse_number(char*** str, size_t* num)
{
    ++(*str);
    if (**str == NULL) {
        return ERR_BAD_PARAMETER;
    }
    int num_test = 0;
    int nb = sscanf(**str, "%d", &num_test);
    if (nb != 1 || num_test <= 0) {
        return ERR_BAD_PARAMETER;
    }
    *num = (size_t) num_test;
    ++(*str);
    return ERR_NONE;
}

args_t *parse_opt_args(size_t supported_args, char ***rem_argv)
{
    args_t* args = malloc(sizeof(args_t));
    if (args == NULL) {
        return NULL;
    }
    args->N = BASE_N;
    args->W = BASE_W;
    args->R = BASE_R;

    // Indicates whether the user has modified some values
    int N_mod = 0;
    int W_mod = 0;
    int R_mod = 0;

    while (supported_args && **rem_argv != NULL) {
        // num will point to the required argument
        size_t* num = NULL;
        int* mod = NULL;
        if (!strcmp(*rem_argv[0], "-n")
            && (supported_args & TOTAL_SERVERS)) {
            num = &args->N;
            mod = &N_mod;
        } else if (!strcmp(*rem_argv[0], "-r")
                   && (supported_args & GET_NEEDED)) {
            num = &args->R;
            mod = &R_mod;
        } else if (!strcmp(*rem_argv[0], "-w")
                   && (supported_args & PUT_NEEDED)) {
            num = &args->W;
            mod = &W_mod;
        } else {
            if (!strcmp(*rem_argv[0], "--")) {
                ++(*rem_argv);
            }
            break;
        }
        // Extract the value and save it to where num points
        // Tests if the value is positive. If it is zero or negative,
        // returns an error code.
        if (num != NULL
            && go_fwd_and_parse_number(rem_argv, num) == ERR_NONE
            && mod != NULL
            && *mod == 0) {
            ++(*mod);
        } else {
            free(args);
            return NULL;
        }
    }

    // Handle incoherences
    if (N_mod) {
        if (W_mod && args->W > args->N) {
            free(args);
            return NULL;
        } else {
            args->W = min(args->N, args->W);
        }
        if (R_mod && args->R > args->N) {
            free(args);
            return NULL;
        } else {
            args->R = min(args->N, args->R);
        }
    } else {
        if (W_mod && R_mod) {
            args->N = max(args->N, max(args->R, args->W));
        } else if (W_mod) {
            args->N = max(args->N, args->W);
        } else if (R_mod) {
            args->N = max(args->N, args->R);
        }
    }

    return args;
}
