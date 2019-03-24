//
//  vec.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vec.h"

void bb_vec_init(bb_vec* vec, void* f)
{
    vec->size = 0;
    vec->count = 0;
    vec->data = NULL;
    vec->f = f;
}

int bb_vec_count(bb_vec* vec)
{
    return(vec->count);
}

void bb_vec_add(bb_vec* vec, void* data)
{
    if(vec->count == 0)
    {
        vec->data = calloc(10, sizeof(void*));
        vec->size = 10;
    }

    if(vec->count == vec->size)
    {
        vec->data = realloc(vec->data, vec->size * sizeof(void*) * 2);
        vec->size = vec->size * 2;
    }

    vec->data[vec->count] = data;
    vec->count++;
}

void* bb_vec_get(bb_vec* vec, int i)
{
    return(vec->data[i]);
}

void bb_vec_free(bb_vec* vec)
{
    // Free contents
    for (int i = 0; i < vec->count; i++)
    {
        if (vec->f == NULL)
        {
            free(vec->data[i]);
        } else {
            vec->f(vec->data[i]);
        }
    }

    // Free array
    if (vec->data != NULL) free(vec->data);

    // Free itself
    free(vec);
}