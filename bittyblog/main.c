//
//  main.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

#ifdef _FCGI
#include "cachemap.h"
#include <fcgi_stdio.h>
#endif

int main(int argc, char **argv, char **envp)
{

#ifdef _FCGI
    DString *uri;
    bb_map *cache;
    if (USE_CACHE) {
        cache = bb_map_init(CACHE_INIT_BUCKETS, MAX_CACHE_BYTES, CACHE_TIMEOUT_SECONDS);
    }

    while(FCGI_Accept() >= 0) {
        // Check if we can use gzip
        int accept_gzip = bb_check_accept_encoding("gzip");

        if (USE_CACHE) {
            uri = d_string_new(getenv("REQUEST_URI"));
            if (accept_gzip)
                d_string_prepend(uri, "gzip::");
            bb_map_node *cached_resp = bb_map_get(cache, uri->str);
            time_t last_db_update = db_get_last_update();
            if (cached_resp != NULL && cached_resp->time >= last_db_update) {
                printf("X-bb-cache: hit\r\n");
                fwrite(cached_resp->data, 1, cached_resp->datalen, stdout);
                d_string_free(uri, 1);
                continue;
            }
        }
#endif
#ifndef _FCGI
    // Check if we can use gzip
    int accept_gzip = bb_check_accept_encoding("gzip");
#endif

    // Init page request
    bb_page_request req;
    bb_init(&req, PARSE_GET);

    // Initiate JSON objects
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    bb_default_to_json(root_object, &req);

    // If page exists, load posts and archive data
    if (req.page != NULL) {
        // Posts
        bb_load_posts(&req);
        bb_posts_to_json(root_object, &req, 1);

        // Archives
        Archives archives;
        archives = load_archives();
        bb_archives_to_json(root_object, &archives);
        free_archives(&archives);

        // Set nav buttons
        bb_nav_buttons_to_json(root_object, &req);
    }

    // If we have an id, always use FULL BLOG POST style
    if( req.page != NULL && bb_cgi_get_var(req.q_vars, "id") ) req.page->style = BLOG_FULL_POST;
    // If we have rss, use the rss theme
    if( req.page != NULL && bb_cgi_get_var(req.q_vars, "rss") ) req.page->style = RSS;

    // Load the currect template file, based on page style
    DString *template;
    int page_style = req.page != NULL ? req.page->style : MISSING;
    switch( page_style )
    {
        char dir_base[1024];
        case BLOG_FULL_POST:
            snprintf(dir_base, 1023, "%s/blog.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        case BLOG_SMALL_POST:
            snprintf(dir_base, 1023, "%s/blog_preview.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        case CONTACT:
            snprintf(dir_base, 1023, "%s/contact.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        case RSS:
            snprintf(dir_base, 1023, "%s/rss.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        case MISSING:
            snprintf(dir_base, 1023, "%s/404.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        default:
            snprintf(dir_base, 1023, "%s/blog.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
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

    // Start of HTML output
    printf("X-bb-cache: miss\r\n%s", headers->str);
    fwrite(out->str, 1, out->currentStringLength, stdout);

#ifdef _FCGI
    if (USE_CACHE) {
        // Finish CGI before updating cache
        FCGI_Finish();

        d_string_prepend(out, headers->str);
        bb_map_insert(&cache, uri->str, out->str, out->currentStringLength);
        d_string_free(uri, 1);
    }
    else
    {
        FCGI_Finish();
    }
    
#endif

    d_string_free(template, 1);
    d_string_free(headers, 1);
    d_string_free(out, 1);
    json_value_free(root_value);

    bb_free(&req);
    
#ifdef _FCGI
    }  // Leave Accept loop
    // Final cleanup
    if (USE_CACHE) {
        bb_map_free(cache);
    }
    FCGI_Finish();
#endif
}
