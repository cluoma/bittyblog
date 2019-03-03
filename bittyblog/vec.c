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

void bb_vec_init(bb_vec* vec)
{
    vec->size = 0;
    vec->count = 0;
    vec->data = NULL;
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
    free(vec->data);
}