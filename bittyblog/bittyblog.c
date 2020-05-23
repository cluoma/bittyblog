//
//  bittyblog.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include "bittyblog.h"
#include "cgi.h"
#include "vec.h"
#include "tinydir.h"
#include "config.h"
#include "db_interface.h"

void handle_rewrite(bb_page_request *req)
{
    // Save these
    char *search    = bb_strcpy(bb_cgi_get_var(req->q_vars, "search"));
    char *start     = bb_strcpy(bb_cgi_get_var(req->q_vars, "start"));

    char *uri0 = bb_strcpy(bb_cgi_get_var(req->q_vars, "uripath0"));
    char *uri1 = bb_strcpy(bb_cgi_get_var(req->q_vars, "uripath1"));
    char *uri2 = bb_strcpy(bb_cgi_get_var(req->q_vars, "uripath2"));

    if (strcmp(uri0, "") != 0) {
        if (strcmp(uri0, "post") == 0)
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "id", uri1, strlen(uri1)+1);
        }
        else if (strcmp(uri0, "search") == 0)
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "search", search, strlen(search)+1);
            bb_cgi_add_var(&(req->q_vars), "start", start, strlen(start)+1);
        }
        else if (strcmp(uri0, "tag") == 0)
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "tag", uri1, strlen(uri1)+1);
            bb_cgi_add_var(&(req->q_vars), "start", start, strlen(start)+1);
        }
        else if (strcmp(uri0, "author") == 0)
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "author", uri1, strlen(uri1)+1);
            bb_cgi_add_var(&(req->q_vars), "start", start, strlen(start)+1);
        }
        else if (strcmp(uri0, "archive") == 0)
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "year", uri1, strlen(uri1)+1);
            bb_cgi_add_var(&(req->q_vars), "month", uri2, strlen(uri2)+1);
        }
        else
        {
            bb_cgi_remove_all_var(&(req->q_vars));
            bb_cgi_add_var(&(req->q_vars), "page", uri0, strlen(uri0)+1);
            bb_cgi_add_var(&(req->q_vars), "start", start, strlen(start)+1);

            // add rss if it is the second argument
            if (strcmp(uri1, "rss") == 0)
                bb_cgi_add_var(&(req->q_vars), "rss", uri1, strlen(uri1)+1);
        }
    }
    else
    {
        bb_cgi_remove_all_var(&(req->q_vars));
        bb_cgi_add_var(&(req->q_vars), "start", start, strlen(start)+1);
    }

    // Free everything
    free(search); free(start);
    free(uri0); free(uri1); free(uri2);
}

void bb_free(bb_page_request *req)
{   
    // Free query vars
    query_var *curr, *prev;
    curr = req->q_vars;
    while (curr != NULL) {
        free(curr->key);
        free(curr->val);
        prev = curr;
        curr = curr->next;
        free(prev);
    }

    // Free list of all pages on the site
    if (req->pages != NULL) {
        bb_vec_free(req->pages);
    }

    // Free list of posts
    if (req->posts != NULL) {
        bb_vec_free(req->posts);
    }
}

void bb_init(bb_page_request *req, int options)
{
    // Set NULL database connection
    req->dbcon = NULL;

    // Get environment variables
    req->request_method = GET_ENV_VAR("REQUEST_METHOD");
    req->script_name    = GET_ENV_VAR("SCRIPT_NAME");
    
    // Parse query string and POST data
    req->q_vars = NULL;
    if (options & PARSE_GET) {
        req->q_vars = bb_cgi_get_query(GET_ENV_VAR("QUERY_STRING"));
    }
    if (options & PARSE_POST) {
        req->q_vars = bb_cgi_get_post(req->q_vars);
    }

    // Handle rewrite if one was supplied in the query string
    if (bb_cgi_get_var(req->q_vars, "rewrite") != NULL) {
        req->q_vars = bb_cgi_get_uri(req->q_vars, bb_cgi_get_var(req->q_vars, "rewrite"));
        handle_rewrite(req);
        req->rewrite = 1;
    } else {
        req->rewrite = 0;
    }

    // Get the requested page name, default to DEFAULT_PAGE if none was supplied
    char * page = bb_cgi_get_var(req->q_vars, "page");
    if( page == NULL ) {
        bb_cgi_add_var(&req->q_vars, "page", DEFAULT_PAGE, strlen(DEFAULT_PAGE)+1);
        req->page_name = bb_cgi_get_var(req->q_vars, "page");
    }
    else {
        req->page_name = page;
    }

    // Fill in the rest of the needed info
    req->copyright_owner    = COPYRIGHT_OWNER;
    req->navbar_title       = NAVBAR_TITLE;
    req->html_title         = HTML_TITLE;
    req->db_path            = DB_PATH;
    req->image_dir          = IMAGE_PATH;

    // Init variables for blog posts
    req->posts = NULL;
    req->total_post_count = 0;
}

void bb_set_dbcon(bb_page_request *req, db_conn *con) {
    req->dbcon = con;
}

// Load all pages, and request page, from database
void bb_load_pages(bb_page_request *req) {
    // Get a list of all pages on the site
    req->pages = db_pages(req->dbcon);
    req->page = NULL;
    // Check if the request page exists
    for (int i = 0; i < bb_vec_count(req->pages); i++) {
        bb_page *page = bb_vec_get(req->pages, i);
        if (strcmp(page->id_name, req->page_name) == 0) {
            req->page = page;
            break;
        }
    }
}

// Load the requested posts and store them
void bb_load_posts(bb_page_request *req) {

    bb_vec * entries;

    char *search    = bb_cgi_get_var( req->q_vars, "search" );
    char *tag       = bb_cgi_get_var( req->q_vars, "tag" );
    char *author    = bb_cgi_get_var( req->q_vars, "author" );
    char *id        = bb_cgi_get_var( req->q_vars, "id" );
    char *start     = bb_cgi_get_var( req->q_vars, "start" );
    char *month     = bb_cgi_get_var( req->q_vars, "month" );
    char *year      = bb_cgi_get_var( req->q_vars, "year" );

    if (id == NULL)
    {
        if (search != NULL)
        {
            entries = db_nsearch(req->dbcon, req->page->id_name, search, POSTS_PER_PAGE, (int)bb_strtol(start, 0));
            req->total_post_count = db_search_count(req->dbcon, req->page->id_name, search);
        }
        else if (tag != NULL)
        {
            entries = db_ntag(req->dbcon, tag, POSTS_PER_PAGE, (int)bb_strtol(start, 0));
            req->total_post_count = db_tag_count(req->dbcon, tag);
        }
        else if (author != NULL)
        {
            entries = db_nauthor(req->dbcon, author, POSTS_PER_PAGE, (int)bb_strtol(start, 0));
            req->total_post_count = db_author_count(req->dbcon, author);
        }
        else if (start != NULL)
        {
            entries = db_nposts(req->dbcon, req->page->id_name, POSTS_PER_PAGE, (int)bb_strtol(start, 0));
            req->total_post_count = db_count(req->dbcon, req->page->id_name);
        }
        else if (month != NULL && year != NULL)
        {
            entries = db_monthyear(req->dbcon, req->page->id_name, (int)bb_strtol(month, 1), (int)bb_strtol(year, 1));
        }
        else
        {
            entries = db_nposts(req->dbcon, req->page->id_name, POSTS_PER_PAGE, 0);
            req->total_post_count = db_count(req->dbcon, req->page->id_name);
        }
    }
    else
    {
        entries = db_id(req->dbcon, (int)bb_strtol(id, 1));
    }
    req->posts = entries;
}

/*
 * bb_user struct fuctions
 */
int bb_user_init(bb_user *u)
{
    u->id           = -1;
    u->email        = NULL;
    u->name_id      = NULL;
    u->name         = NULL;
    u->about        = NULL;
    u->thumbnail    = NULL;

    return 0;
}
void bb_user_free(bb_user *u)
{
    if(u->email != NULL)        free(u->email);
    if(u->name_id != NULL)      free(u->name_id);
    if(u->name != NULL)         free(u->name);
    if(u->about != NULL)        free(u->about);
    if(u->thumbnail != NULL)    free(u->thumbnail);
}

/*
 * bb_post struct functions
 */
int bb_post_init(bb_post* p) {
    p->p_id = -1;
    p->page_id = -1;
    p->time_r = 0;
    p->page = NULL;
    p->title = NULL;
    p->text = NULL;
    p->text_len = 0;
    p->markdown = NULL;
    p->time = NULL;
    p->byline = NULL;
    p->extra = NULL;
    p->thumbnail = NULL;
    p->visible = 0;
    p->tags = NULL;

    bb_user_init(&(p->user));

    return 0;
}
void bb_post_free(bb_post* p) {
    if (p->page != NULL)        free(p->page);
    if (p->title != NULL)       free(p->title);
    if (p->text != NULL)        free(p->text);
    if (p->markdown != NULL)    free(p->markdown);
    if (p->time != NULL)        free(p->time);
    if (p->byline != NULL)      free(p->byline);
    if (p->extra != NULL)       free(p->extra);
    if (p->thumbnail != NULL)   free(p->thumbnail);

    if (p->tags != NULL)        bb_vec_free(p->tags);
    
    bb_user_free(&(p->user));
}

// Parse string into long, return def if none found or error
long bb_strtol(char *str, long def)
{
    long ret_val;
    char *endptr;
    errno = 0;

    if (str == NULL) {
        return def;
    }

    ret_val = strtol(str, &endptr, 10);
    if (errno || endptr == str) {
        return def;
    } else {
        return ret_val;
    }
}

char *bb_strcpy(const char* str)
{
    char *ret;
    if (str == NULL)
    {
        ret = malloc(1);
        ret[0] = '\0';
    }
    else
    {
        int len = strlen(str);
        ret = malloc(len+1);
        strcpy(ret, str);
        ret[len] = '\0';
    }
    return ret;
}

bb_vec * bb_image_list(bb_page_request *req, int thumbnail_only) {
    tinydir_dir dir;
    tinydir_open_sorted(&dir, req->image_dir);

    bb_vec * image_files = malloc(sizeof(bb_vec));
    bb_vec_init(image_files, NULL);

    for(int i = 0; i < dir.n_files; i++)
    {
	    tinydir_file file;
	    tinydir_readfile_n(&dir, &file, i);

	    if (!file.is_dir)
	    {
		    if(strcmp(file.extension, "jpg")  == 0 ||
               strcmp(file.extension, "JPG")  == 0 ||
               strcmp(file.extension, "jpeg") == 0 ||
               strcmp(file.extension, "png")  == 0 ||
               strcmp(file.extension, "PNG")  == 0 ||
               strcmp(file.extension, "gif")  == 0)
            {
                if ( !thumbnail_only ||
                    (thumbnail_only && strstr(file.name, ".thumbnail.") != NULL))
                {
                    char *name = calloc(256+1, 1);
                    strncpy(name, file.name, 256);
                    bb_vec_add(image_files, name);
                }
            }
	    }
    }
    tinydir_close(&dir);

    return image_files;
}

// Returns vector tokenized by delim, leading and trailing whitespace is removed
bb_vec * tokenize_tags(const char *str, const char * delim)
{
    char *s;
    char *token;
    char *str_cpy;
    int str_length = strlen(str);

    str_cpy = malloc(str_length+1);
    memcpy(str_cpy, str, str_length);
    str_cpy[str_length] = '\0';
    str_cpy = strtrim(str_cpy);

    bb_vec *vec = malloc(sizeof(bb_vec));
    bb_vec_init(vec, NULL);

    token = strtok_r(str_cpy, delim, &s);

    while(token != NULL)
    {
        size_t token_length = strlen(token);
        char *tmp = calloc(token_length+1, 1);
        memcpy(tmp, token, token_length);
        tmp = strtrim(tmp);
        if (*tmp != '\0') {
            bb_vec_add(vec, tmp);
        } else {
            free(tmp);
        }
        token = strtok_r(NULL, delim, &s);
    }
    free(str_cpy);

    return vec;
}

// Checks the HTTP_ACCEPT_ENCODING env var for the argument given in enc
int bb_check_accept_encoding(const char *enc)
{
    char *accept_encoding = GET_ENV_VAR("HTTP_ACCEPT_ENCODING");

    if (accept_encoding == NULL) return 0;
    if (strstr(accept_encoding, enc) == NULL) return 0;

    return 1;
}

// Compress a buffer using gzip, return byte length of compressed data
unsigned long bb_gzip_compress(const char* buf, int buf_len, char* out, int out_len)
{
    // Initialize stream
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = (uInt)buf_len;
    zs.next_in = (Bytef *)buf;
    zs.avail_out = (uInt)out_len;
    zs.next_out = (Bytef *)out;

    // Adding 16 to window bits tells zlib to create gzip headers
    deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    return zs.total_out;
}