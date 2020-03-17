//
//  main.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#define DEFINE_TEMPLATES
#include "main.h"
#include "cgi.h"
#include "db_interface.h"
#include "config.h"
#include "vec.h"
#include "bittyblog.h"
#include "to_json.h"

#include <parson.h>
#include <libMagnum.h>
#include <d_string.h>
#include <file.h>

// Modules
#include "mod_archives.h"

#ifdef _FCGI
#include "cachemap.h"
#include <fcgi_stdio.h>
#endif


struct timeval timer;
int start_timer()
{
    timer.tv_sec = 0; timer.tv_usec = 0;
    if (gettimeofday(&timer, NULL)) return 1;
    return 0;
}
int end_timer()
{
    struct timeval cur;
    if (timer.tv_sec == 0 && timer.tv_usec == 0) return 1;
    if (gettimeofday(&cur, NULL)) return 1;

    timer.tv_sec = cur.tv_sec - timer.tv_sec;
    timer.tv_usec = cur.tv_usec - timer.tv_usec;

    return 0;
}

int main(int argc, char **argv, char **envp)
{

#ifdef _FCGI
    DString *uri;
    bb_map *cache;
    if (USE_CACHE) {
        cache = bb_map_init(CACHE_INIT_BUCKETS, MAX_CACHE_BYTES, CACHE_TIMEOUT_SECONDS);
    }

    // Open long-lived database connection
    db_conn *con = db_open_conn(DB_SQLITE, DB_WRITE);
    if (con == NULL)
        return 1;

    while(FCGI_Accept() >= 0) {
#endif

        start_timer();

        // Check if we can use gzip
        int accept_gzip = bb_check_accept_encoding("gzip");

#ifdef _FCGI
        if (USE_CACHE) {
            uri = d_string_new(getenv("REQUEST_URI"));
            if (accept_gzip)
                d_string_prepend(uri, "gzip::");
            bb_map_node *cached_resp = bb_map_get(cache, uri->str);
            time_t last_db_update = db_get_last_update(con);
            if (cached_resp != NULL && cached_resp->time >= last_db_update) {
                
                if(end_timer()) {
                    printf("X-bb-time: %ld.%06ld\r\n", (long)0, (long)0);
                } else {
                    printf("X-bb-time: %ld.%06ld\r\n", timer.tv_sec, timer.tv_usec);
                }

                printf("X-bb-cache: hit\r\n");
                fwrite(cached_resp->data, 1, cached_resp->datalen, stdout);

                FCGI_Finish();

                d_string_free(uri, 1);
                continue;
            }
        }
#endif

#ifndef _FCGI
    // Open short-lived database connection
    db_conn *con = db_open_conn(DB_SQLITE, DB_WRITE);
    if (con == NULL)
        return 1;
#endif

    // Init page request
    bb_page_request req;
    bb_init(&req, PARSE_GET);
    bb_set_dbcon(&req, con);
    bb_load_pages(&req);

    // Initiate JSON objects
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    bb_default_to_json(root_object, &req);

    // If page exists, load posts and archive data
    if (req.page != NULL) {
        // Posts
        bb_load_posts(&req);
        bb_posts_to_json(root_object, &req, 1);

        // Special info box
        bb_special_info_box_to_json(root_object, &req);

        // Archives
        Archives archives;
        archives = mod_archives_load(con);
        mod_archives_to_json(root_object, &archives);
        mod_archives_free(&archives);

        // Set nav buttons
        bb_nav_buttons_to_json(root_object, &req);
    }

    // If we have an id, always use FULL BLOG POST style
    if( req.page != NULL && bb_cgi_get_var(req.q_vars, "id") ) req.page->style = BLOG_SINGLE_POST;
    // If we have rss, use the rss theme
    if( req.page != NULL && bb_cgi_get_var(req.q_vars, "rss") ) req.page->style = RSS;

    // Load the currect template file, based on page style
    DString *template;
    int page_style = req.page != NULL ? req.page->style : MISSING;
    char dir_base[1024];
    if (page_style >= 0 && page_style < STYLE_LAST)
    {
        snprintf(dir_base, 1023, "%s/%s", TEMPLATE_PATH, style_template[page_style]);
        template = scan_file(dir_base);
    }
    else {
        snprintf(dir_base, 1023, "%s/%s", TEMPLATE_PATH, style_template[0]);
        template = scan_file(dir_base);
    }

    // Response content
    DString *out = d_string_new("");
    magnum_populate_from_json(template, root_value, out, TEMPLATE_PATH, NULL);

    // Compress output if we need to
    if (accept_gzip) {
        char *cmp_out = malloc(out->currentStringLength);
        unsigned long cmp_len = bb_gzip_compress(out->str, out->currentStringLength,
                                                 cmp_out, out->currentStringLength);
        d_string_free(out, 1);
        out = d_string_new("");
        d_string_append_c_array(out, cmp_out, cmp_len);
        free(cmp_out);
    }

    // Response headers
    // Add content length and encoding type
    DString *headers = d_string_new("");
    d_string_append_printf(headers, "Content-Length: %lu\r\n", out->currentStringLength);
    if (accept_gzip) d_string_append_printf(headers, "Content-Encoding: gzip\r\n");
    if (page_style == RSS)
    {
        d_string_append_printf(headers, "Content-Type: application/rss+xml\r\n\r\n");
    }
    else
    {
        d_string_append_printf(headers, "Content-Type: text/html\r\n\r\n");
    }
    // Add timer
    if(end_timer()) {
        printf("X-bb-time: %ld.%06ld\r\n", (long)0, (long)0);
    } else {
        printf("X-bb-time: %ld.%06ld\r\n", timer.tv_sec, timer.tv_usec);
    }
    // Add cache miss
    printf("X-bb-cache: miss\r\n%s", headers->str);

    // Start of HTML output
    fwrite(out->str, 1, out->currentStringLength, stdout);

#ifdef _FCGI
    FCGI_Finish();
    if (USE_CACHE) {
        d_string_prepend(out, headers->str);
        bb_map_insert(&cache, uri->str, out->str, out->currentStringLength);
        d_string_free(uri, 1);
    }
#endif

    d_string_free(template, 1);
    d_string_free(headers, 1);
    d_string_free(out, 1);
    json_value_free(root_value);

    bb_free(&req);

#ifndef _FCGI
    db_close_conn(con);
#endif
    
#ifdef _FCGI
    }  // Leave Accept loop

    // Close database connection
    db_close_conn(con);

    // Final cleanup
    if (USE_CACHE) {
        bb_map_free(cache);
    }
#endif
}
