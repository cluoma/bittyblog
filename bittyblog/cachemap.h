//
//  cachemap.h
//  bittyblog
//
//  Created by Colin Luoma on 2018-07-25.
//  Copyright © 2018 Colin Luoma. All rights reserved.
//

#ifndef cachemap_h
#define cachemap_h

#include <time.h>

// Return values
#define BBMAP_OK       0
#define BBMAP_NOT_OK   1
#define BBMAP_FULL     2

// Options for the hash table
//#define CACHE_OVERWRITE 1  // Overwrite on collision

typedef struct bb_map_node {
    char *key;
    char *data;
    unsigned long datalen;
    time_t time;  // Time of node creation
    struct bb_map_node *next;
} bb_map_node;

typedef struct {
    //int options;
    time_t timeout;

    unsigned long long max_bytes;
    unsigned long long bytes;       // Total byte count of all data
    unsigned long size;             // Total number of slots
    unsigned long filled_slots;     // Number of filled slots
    unsigned long node_count;       // Number of nodes, node_count >= count
    bb_map_node **nodes;
} bb_map;

bb_map *bb_map_init(unsigned long size, unsigned long long max_bytes, time_t timeout);
int bb_map_free(bb_map *m);
int bb_map_insert(bb_map **m, char *key, char *data, unsigned long datalen);
int bb_map_remove(bb_map *m, const char *key);
bb_map_node *bb_map_get(bb_map *m, const char *key);

#endif