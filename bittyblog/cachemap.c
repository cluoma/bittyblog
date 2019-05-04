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
    if (n == NULL) return BBMAP_OK;

    if (n->key != NULL) free(n->key);
    if (n->data != NULL) free(n->data);
    free(n);

    return BBMAP_OK;
}
int bb_map_node_set_key(bb_map_node *n, const char* key)
{   
    if (n->key != NULL) free(n->key);

    n->key = strcopy(key, strlen(key));
    if (n->key == NULL)
        return BBMAP_NOT_OK;
    
    return BBMAP_OK;
}
int bb_map_node_set_data(bb_map_node *n, const char* data, unsigned long datalen)
{
    if (n->data != NULL) free(n->data);

    n->data = strcopy(data, datalen);
    if (n->data == NULL)
    {
        free(n->key);
        return BBMAP_NOT_OK;
    }
    n->datalen = datalen;
    n->time = time(NULL);

    return BBMAP_OK;
}
int bb_map_node_set(bb_map_node *n, const char* key, const char* data, unsigned long datalen)
{
    
    if ( bb_map_node_set_key(n, key) != BBMAP_OK ||
         bb_map_node_set_data(n, data, datalen) != BBMAP_OK )
    {
        return BBMAP_NOT_OK;
    }

    return BBMAP_OK;
}

/*
 * Hash Map Functions
 */
bb_map *bb_map_init(unsigned long size, unsigned long long max_bytes, time_t timeout)
{
    bb_map *m = malloc(sizeof(bb_map));
    if (m == NULL) return NULL;

    //m->options      = options;

    m->timeout      = timeout;

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

    return BBMAP_OK;
}
int bb_map_free(bb_map *m)
{
    bb_map_purge(m);
    free(m->nodes);
    free(m);

    return 0;
}

// Insert a node into a table without copying
int bb_map_insert_nocpy(bb_map **m, bb_map_node *n)
{
    unsigned long l = djb2_hash((unsigned char*)(n->key)) % (*m)->size;

    bb_map_node *cn = (*m)->nodes[l];
    if ( cn == NULL )                       // Empty linked list
    {
        (*m)->bytes         += n->datalen+1;
        (*m)->filled_slots  += 1;
        (*m)->node_count    += 1;

        n->next = NULL;
        (*m)->nodes[l] = n;
    }
    else                                    // Replace matching key, or insert
    {
        bb_map_node *prev;
        while (cn != NULL) {
            prev = cn;
            cn = cn->next;
        } // End of list

        (*m)->bytes += n->datalen+1;
        (*m)->node_count += 1;
        n->next = NULL;
        prev->next = n;
    }

    return BBMAP_OK;
}
// Double the size of the hash table
int bb_map_resize(bb_map **m)
{
    bb_map *new = bb_map_init((*m)->size*2, (*m)->max_bytes, (*m)->timeout);

    for (unsigned long i = 0; i < (*m)->size; i++)
    {
        bb_map_node *n = (*m)->nodes[i];
        bb_map_node *next;
        while (n != NULL)
        {
            next = n->next;
            bb_map_insert_nocpy(&new, n);
            n = next;
        }
    }
    free((*m)->nodes);
    free((*m));
    *m = new;

    return BBMAP_OK;
}
// Remove stale hash table entries to try and free up space
int bb_map_cleanup(bb_map *m)
{
    time_t cur_time = time(NULL);

    for (unsigned long i = 0; i < m->size; i++)
    {
        bb_map_node *n = m->nodes[i];
        bb_map_node *next;
        while (n != NULL)
        {
            next = n->next;
            if (cur_time-n->time >= m->timeout)
                bb_map_remove(m, n->key);
            n = next;
        }
    }

    return BBMAP_OK;
}

int bb_map_insert(bb_map **m, char *key, char *data, unsigned long datalen)
{
    // Check if we have enough data to add, if not try to purge old entries
    // then try again
    if ((*m)->bytes + datalen + 1 > (*m)->max_bytes)
    {
        if ((*m)->timeout > 0)
        {
            bb_map_cleanup(*m);
            if ((*m)->bytes + datalen + 1 > (*m)->max_bytes)
            {
                return BBMAP_FULL;
            }
        }
        else
        {
            return BBMAP_FULL;
        }
    }
    
    // Expand hash table if we need to
    if ((float)((float)(*m)->filled_slots/(float)(*m)->size) >= (float)0.6)
    {
        bb_map_resize(m);
    }

    unsigned long l = djb2_hash((unsigned char*)key) % (*m)->size;

    bb_map_node *cn = (*m)->nodes[l];
    if ( cn == NULL )                       // Empty linked list
    {
        bb_map_node *n = bb_map_node_init();
        if (bb_map_node_set(n, key, data, datalen) != BBMAP_OK)
        {
            bb_map_node_clear(n);
            return BBMAP_NOT_OK;
        }

        (*m)->bytes         += datalen+1;
        (*m)->filled_slots  += 1;
        (*m)->node_count    += 1;

        (*m)->nodes[l] = n;
    }
    // else if ((*m)->options & CACHE_OVERWRITE)     // Replace first element
    // {
    //     (*m)->bytes -= cn->datalen+1;
    //     if (bb_map_node_set(cn, key, data, datalen) != BBMAP_OK)
    //     {
    //         return BBMAP_NOT_OK;
    //     }
    //     (*m)->bytes += cn->datalen+1;
    // }
    else                                    // Replace matching key, or insert
    {
        bb_map_node *prev;
        while (cn != NULL) {
            if (strcmp(cn->key, key) == 0) { // Key match, replace data
                (*m)->bytes -= cn->datalen+1;
                if (bb_map_node_set_data(cn, data, datalen) == BBMAP_OK)
                {
                    (*m)->bytes += datalen+1;
                    return BBMAP_OK;
                }
                else
                {
                    return BBMAP_NOT_OK;
                }
            }
            prev = cn;
            cn = cn->next;
        } // End of list

        bb_map_node *n = bb_map_node_init();
        if (bb_map_node_set(n, key, data, datalen) != BBMAP_OK)
        {
            bb_map_node_clear(n);
            return BBMAP_NOT_OK;
        }
        (*m)->bytes += datalen+1;
        (*m)->node_count += 1;
        prev->next = n;
    }

    return BBMAP_OK;
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
    if (n == NULL) return BBMAP_OK;

    bb_map_node *prev = NULL;
    bb_map_node *next;

    int is_first = 1;
    while (n != NULL)
    {
        next = n->next;
        if (strcmp(n->key, key) == 0)
        {
            m->bytes -= n->datalen+1;
            m->node_count -= 1;
            if (is_first && next == NULL) m->filled_slots -= 1;
            bb_map_node_clear(n);

            if (prev == NULL)
            {
                m->nodes[l] = next;
            }
            else
            {
                prev->next = next;
            }
            break;
        }
        prev = n;
        n = n->next;

        is_first = 0;
    }

    return BBMAP_OK;
}
