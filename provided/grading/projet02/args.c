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
    //correcteur: num étant un size_t, si un valeur négative est passée, elle sera castée en un grand nombre positif (-0.5)
    int nb = sscanf(**str, "%lu", num);
    if (nb != 1) {
        return ERR_BAD_PARAMETER;
    }
    ++(*str);
    return ERR_NONE;
}

args_t *parse_opt_args(size_t supported_args, char ***rem_argv)
{
    args_t* args = malloc(sizeof(args_t));
    //correcteur: il faut tester que args != NULL (-0.5)
    args->N = BASE_N;
    args->W = BASE_W;
    args->R = BASE_R;


    while (supported_args && **rem_argv != NULL) {
        // num will point to the required argument
        size_t* num = NULL;
        if (!strcmp(*rem_argv[0], "-n")
            && (supported_args & TOTAL_SERVERS)) {
            num = &args->N;
        } else if (!strcmp(*rem_argv[0], "-r")
                   && (supported_args & GET_NEEDED)) {
            num = &args->R;
        } else if (!strcmp(*rem_argv[0], "-w")
                   && (supported_args & PUT_NEEDED)) {
            num = &args->W;
        } else {
            if (!strcmp(*rem_argv[0], "--")) {
                ++(*rem_argv);
            }
            break;
        }
        // Extract the value and save it to where num points
        if (num != NULL
            && go_fwd_and_parse_number(rem_argv, num) != ERR_NONE) {
            free(args);
            return NULL;
        }
    }
    //correcteur: il faut tester que N,W et R soient différent de 0 (-0.5)
    //correcteur: il faut tester que N >= W et R (-0.5)

    return args;
}
