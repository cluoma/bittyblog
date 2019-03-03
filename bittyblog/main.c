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

int main()
{
    //for (int i =0; i < 100000; i++) {
    // Init page request
    bb_page_request req;
    bb_init(&req, PARSE_GET);

    // If page does not exist, goto a page not found page
    if (req.page == NULL) {
        bb_free(&req);
        return 0;
    }
    Archives archives;

    char *id;

    /* Get QueryString variables */
    id = bb_cgi_get_var( req.q_vars, "id" );

    // If we have an id, always use FULL BLOG POST style
    if(id) req.page->style = BLOG_FULL_POST;

    // Initiate JSON objects
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    bb_default_to_json(root_object, &req);

    // Load the currect template file, based on page style
    DString *template;
    switch(req.page->style) {
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
        default:
            snprintf(dir_base, 1023, "%s/blog.m", TEMPLATE_PATH);
            template = scan_file(dir_base);
            break;
    }

    bb_load_posts(&req);
    bb_posts_to_json(root_object, &req, 1);

    archives = load_archives();
    bb_archives_to_json(root_object, &archives);
    free_archives(&archives);

    bb_nav_buttons_to_json(root_object, &req);

    // char *serialized_string;
    // serialized_string = json_serialize_to_string_pretty(root_value);
    // puts(serialized_string);
    // json_free_serialized_string(serialized_string);

    DString * out = d_string_new("");
    magnum_populate_from_json(template, root_value, out, NULL, NULL);

    // Start of HTML output
    printf("Content-Length: %lu\r\n", out->currentStringLength);
    printf("Content-Type: text/html\r\n\r\n");
    printf("%s", out->str);

    d_string_free(template, 1);
    d_string_free(out, 1);
    json_value_free(root_value);

    bb_free(&req);
    //}
}
