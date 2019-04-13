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
#include "bittyblog.h"
#include "cgi.h"
#include "vec.h"
#include "tinydir.h"
#include "config.h"
#include "db_interface.h"

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

void handle_rewrite(bb_page_request *req)
{
    // Save these
    char *search = bb_strcpy(bb_cgi_get_var(req->q_vars, "search"));
    char *start = bb_strcpy(bb_cgi_get_var(req->q_vars, "start"));

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
        }
    }
    else
    {
        bb_cgi_remove_all_var(&(req->q_vars));
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
        //vector_p_free(req->posts);
        bb_vec_free(req->posts);
    }
}

void bb_init(bb_page_request *req, int options)
{
    // Get environment variables
    req->request_method = GET_ENV_VAR("REQUEST_METHOD");
    req->script_name = GET_ENV_VAR("SCRIPT_NAME");
    
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

    // Get the requested page name, default to 'blog' if none was supplied
    char * page = bb_cgi_get_var(req->q_vars, "page");
    if( page == NULL ) {
        bb_cgi_add_var(&req->q_vars, "page", "blog", strlen("blog")+1);
        req->page_name = bb_cgi_get_var(req->q_vars, "page");
    }
    else {
        req->page_name = page;
    }

    // Get a list of all pages on the site
    req->pages = db_pages();
    req->page = NULL;
    // Check if the request page exists
    for (int i = 0; i < bb_vec_count(req->pages); i++) {
        bb_page *page = bb_vec_get(req->pages, i);
        if (strcmp(page->id_name, req->page_name) == 0) {
            req->page = page;
            break;
        }
    }

    // Fill in the rest of the needed info
    req->copyright_owner = COPYRIGHT_OWNER;
    req->navbar_title = NAVBAR_TITLE;
    req->html_title = HTML_TITLE;
    req->db_path = DB_PATH;
    req->image_dir = IMAGE_PATH;

    // Init variables for blog posts
    req->posts = NULL;
    req->total_post_count = 0;
}

// Load the requested posts and store them
void bb_load_posts(bb_page_request *req) {

    bb_vec * entries;

    char *search    = bb_cgi_get_var( req->q_vars, "search" );
    char *tag       = bb_cgi_get_var( req->q_vars, "tag" );
    char *id        = bb_cgi_get_var( req->q_vars, "id" );
    char *start     = bb_cgi_get_var( req->q_vars, "start" );
    char *month     = bb_cgi_get_var( req->q_vars, "month" );
    char *year      = bb_cgi_get_var( req->q_vars, "year" );

    if (id == NULL)
    {
        if (search != NULL)
        {
            entries = db_nsearch(req->page->id_name, search, POSTS_PER_PAGE, (start == NULL ? 0 : (int)bb_strtol(start, 0)));
            req->total_post_count = db_search_count(req->page->id_name, search);
        }
        else if (tag != NULL)
        {
            entries = db_ntag(tag, POSTS_PER_PAGE, (start == NULL ? 0 : (int)bb_strtol(start, 0)));
            req->total_post_count = db_tag_count(tag);
        }
        else if (start != NULL)
        {
            entries = db_nposts(req->page->id_name, POSTS_PER_PAGE, (int)bb_strtol(start, 0));
            req->total_post_count = db_count(req->page->id_name);
        }
        else if (month != NULL && year != NULL)
        {
            entries = db_monthyear(req->page->id_name, (int)bb_strtol(month, 1), (int)bb_strtol(year, 1));
        }
        else
        {
            entries = db_nposts(req->page->id_name, POSTS_PER_PAGE, 0);
            req->total_post_count = db_count(req->page->id_name);
        }
    }
    else
    {
        entries = db_id((int)bb_strtol(id, 1));
    }
    req->posts = entries;
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
