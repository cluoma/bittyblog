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
#include <time.h>

#define GET_ENV_VAR(X) ( (getenv(X) == NULL) ? "" : getenv(X) )

#define POST            1
#define PAGE            2

// Option flags
#define PARSE_GET       1
#define PARSE_POST      2

// For image list
#define ALL             0
#define THUMBNAILS      1

// Database connection
enum db_types  { DB_SQLITE, DB_TYPE_LAST };
enum db_modes  { DB_READ, DB_WRITE, DB_MODE_LAST };
enum db_states { DB_OPEN, DB_CLOSED, DB_STATE_LAST };
typedef struct {
	int dbtype;
	int mode;
	int state;
	void *con;
} db_conn;

// User information
typedef struct {
    int id;
    char* email;
    char* name_id;
    char* name;
    char* about;
    char* thumbnail;
} bb_user;

// Post struct
typedef struct {
    int p_id;
    int page_id;
    int visible;
    char *page;
    char *title;
    char *text;
    unsigned long text_len;
    char *markdown;
    char *time;
    time_t time_r;
    char *byline;
    char *extra;
    char *thumbnail;
    bb_user user;
    bb_vec *tags;
} bb_post;

// Information on the type of webpage
typedef struct {
    int id;
    char* id_name;
    char* name;
    int style;
    bb_vec *tags;
} bb_page;

// Main page request state
typedef struct {
    // Database connection
    db_conn *dbcon;

    // CGI Vars
    char *request_method;
    char *script_name;
    char *uri;
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
    bb_vec *posts;
    int total_post_count;

    // Is this a rewrite?
    int rewrite;
} bb_page_request;

void bb_init(bb_page_request *, int options);
void bb_free(bb_page_request *);

void bb_set_dbcon(bb_page_request *, db_conn *);
void bb_load_pages(bb_page_request *);
void bb_load_posts(bb_page_request *);

bb_vec * bb_image_list(bb_page_request *, int thumbnail_only);
bb_vec * tokenize_tags(const char *str, const char * delim);
long bb_strtol(char *str, long def);
char *bb_strcpy(const char* str);

/*
 * bb struct functions
 */
int bb_user_init(bb_user *);
void bb_user_free(bb_user *);
int bb_post_init(bb_post *);
void bb_post_free(bb_post *);

// These should probably be in CGI
int bb_check_accept_encoding(const char *enc);
unsigned long bb_gzip_compress(const char* input, int inputSize, char* output, int outputSize);

#endif /* bittyblog_h */