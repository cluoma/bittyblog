//
//  cachemap.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cachemap.h"

#define OK      0
#define NOT_OK   1

/*
 * Helper Functions
 */
char *strcopy(const char *src, size_t len)
{
    if (len == 0 || src == NULL) return NULL;

    char *dest = malloc(len + 1);
    if (dest == NULL) return NULL;

    memmove((void *)dest, (void *)src, len);
    dest[len] = '\0';

    return dest;
}

unsigned long djb2_hash(unsigned char *str) {
    // djb2 hash function, taken from http://www.cse.yorku.ca/~oz/hash.html
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/*
 * Node Functions
 */
bb_map_node *bb_map_node_init()
{
    bb_map_node *n = malloc(sizeof(bb_map_node));
    if (n == NULL) return NULL;
    
    n->key      = NULL;
    n->data     = NULL;
    n->datalen  = 0;
    n->time     = time(NULL);
    n->next     = NULL;

    return n;
}
int bb_map_node_clear(bb_map_node *n)
{
    if (n == NULL) return OK;

    if (n->key != NULL) free(n->key);
    if (n->data != NULL) free(n->data);
    free(n);

    return OK;
}
int bb_map_node_set_key(bb_map_node *n, const char* key)
{   
    if (n->key != NULL) free(n->key);

    n->key = strcopy(key, strlen(key));
    if (n->key == NULL)
        return NOT_OK;
    
    return OK;
}
int bb_map_node_set_data(bb_map_node *n, const char* data, unsigned long datalen)
{
    if (n->data != NULL) free(n->data);

    n->data = strcopy(data, datalen);
    if (n->data == NULL)
    {
        free(n->key);
        return NOT_OK;
    }
    n->datalen = datalen;
    n->time = time(NULL);

    return OK;
}
int bb_map_node_set(bb_map_node *n, const char* key, const char* data, unsigned long datalen)
{
    
    if ( bb_map_node_set_key(n, key) != OK ||
         bb_map_node_set_data(n, data, datalen) != OK )
    {
        return NOT_OK;
    }

    return OK;
}

/*
 * Hash Map Functions
 */
bb_map *bb_map_init(unsigned long size, unsigned long long max_bytes, int options)
{
    bb_map *m = malloc(sizeof(bb_map));
    if (m == NULL) return NULL;

    m->options      = options;

    m->max_bytes    = max_bytes;
    m->size         = size;
    m->bytes        = 0;
    m->filled_slots = 0;
    m->node_count   = 0;
    m->nodes        = malloc(sizeof(bb_map_node *) * size);
    for (unsigned long i = 0; i < size; i++)
        m->nodes[i] = NULL;

    return m;
}
int bb_map_purge(bb_map *m)
{
    for (unsigned long i = 0; i < m->size; i++)
    {
        bb_map_node *n = m->nodes[i];
        bb_map_node *next;
        while (n != NULL)
        {
            next = n->next;
            bb_map_node_clear(n);
            n = next;
        }
    }

    return OK;
}
int bb_map_free(bb_map *m)
{
    bb_map_purge(m);
    free(m->nodes);
    free(m);

    return 0;
}
int bb_map_resize(bb_map **m)
{
    bb_map *new = bb_map_init((*m)->size * 2, (*m)->max_bytes, (*m)->options);

    for (unsigned long i = 0; i < (*m)->size; i++)
    {
        bb_map_node *n = (*m)->nodes[i];
        bb_map_node *next;
        while (n != NULL)
        {
            bb_map_insert(&new, n->key, n->data, n->datalen);
            next = n->next;
            bb_map_remove((*m), n->key);
            n = next;
        }
    }
    bb_map_free(*m);
    *m = new;

    return OK;
}

int bb_map_insert(bb_map **m, char *key, char *data, unsigned long datalen)
{
    // Check if we have enough data to add
    if ((*m)->bytes + datalen + 1 > (*m)->max_bytes)
        return NOT_OK;
    
    // Expand hash table if we need to
    if ((float)(*m)->filled_slots/(float)(*m)->size >= 0.7)
    {
        bb_map_resize(m);
    }

    unsigned long l = djb2_hash((unsigned char*)key) % (*m)->size;

    bb_map_node *cn = (*m)->nodes[l];
    if ( cn == NULL )                       // Empty linked list
    {
        bb_map_node *n = bb_map_node_init();
        if (bb_map_node_set(n, key, data, datalen) != OK)
        {
            bb_map_node_clear(n);
            return NOT_OK;
        }

        (*m)->bytes         += datalen+1;
        (*m)->filled_slots  += 1;
        (*m)->node_count    += 1;

        (*m)->nodes[l] = n;
    }
    else if ((*m)->options & CACHE_OVERWRITE)     // Replace first element
    {
        (*m)->bytes -= cn->datalen+1;
        if (bb_map_node_set(cn, key, data, datalen) != OK)
            return NOT_OK;
        (*m)->bytes += cn->datalen+1;
    }
    else                                    // Replace matching key, or insert
    {
        while (cn->next != NULL) {
            cn = cn->next;
            if (strcmp(cn->key, key) == 0) { // Key match, replace data
                (*m)->bytes -= cn->datalen+1;
                if (bb_map_node_set_data(cn, data, datalen) != OK)
                {
                    return NOT_OK;
                }
                else
                {
                    (*m)->bytes += datalen+1;
                    return OK;
                }
            }
        } // End of list

        bb_map_node *n = bb_map_node_init();
        if (bb_map_node_set(n, key, data, datalen) != OK)
        {
            bb_map_node_clear(n);
            return 1;
        }
        (*m)->bytes += datalen+1;
        (*m)->node_count += 1;
        cn->next = n;
    }

    return OK;
}
bb_map_node *bb_map_get(bb_map *m, const char *key)
{
    unsigned long l = djb2_hash((unsigned char*)key) % m->size;

    bb_map_node *n = m->nodes[l];

    while (n != NULL) {
        if (strcmp(n->key, key) == 0) {
            return n;
        } else {
            n = n->next;
        }
    }

    return NULL;
}
int bb_map_remove(bb_map *m, const char *key)
{
    unsigned long l = djb2_hash((unsigned char*)key) % m->size;

    bb_map_node *n = m->nodes[l];
    bb_map_node *next = n->next;

    if (n == NULL) return OK;

    int is_first = 1;
    while (n != NULL)
    {
        if (strcmp(n->key, key) == 0)
        {
            m->bytes -= n->datalen+1;
            m->node_count -= 1;
            if (is_first) m->filled_slots -= 1;
            bb_map_node_clear(n);
            m->nodes[l] = next;
            break;
        }
        n = n->next;
        next = n->next;

        is_first = 0;
    }

    return OK;
}
