//
//  admin.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include "main.h"
#include "cgi.h"
#include "db_interface.h"
#include "config.h"
#include "vec.h"
#include "bittyblog.h"
#include <libMagnum.h>
#include <parson.h>
#include <d_string.h>
#include <file.h>


void posts(JSON_Object *root_object, bb_page_request* req) {
    vector_p * entries = db_admin_all_posts_preview();

    JSON_Array *posts = json_value_get_array(json_value_init_array());
    for (int i = 0; i < entries->n; i++) {
        JSON_Value *tmp_post = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp_post), "p_id", entries->p[i].p_id);
        json_object_set_string(json_value_get_object(tmp_post), "page", entries->p[i].page);
        json_object_set_string(json_value_get_object(tmp_post), "title", entries->p[i].title);
        json_object_set_string(json_value_get_object(tmp_post), "time", entries->p[i].time);
        json_object_set_string(json_value_get_object(tmp_post), "byline", entries->p[i].byline);
        json_object_set_string(json_value_get_object(tmp_post), "thumbnail", entries->p[i].thumbnail);
        json_object_set_number(json_value_get_object(tmp_post), "visible", entries->p[i].visible);
        json_array_append_value(posts, tmp_post);
    }
    json_object_set_value(root_object, "posts", json_array_get_wrapping_value(posts));

    vector_p_free(entries);
}

void edit_post(JSON_Object *root_object, bb_page_request* req, int p_id) {
    vector_p * entries = db_admin_id(p_id);
    bb_vec * image_list = bb_image_list(req);

    // Add post to JSON
    if (entries->n > 0) {
        JSON_Array *posts = json_value_get_array(json_value_init_array());
        JSON_Value *tmp_post = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp_post), "p_id", entries->p[0].p_id);
        json_object_set_string(json_value_get_object(tmp_post), "page", entries->p[0].page);
        json_object_set_string(json_value_get_object(tmp_post), "title", entries->p[0].title);
        json_object_set_string(json_value_get_object(tmp_post), "text", entries->p[0].text);
        json_object_set_string(json_value_get_object(tmp_post), "time", entries->p[0].time);
        json_object_set_string(json_value_get_object(tmp_post), "byline", entries->p[0].byline);
        json_object_set_string(json_value_get_object(tmp_post), "extra", entries->p[0].extra);
        json_object_set_string(json_value_get_object(tmp_post), "thumbnail", entries->p[0].thumbnail);
        json_object_set_number(json_value_get_object(tmp_post), "visible", entries->p[0].visible);
        json_array_append_value(posts, tmp_post);
        json_object_set_value(root_object, "posts", json_array_get_wrapping_value(posts));
    }
    

    // Add image list to JSON
    if (image_list->count > 0) {
        JSON_Array *images = json_value_get_array(json_value_init_array());
        for (int i = 0; i < image_list->count; i++) {
                JSON_Value *tmp = json_value_init_object();
                json_object_set_string(json_value_get_object(tmp), "filename", (char*)bb_vec_get(image_list, i));
            if (strcmp((char*)bb_vec_get(image_list, i), entries->p[0].thumbnail) == 0) {
                json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
            } else {
                json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
            }
            json_array_append_value(images, tmp);
            free(bb_vec_get(image_list, i));
        }
        json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
    }
    bb_vec_free(image_list);
    free(image_list);

    // Add pages to JSON
    JSON_Array *pages = json_value_get_array(json_value_init_array());
    for (int i = 0; i < req->pages->count; i++) {
        JSON_Value *tmp = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp), "id", ((bb_page*)bb_vec_get(req->pages, i))->id);
        json_object_set_string(json_value_get_object(tmp), "id_name", ((bb_page*)bb_vec_get(req->pages, i))->id_name);
        json_object_set_string(json_value_get_object(tmp), "name", ((bb_page*)bb_vec_get(req->pages, i))->name);
        if (((bb_page*)bb_vec_get(req->pages, i))->id == entries->p[0].page_id) {
            json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
        } else {
            json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
        }
        json_array_append_value(pages, tmp);
    }
    json_object_set_value(root_object, "pages", json_array_get_wrapping_value(pages));

    vector_p_free(entries);
}

void new_post(JSON_Object *root_object, bb_page_request* req) {
    bb_vec * image_list = bb_image_list(req);

    // Add image list to JSON
    if (image_list->count > 0) {
        JSON_Array *images = json_value_get_array(json_value_init_array());
        for (int i = 0; i < image_list->count; i++) {
                JSON_Value *tmp = json_value_init_object();
                json_object_set_string(json_value_get_object(tmp), "filename", (char*)bb_vec_get(image_list, i));
                json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
            json_array_append_value(images, tmp);
            free(bb_vec_get(image_list, i));
        }
        json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
    }
    bb_vec_free(image_list);
    free(image_list);

    // Add pages to JSON
    JSON_Array *pages = json_value_get_array(json_value_init_array());
    for (int i = 0; i < req->pages->count; i++) {
        JSON_Value *tmp = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp), "id", ((bb_page*)bb_vec_get(req->pages, i))->id);
        json_object_set_string(json_value_get_object(tmp), "id_name", ((bb_page*)bb_vec_get(req->pages, i))->id_name);
        json_object_set_string(json_value_get_object(tmp), "name", ((bb_page*)bb_vec_get(req->pages, i))->name);
        json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
        json_array_append_value(pages, tmp);
    }
    json_object_set_value(root_object, "pages", json_array_get_wrapping_value(pages));
}

void fill_post(bb_page_request *req, Post *p) {
    int t;
    // Post ID
    if (bb_cgi_get_var(req->q_vars, "post_id") != NULL) {
        errno = 0;
        t = strtol(bb_cgi_get_var(req->q_vars, "post_id"), NULL, 10);
        if (errno) {
            p->p_id = -1;
        } else {
            p->p_id = t;
        }
    } else {p->p_id = -1;}
    // Page ID
    if (bb_cgi_get_var(req->q_vars, "post_page") != NULL) {
        errno = 0;
        t = strtol(bb_cgi_get_var(req->q_vars, "post_page"), NULL, 10);
        if (errno) {
            p->page_id = -1;
        } else {
            p->page_id = t;
        }
    } else {p->page_id = -1;}
    // Visible
    if (bb_cgi_get_var(req->q_vars, "post_visible") != NULL) {
        p->visible = 1;
    } else {p->visible = 0;}
    // Title
    if (bb_cgi_get_var(req->q_vars, "post_title") != NULL) {
        p->title = bb_cgi_get_var(req->q_vars, "post_title");
    } else {p->title = NULL;}
    // Text
    if (bb_cgi_get_var(req->q_vars, "post_text") != NULL) {
        p->text = bb_cgi_get_var(req->q_vars, "post_text");
    } else {p->text = NULL;}
    // Byline
    if (bb_cgi_get_var(req->q_vars, "post_byline") != NULL) {
        p->byline = bb_cgi_get_var(req->q_vars, "post_byline");
    } else {p->byline = NULL;}
    // Thumbnail
    if (bb_cgi_get_var(req->q_vars, "post_thumbnail") != NULL) {
        p->thumbnail = bb_cgi_get_var(req->q_vars, "post_thumbnail");
    } else {p->thumbnail = NULL;}
}

void media_to_json(JSON_Object *root_object, bb_page_request* req) {
    bb_vec * image_list = bb_image_list(req);
    // Add image list to JSON
    if (image_list->count > 0) {
        JSON_Array *images = json_value_get_array(json_value_init_array());
        for (int i = 0; i < image_list->count; i++) {
            JSON_Value *tmp = json_value_init_object();
            json_object_set_string(json_value_get_object(tmp), "filename", (char*)bb_vec_get(image_list, i));
            json_array_append_value(images, tmp);
            free(bb_vec_get(image_list, i));
        }
        json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
    }
    bb_vec_free(image_list);
    free(image_list);
}

int main()
{
    // Init page request
    bb_page_request req;
    bb_init(&req, PARSE_GET | PARSE_POST);

    char *username = bb_cgi_get_var(req.q_vars, "username");
    char *password = bb_cgi_get_var(req.q_vars, "password");
    char *sid = bb_cgi_get_var(req.q_vars, "sid");
    char *category = bb_cgi_get_var(req.q_vars, "c");
    char *action = bb_cgi_get_var(req.q_vars, "a");

    /* Authenticate user and set session */
    if (username != NULL && password != NULL && verify_user(username, password)) {
        char s [20];
        srand(time(NULL) + hash((unsigned char*)password) + hash((unsigned char*)password));
        snprintf(s, 20, "%x", rand());
        set_user_session(username, password, s);
        printf("Refresh: 0;url=%s?sid=%s\r\n\r\n", req.script_name, s);
        bb_free(&req);
        return 0;
    }
    
    // Verify user, otherwise show login form
    if (sid == NULL || !verify_session(sid)) {
        // Start form
        printf("Content-Type: text/html\r\n\r\n");
        printf(" <form action=\"%s\" method=\"GET\">\
        Username<br><input type=\"text\" name=\"username\" value=\"\"><br>\
        Password<br><input type=\"password\" name=\"password\" value=\"\"><br><br>\
        <input type=\"submit\" value=\"Submit\">\
        </form>", req.script_name);

        bb_free(&req);

        return 0;
    }

    // Initiate JSON objects
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    // Add SETTINGS variables to JSON
    json_object_set_string(root_object, "title", HTML_TITLE);
    json_object_set_string(root_object, "navbar_title", NAVBAR_TITLE);
    json_object_set_string(root_object, "owner", COPYRIGHT_OWNER);

    // Add REQUEST variables to JSON
    json_object_set_string(root_object, "script_name", req.script_name);

    // Add sid to JSON
    json_object_set_string(root_object, "sid", sid);

    // Add CURRENT YEAR to JSON
    time_t timeval;
    struct tm *tp = malloc(sizeof(struct tm));
    time (&timeval);
    gmtime_r(&timeval, tp);
    json_object_set_number(root_object, "current_year", tp->tm_year+1900);
    free(tp);

    /* Start of HTML outut */
    printf("Content-Type: text/html\r\n\r\n");

    if (strcmp(GET_ENV_VAR("REQUEST_METHOD"), "POST") == 0)
    {
        if (strcmp(category, "posts") == 0) {
            Post p;
            fill_post(&req, &p);
            if (strcmp(action, "update") == 0) {
                db_update_post(&p);
                printf("Successful :)<br>");
            } else if (strcmp(action, "new") == 0) {
                db_new_post(&p);
                printf("Successful :)<br>");
            } else if (strcmp(action, "delete") == 0) {
                // Handle deletion of a post
            }
        } else if (strcmp(category, "pages") == 0) {
            if (strcmp(action, "update") == 0) {
                // Handle update of page
            } else if (strcmp(action, "new") == 0) {
                // Handle creation of new page
            } else if (strcmp(action, "delete") == 0) {
                // Handle deletion of page
            }
        } else if (strcmp(category, "settings") == 0) {
            if (strcmp(action, "update") == 0) {
                // Handle update settings
            } else if (strcmp(action, "new") == 0) {
                // Handle new setting creation
            } else if (strcmp(action, "delete") == 0) {
                // Handle settings delete
            }
        } else if (strcmp(category, "media") == 0) {
            if (strcmp(action, "update") == 0) {
                // Handle update settings
            } else if (strcmp(action, "new") == 0) {
                char * data = bb_cgi_get_var(req.q_vars, "media_upload");
                long data_len = bb_cgi_get_var_len(req.q_vars, "media_upload");
                if (data != NULL && data_len > 0) {
                    char * filename = bb_cgi_get_var(req.q_vars, "media_upload.filename");
                    FILE *out;
                    char filepath[1024];
                    snprintf(filepath, sizeof(filepath)-1, "%s/%s", req.image_dir, filename);
                    out = fopen(filepath, "wb+");
                    fwrite(data, 1, data_len, out);
                    fclose(out);
                    printf("Successful :)<br>");
                } else {
                    printf("Unsuccessful :)<br>");
                }
            } else if (strcmp(action, "delete") == 0) {
                // Handle settings delete
                char * filename = bb_cgi_get_var(req.q_vars, "media_delete");
                if (filename != NULL) {
                    char filepath[1024];
                    snprintf(filepath, sizeof(filepath)-1, "%s/%s", req.image_dir, filename);
                    if (!remove(filepath)) {
                        printf("Successful :)<br>");
                    } else {
                        printf("Unsuccessful :)<br>");
                    }
                }
            }
        }
        
        // Cleanup
        json_value_free(root_value);
        bb_free(&req);

        return 0;
    }
    else if (strcmp(GET_ENV_VAR("REQUEST_METHOD"), "GET") == 0)
    {
        if (category != NULL && strcmp(category, "posts") == 0)
        {           // Load list of posts
            if (action == NULL) {
                posts(root_object, &req);
                json_object_set_string(root_object, "category_posts", category);
            }
            if (action != NULL && strcmp(action, "new") == 0) {
                new_post(root_object, &req);
                json_object_set_string(root_object, "category_new_posts", category);
            }
        }
        else if (category != NULL && strcmp(category, "pages") == 0)
        {    // Load list of pages

        }
        else if (category != NULL && strcmp(category, "media") == 0)
        {   // List of images
            media_to_json(root_object, &req);
            json_object_set_string(root_object, "category_media", category);
        }
        else if (bb_cgi_get_var(req.q_vars, "p_id") != NULL)
        {            // Load single post to edit
            edit_post(root_object, &req, atoi(bb_cgi_get_var(req.q_vars, "p_id")));
            json_object_set_boolean(root_object, "category_edit_posts", 1);
        }

    }

    char dir_base[1024];
    snprintf(dir_base, 1023, "%s/admin.m", TEMPLATE_PATH);
    DString *template = scan_file(dir_base);
    DString * out = d_string_new("");
    magnum_populate_from_json(template, root_value, out, ".", NULL);

    // Start of HTML output
    printf("%s", out->str);

    d_string_free(template, 1);
    d_string_free(out, 1);

    json_value_free(root_value);

    bb_free(&req);
}
