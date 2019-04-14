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
#include <fcgi_stdio.h>
#endif

int main()
{
    
#ifdef _FCGI
    while(FCGI_Accept() >= 0 ) {
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
        case MISSING:
            snprintf(dir_base, 1023, "%s/404.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
        default:
            snprintf(dir_base, 1023, "%s/blog.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
    }

    DString * out = d_string_new("");
    magnum_populate_from_json(template, root_value, out, TEMPLATE_PATH, NULL);

    // Start of HTML output
    printf("Content-Length: %lu\r\n", out->currentStringLength);
    printf("Content-Type: text/html\r\n\r\n");
    printf("%s", out->str);

    d_string_free(template, 1);
    d_string_free(out, 1);
    json_value_free(root_value);

    bb_free(&req);
    
#ifdef _FCGI
    }
#endif
}
