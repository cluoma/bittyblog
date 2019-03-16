//
//  vec.h
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#ifndef vec_h
#define vec_h

// Basic vector, it's up to the user to free contents
typedef struct {
    int size;
    int count;
    void** data;
    void (*f) (void *d);
} bb_vec;

void bb_vec_init(bb_vec*, void *);
void bb_vec_free(bb_vec* );

int bb_vec_count(bb_vec*);
void bb_vec_add(bb_vec*, void*);
void* bb_vec_get(bb_vec*, int);

#endif /* vec_h */