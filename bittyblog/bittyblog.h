//
//  bittyblog.h
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#ifndef bittyblog_h
#define bittyblog_h

#include "cgi.h"
#include "vec.h"
#include "db_interface.h"

#define GET_ENV_VAR(X) ( (getenv(X) == NULL) ? "" : getenv(X) )

#define PARSE_GET     1
#define PARSE_POST    2

// Style definitions
enum bb_page_styles {
    BLOG_FULL_POST  = 0,
    BLOG_SMALL_POST = 1,
    CONTACT         = 2
};

// Information on the type of webpage
typedef struct {
    int id;
    char* id_name;
    char* name;
    int style;
} bb_page;

// Main page request state
typedef struct {
    // CGI Vars
    char *request_method;
    char *script_name;
    char *page_name;
    // URI query variables
    query_var *q_vars;

    // List of available pages
    bb_vec *pages;
    // Pointer to requested page
    bb_page *page;

    // Basic settings
    char *copyright_owner;
    char *navbar_title;
    char *html_title;
    char *db_path;
    char *image_dir;

    // List of posts for the page request
    vector_p *posts;
    int total_post_count;
} bb_page_request;

void bb_init(bb_page_request *, int options);
void bb_free(bb_page_request *);

void bb_load_posts(bb_page_request *);
bb_vec * bb_image_list(bb_page_request *);

#endif /* bittyblog_h */