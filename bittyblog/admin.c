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
#include "to_json.h"
#include "bittyblog.h"
#include <libMagnum.h>
#include <parson.h>
#include <d_string.h>
#include <file.h>

#ifdef _FCGI
#include <fcgi_stdio.h>
#endif


void posts(JSON_Object *root_object, bb_page_request* req) {
    bb_vec * entries = db_admin_all_posts_preview();
    bb_posts_to_json_admin(root_object, req, entries, VIEW);
    bb_vec_free(entries);
}
void edit_post(JSON_Object *root_object, bb_page_request* req, int p_id) {
    bb_vec * entries = db_admin_id(p_id);
    bb_posts_to_json_admin(root_object, req, entries, EDIT);
    bb_vec_free(entries);
}
void new_post(JSON_Object *root_object, bb_page_request* req) {
    bb_posts_to_json_admin(root_object, req, NULL, NEW);
}

void fill_post(bb_page_request *req, Post *p) {
    // Post ID
    if (bb_cgi_get_var(req->q_vars, "post_id") != NULL) {
        p->p_id = bb_strtol(bb_cgi_get_var(req->q_vars, "post_id"), -1);
    } else {p->p_id = -1;}
    // Page ID
    if (bb_cgi_get_var(req->q_vars, "post_page") != NULL) {
        p->page_id = bb_strtol(bb_cgi_get_var(req->q_vars, "post_page"), -1);
    } else {p->page_id = -1;}
    // Visible
    if (bb_cgi_get_var(req->q_vars, "post_visible") != NULL) {
        p->visible = 1;
    } else {p->visible = 0;}
    // Title
    if (bb_cgi_get_var(req->q_vars, "post_title") != NULL) {
        p->title = bb_cgi_get_var(req->q_vars, "post_title");
    } else {p->title = NULL;}
    // Time
    if (bb_cgi_get_var(req->q_vars, "post_time") != NULL) {
        p->time_r = bb_strtol(bb_cgi_get_var(req->q_vars, "post_time"), time(NULL));
    } else {p->time_r = time(NULL);}
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
    // Tags
    if (bb_cgi_get_var(req->q_vars, "post_tags") != NULL) {
        p->tags = tokenize_tags(bb_cgi_get_var(req->q_vars, "post_tags"), ",");
    } else {p->tags = tokenize_tags("", ",");}
}

void pages(JSON_Object *root_object, bb_page_request* req) {
    bb_pages_to_json_admin(root_object, req, 0, VIEW);
}
void new_page(JSON_Object *root_object, bb_page_request* req) {
    bb_pages_to_json_admin(root_object, req, 0, NEW);
}
void edit_page(JSON_Object *root_object, bb_page_request* req, int page_id) {
    bb_pages_to_json_admin(root_object, req, page_id, EDIT);
}

void fill_page(bb_page_request *req, bb_page *p) {
    // Page ID
    if (bb_cgi_get_var(req->q_vars, "page_id") != NULL) {
        p->id = bb_strtol(bb_cgi_get_var(req->q_vars, "page_id"), -1);
    } else {p->id = -1;}
    // Page Style
    if (bb_cgi_get_var(req->q_vars, "page_style") != NULL) {
        p->style = bb_strtol(bb_cgi_get_var(req->q_vars, "page_style"), 0);
    } else {p->style = 0;}
    // Page Name Id
    if (bb_cgi_get_var(req->q_vars, "page_name_id") != NULL) {
        fprintf(stderr, "page_name_id: '%s'\n", bb_cgi_get_var(req->q_vars, "page_name_id"));
        p->id_name = bb_cgi_get_var(req->q_vars, "page_name_id");
    } else {p->id_name = NULL;}
    // Page Name
    if (bb_cgi_get_var(req->q_vars, "page_name") != NULL) {
        p->name = bb_cgi_get_var(req->q_vars, "page_name");
    } else {p->name = NULL;}
    // Tags
    if (bb_cgi_get_var(req->q_vars, "page_tags") != NULL) {
        p->tags = tokenize_tags(bb_cgi_get_var(req->q_vars, "page_tags"), ",");
    } else {p->tags = tokenize_tags("", ",");}
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
        }
        json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
    }
    bb_vec_free(image_list);
}

int main()
{

#ifdef _FCGI
    while(FCGI_Accept() >= 0 ) {
#endif

    // Init page request
    bb_page_request req;
    bb_init(&req, PARSE_GET | PARSE_POST);

    char *username  = bb_cgi_get_var(req.q_vars, "username");
    char *password  = bb_cgi_get_var(req.q_vars, "password");
    char *sid       = bb_cgi_get_var(req.q_vars, "sid");    // Session ID
    char *category  = bb_cgi_get_var(req.q_vars, "c");      // Category
    char *action    = bb_cgi_get_var(req.q_vars, "a");      // Action

    // Authenticate user and set session
    if (username != NULL && password != NULL && verify_user(username, password)) {
        char s [20];
        srand(time(NULL) + hash((unsigned char*)password) + hash((unsigned char*)password));
        snprintf(s, 20, "%x", rand());
        set_user_session(username, password, s);
        // Switch these depending if your browser supports status headers
        printf("Refresh: 0;url=%s?sid=%s\r\n\r\n", req.script_name, s);
        // printf("Status: 303 See Other\r\n");
        // printf("Location: %s?sid=%s\r\n\r\n", req.script_name, s);
        bb_free(&req);
        return 0;
    }
    
    // Verify user, otherwise show login form
    if (sid == NULL || !verify_session(sid)) {
        // Login form
        printf("Content-Type: text/html\r\n\r\n");
        printf("<h3>bittyblog Login</h3><br> <form action=\"%s\" method=\"POST\">\
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
                bb_vec_free(p.tags);
                printf("Successful :)<br>");
            } else if (strcmp(action, "new") == 0) {
                int r = db_new_post(&p);
                bb_vec_free(p.tags);
                if (r) {
                    printf("Successful :)<br>");
                } else {
                    printf("Unsuccessful :)<br>");
                }
            } else if (strcmp(action, "delete") == 0) {
                // Handle deletion of a post
                if (bb_cgi_get_var(req.q_vars, "post_id") != NULL) {
                    int p_id = (int)bb_strtol(bb_cgi_get_var(req.q_vars, "post_id"), -1);
                    db_delete_post(p_id);
                    bb_vec_free(p.tags);
                    fprintf(stderr, "Attempted delete of Post ID: %d\n", p_id);
                }
                printf("Successful :)<br>");
            }
        } else if (strcmp(category, "pages") == 0) {
            bb_page p;
            fill_page(&req, &p);
            if (strcmp(action, "update") == 0) {
                // Handle update of page
                db_update_page(&p);
                printf("Successful :)<br>");
            } else if (strcmp(action, "new") == 0) {
                // Handle creation of new page
                db_new_page(&p);
                printf("Successful :)<br>");
            } else if (strcmp(action, "delete") == 0) {
                // Handle deletion of page
                db_delete_page(p.id);
                printf("Successful :)<br>");
            }
            bb_vec_free(p.tags);
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
                if (data != NULL && data_len > 4 && is_image_file(data)) {
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
        { // Load list of posts
            if (action == NULL)
            {
                posts(root_object, &req);
            }
            if (action != NULL && strcmp(action, "new") == 0)
            {
                new_post(root_object, &req);
            }
        }
        else if (category != NULL && strcmp(category, "pages") == 0)
        { // Load list of pages
            if (action == NULL)
            {
                pages(root_object, &req);
            }
            else if (strcmp(action, "new") == 0)
            {
                new_page(root_object, &req);
            }
        }
        else if (category != NULL && strcmp(category, "media") == 0)
        { // List of images
            media_to_json(root_object, &req);
            json_object_set_string(root_object, "category_media", category);
        }
        else if (bb_cgi_get_var(req.q_vars, "p_id") != NULL)
        { // Load single post to edit
            edit_post(root_object, &req, (int)bb_strtol(bb_cgi_get_var(req.q_vars, "p_id"), -1));
        }
        else if (bb_cgi_get_var(req.q_vars, "page_id") != NULL)
        {
            // Load a single page to edit
            edit_page(root_object, &req, (int)bb_strtol(bb_cgi_get_var(req.q_vars, "page_id"), -1));
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

#ifdef _FCGI
    }
#endif

}
