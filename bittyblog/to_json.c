//
//  to_json.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <time.h>
#include <errno.h>
#include <parson.h>
#include "bittyblog.h"
#include "to_json.h"
#include "db_interface.h"
#include "config.h"

int bb_default_to_json(JSON_Object *root_object, bb_page_request *req) {
    // Add SETTINGS variables to JSON
    json_object_set_string(root_object, "title", req->html_title);
    json_object_set_string(root_object, "navbar_title", req->navbar_title);
    json_object_set_string(root_object, "owner", req->copyright_owner);
    json_object_set_string(root_object, "about", ABOUT);

    // Add REQUEST variables to JSON
    json_object_set_string(root_object, "script_name", req->script_name);
    json_object_set_string(root_object, "page_name", req->page_name);

    // Add QUERY STRING variants to JSON
    // Query string without the page variable
    char *qs = bb_cgi_query_string_wo(req->q_vars, "page");
    json_object_set_string(root_object, "query_string_wo_page", qs);
    free(qs);
    // Query string without the start variable
    qs = bb_cgi_query_string_wo(req->q_vars, "start");
    json_object_set_string(root_object, "query_string_wo_start", qs);
    free(qs);

    // Add search query string variable to json
    char *search = bb_cgi_get_var(req->q_vars, "search");
    if (search) json_object_set_string(root_object, "search", search);
    // Add tag name to json if needed
    char *tag = bb_cgi_get_var(req->q_vars, "tag");
    if (tag && !search) json_object_set_string(root_object, "tag", tag);

    // Add CURRENT YEAR to JSON
    time_t timeval;
    struct tm *tp = malloc(sizeof(struct tm));
    time (&timeval);
    gmtime_r(&timeval, tp);
    json_object_set_number(root_object, "current_year", tp->tm_year+1900);
    free(tp);

    // Add PAGES to JSON
    JSON_Array *pages = json_value_get_array(json_value_init_array());
    for (int i = 0; i < req->pages->count; i++) {
        bb_page * p = (bb_page*)bb_vec_get(req->pages, i);
        JSON_Value *tmp_page = json_value_init_object();
        json_object_set_string(json_value_get_object(tmp_page), "id_name", p->id_name);
        json_object_set_string(json_value_get_object(tmp_page), "name", p->name);
        int t = strcmp(p->id_name, req->page_name) == 0 ? 1 : 0;
        json_object_set_boolean(json_value_get_object(tmp_page), "active", t);
        json_array_append_value(pages, tmp_page);
    }
    json_object_set_value(root_object, "pages", json_array_get_wrapping_value(pages));

    return 1;
}

int bb_nav_buttons_to_json(JSON_Object *root_object, bb_page_request *req) {
    char *start = bb_cgi_get_var(req->q_vars, "start");
    long s = 0;

    errno = 0;
    if( start != NULL )
        s = strtol(start, NULL, 10);
    if (errno) s = 0;

    // Older
    if ( s+POSTS_PER_PAGE < req->total_post_count ) {
        char out[20];
        sprintf(out, "%ld", s+POSTS_PER_PAGE);
        json_object_dotset_string(root_object, "nav_buttons.older", out);
    }
    // Newer
    if ( s > 0 ) {
        char out[20];
        sprintf(out, "%ld", s-POSTS_PER_PAGE);
        json_object_dotset_string(root_object, "nav_buttons.newer", out);
    }

    return 1;
}

void bb_posts_to_json(JSON_Object *root_object, bb_page_request *req, int format) {
    vector_p *entries = req->posts;
    JSON_Array *posts = json_value_get_array(json_value_init_array());
    for (int i = 0; i < entries->n; i++) {
        JSON_Value *tmp_post = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp_post), "p_id", entries->p[i].p_id);
        json_object_set_string(json_value_get_object(tmp_post), "page", entries->p[i].page);
        json_object_set_string(json_value_get_object(tmp_post), "title", entries->p[i].title);
        json_object_set_string(json_value_get_object(tmp_post), "time", entries->p[i].time);
        json_object_set_string(json_value_get_object(tmp_post), "byline", entries->p[i].byline);
        json_object_set_string(json_value_get_object(tmp_post), "extra", entries->p[i].extra);

        // Set default thumbnail if we didn't get one from the database
        if (entries->p[i].thumbnail == NULL || strcmp(entries->p[i].thumbnail, "") == 0) {
            json_object_set_string(json_value_get_object(tmp_post), "thumbnail", "bb_default.thumbnail.jpg");
        } else {
            json_object_set_string(json_value_get_object(tmp_post), "thumbnail", entries->p[i].thumbnail);
        }

        // Prepare newlines for HTML output or not
        if (format) {
            char *formatted_post_text = newline_to_html(entries->p[i].text);
            json_object_set_string(json_value_get_object(tmp_post), "text", formatted_post_text);
            free(formatted_post_text);
        } else {
            json_object_set_string(json_value_get_object(tmp_post), "text", entries->p[i].text);
        }

        // Add an array of tags to the post
        bb_vec *tags = entries->p[i].tags;
        if (tags != NULL) {
            JSON_Array *json_tags = json_value_get_array(json_value_init_array());
            for (int j = 0; j < bb_vec_count(tags); j++) {
                json_array_append_string(json_tags, (char*)bb_vec_get(tags, j));
            }
            json_object_set_value(json_value_get_object(tmp_post), "tags", json_array_get_wrapping_value(json_tags));
        }

        // Append post to the array of posts
        json_array_append_value(posts, tmp_post);
    }
    json_object_set_value(root_object, "posts", json_array_get_wrapping_value(posts));
}

void bb_archives_to_json(JSON_Object *root_object, Archives *a) {
    JSON_Array *archs = json_value_get_array(json_value_init_array());
    for (int i = 0; i < a->row_count; i++) {
        JSON_Value *val = json_value_init_object();
        json_object_set_string(json_value_get_object(val), "month_s", a->month_s[i]);
        json_object_set_number(json_value_get_object(val), "month", a->month[i]);
        json_object_set_number(json_value_get_object(val), "year", a->year[i]);
        json_object_set_number(json_value_get_object(val), "post_count", a->post_count[i]);
        json_array_append_value(archs, val);
    }
    json_object_set_value(root_object, "archives", json_array_get_wrapping_value(archs));
}