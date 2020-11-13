#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "char_vector.h"

static size_t initSize = 2;

void initCVector(CVector *cvector) {

    cvector->vector = malloc(initSize * sizeof(char *));

    if (cvector->vector == NULL) {
        // Throw fatal error
        perror("CVector");
        exit(1);
    }

    cvector->vector[0] = NULL;
    cvector->used = 0;
    cvector->size = initSize;
}

static void increase_size(CVector *cvector) {
    cvector->size *= 2;

    cvector->vector = realloc(cvector->vector, 
            cvector->size * sizeof(char *));

    if (cvector->vector == NULL) { 
        // Throw fatal error
        perror("CVector");
        exit(1);
    }

    for (int i = cvector->used; i < cvector->size; i++) {
        cvector->vector[i] = NULL;
    }
}

/* Vector Mode */

void pbCVector(CVector *cvector, char *element) {
    if (cvector->used == cvector->size) {
        increase_size(cvector);
    } 

    cvector->vector[cvector->used++] = element;
}

void freeCVector(CVector *cvector) {
    for (int i = 0; i < cvector->used; i++) {
        free(cvector->vector[i]);
        cvector->vector[i] = NULL;
    }

    free(cvector->vector);
    cvector->used = cvector->size = 0;
}

CVector* to_args(char *str) {
    CVector *argv = malloc(sizeof(CVector));
    if (argv == NULL) {
        // Throw fatal error
        perror("CVector");        
        exit(1);
    }

    initCVector(argv);

    char *save_args;
    char *args = strtok_r(str, " \t\n", &save_args);

    while(args) {
        char *arg = malloc((strlen(args) + 1) * sizeof(char));
        strcpy(arg, args);

        if (arg == NULL) {
            // Throw fatal error
            perror("CVector");
            exit(1);
        }

        pbCVector(argv, arg);

        args = strtok_r(NULL, " \t\n", &save_args);
    }

    return argv;
}
