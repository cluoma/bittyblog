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

int bb_default_to_json(JSON_Object *root_object, bb_page_request *req)
{
    // Add SETTINGS variables to JSON
    json_object_set_string(root_object, "title", req->html_title);
    json_object_set_string(root_object, "navbar_title", req->navbar_title);
    json_object_set_string(root_object, "owner", req->copyright_owner);

    // Add REQUEST variables to JSON
    json_object_set_string(root_object, "script_name", req->script_name);
    json_object_set_string(root_object, "page_name", req->page_name);

    // Set rewrite flag
    if (req->rewrite)
        json_object_set_boolean(root_object, "rewrite", 1);

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

int bb_nav_buttons_to_json(JSON_Object *root_object, bb_page_request *req)
{
    // Query string without the start variable
    char *qs = bb_cgi_query_string_wo(req->q_vars, "start");
    json_object_set_string(root_object, "query_string_wo_start", qs);
    free(qs);
    
    long start = bb_strtol(bb_cgi_get_var(req->q_vars, "start"), 0);
    // Older
    if ( start+POSTS_PER_PAGE < req->total_post_count ) {
        char out[30];
        sprintf(out, "%ld", start+POSTS_PER_PAGE);
        json_object_dotset_string(root_object, "nav_buttons.older", out);
    }
    // Newer
    if ( start > 0 ) {
        char out[30];
        sprintf(out, "%ld", start-POSTS_PER_PAGE);
        json_object_dotset_string(root_object, "nav_buttons.newer", out);
    }

    return 1;
}

void bb_posts_to_json(JSON_Object *root_object, bb_page_request *req)
{
    bb_vec *entries = req->posts;

    JSON_Array *posts = json_value_get_array(json_value_init_array());
    for (int i = 0; i < bb_vec_count(entries); i++) {
        bb_post *p = (bb_post*)bb_vec_get(entries, i);
        JSON_Value *tmp_post = json_value_init_object();
        json_object_set_number(json_value_get_object(tmp_post), "p_id", p->p_id);
        json_object_set_string(json_value_get_object(tmp_post), "page", p->page);
        json_object_set_string(json_value_get_object(tmp_post), "title", p->title);
        json_object_set_string(json_value_get_object(tmp_post), "byline", p->byline);
        json_object_set_string(json_value_get_object(tmp_post), "text", p->text);
        json_object_set_string(json_value_get_object(tmp_post), "extra", p->extra);

        // Add different times
        char time_s[70];
        struct tm *time = malloc(sizeof(struct tm));
        gmtime_r(&(p->time_r), time);
        // Standard time for blog posts
        strftime(time_s, 70, "%Y-%m-%d %H:%M:%S", time);
        json_object_set_string(json_value_get_object(tmp_post), "time", time_s);
        // Special time for RSS
        strftime(time_s, 70, "%a, %d %b %Y %H:%M:%S %z", time);
        json_object_set_string(json_value_get_object(tmp_post), "time_rss", time_s);
        free(time);

        // Set default thumbnail if we didn't get one from the database
        if (p->thumbnail == NULL || strcmp(p->thumbnail, "") == 0) {
            json_object_set_string(json_value_get_object(tmp_post), "thumbnail", "bb_default.thumbnail.jpg");
        } else {
            json_object_set_string(json_value_get_object(tmp_post), "thumbnail", p->thumbnail);
        }

        // Add an array of tags to the post
        bb_vec *tags = p->tags;
        if (tags != NULL) {
            JSON_Array *json_tags = json_value_get_array(json_value_init_array());
            for (int j = 0; j < bb_vec_count(tags); j++) {
                json_array_append_string(json_tags, (char*)bb_vec_get(tags, j));
            }
            json_object_set_value(json_value_get_object(tmp_post), "tags", json_array_get_wrapping_value(json_tags));
        }

        // Add user data
        json_object_set_string(json_value_get_object(tmp_post), "user_name_id", p->user.name_id);
        json_object_set_string(json_value_get_object(tmp_post), "user_name", p->user.name);
        json_object_set_string(json_value_get_object(tmp_post), "user_about", p->user.about);
        json_object_set_string(json_value_get_object(tmp_post), "user_thumbnail", p->user.thumbnail);

        // Append post to the array of posts
        json_array_append_value(posts, tmp_post);
    }
    json_object_set_value(root_object, "posts", json_array_get_wrapping_value(posts));
}

void bb_special_info_box_to_json(JSON_Object *root_object, bb_page_request *req)
{
    JSON_Value *special_info_box_value = json_value_init_object();
    JSON_Object *special_info_box_object = json_value_get_object(special_info_box_value);
    // Add search query string variable to json (if we searched)
    char *search = bb_cgi_get_var(req->q_vars, "search");
    if (search) {
        json_object_set_string(special_info_box_object, "search", search);
        goto end;
    }

    // Add tag name to json if needed (if a tag was supplied)
    char *tag = bb_cgi_get_var(req->q_vars, "tag");
    if (tag) {
        json_object_set_string(special_info_box_object, "tag", tag);
        goto end;
    }

    char *author = bb_cgi_get_var(req->q_vars, "author");
    if (author) {
        bb_vec *user_v = db_author(req->dbcon, author);
        if (bb_vec_count(user_v) > 0) {
            bb_user *user = (bb_user *)bb_vec_get(user_v, 0);
            json_object_set_boolean(special_info_box_object, "author", 1);
            json_object_set_string(special_info_box_object, "user_name", user->name);
            json_object_set_string(special_info_box_object, "user_thumbnail", user->thumbnail);
            json_object_set_string(special_info_box_object, "user_about", user->about);
        }
        bb_vec_free(user_v);
        goto end;
    }
end:
    json_object_set_value(root_object, "special_info_box", json_object_get_wrapping_value(special_info_box_object));
}

/*
 * For admin page
 */
void bb_posts_to_json_admin(JSON_Object *root_object, bb_page_request *req, bb_vec *posts, bb_vec *users, int action)
{   
    switch(action) {
        case VIEW:
            json_object_set_boolean(root_object, "category_posts", 1);
            break;
        case EDIT:
            json_object_set_boolean(root_object, "category_edit_posts", 1);
            break;
        case NEW:
            json_object_set_boolean(root_object, "category_new_posts", 1);
            break;
        default:
            break;
    }

    if (posts != NULL && bb_vec_count(posts) > 0)
    {
        JSON_Array *json_posts = json_value_get_array(json_value_init_array());
        for (int i = 0; i < bb_vec_count(posts); i++)
        {
            bb_post *p = (bb_post *)bb_vec_get(posts, i);

            JSON_Value *tmp_post = json_value_init_object();
            json_object_set_number(json_value_get_object(tmp_post), "p_id", p->p_id);
            json_object_set_string(json_value_get_object(tmp_post), "page", p->page);
            json_object_set_string(json_value_get_object(tmp_post), "title", p->title);
            json_object_set_string(json_value_get_object(tmp_post), "time", p->time);
            json_object_set_string(json_value_get_object(tmp_post), "byline", p->byline);
            json_object_set_string(json_value_get_object(tmp_post), "thumbnail", p->thumbnail);
            json_object_set_number(json_value_get_object(tmp_post), "visible", p->visible);

            // Add user data
            json_object_set_string(json_value_get_object(tmp_post), "user_name_id", p->user.name_id);
            json_object_set_string(json_value_get_object(tmp_post), "user_name", p->user.name);

            // Only add text for editting
            // Add text twice, once for text box and once for the preview
            if (action == EDIT)
            {
                json_object_set_string(json_value_get_object(tmp_post), "text", p->markdown);
                json_object_set_string(json_value_get_object(tmp_post), "text_formatted", p->text);

                char time_r[25];
                sprintf(time_r, "%ld", p->time_r);
                json_object_set_string(json_value_get_object(tmp_post), "time_r", time_r);
                // Add an array of tags to the post
                bb_vec *tags = p->tags;
                if (tags != NULL)
                {
                    JSON_Array *json_tags = json_value_get_array(json_value_init_array());
                    for (int j = 0; j < bb_vec_count(tags); j++)
                    {
                        json_array_append_string(json_tags, (char *)bb_vec_get(tags, j));
                    }
                    json_object_set_value(json_value_get_object(tmp_post), "tags", json_array_get_wrapping_value(json_tags));
                }
            }

            if (action == EDIT)
            {
                // Add image list to JSON
                bb_vec *image_list = bb_image_list(req, THUMBNAILS);
                if (image_list->count > 0)
                {
                    JSON_Array *images = json_value_get_array(json_value_init_array());
                    for (int i = 0; i < image_list->count; i++)
                    {
                        JSON_Value *tmp = json_value_init_object();
                        json_object_set_string(json_value_get_object(tmp), "filename", (char *)bb_vec_get(image_list, i));
                        if (p->thumbnail != NULL && strcmp((char *)bb_vec_get(image_list, i), p->thumbnail) == 0) {
                            json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
                        } else {
                            json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
                        }
                        json_array_append_value(images, tmp);
                    }
                    json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
                }
                bb_vec_free(image_list);

                // Add pages to JSON
                JSON_Array *pages = json_value_get_array(json_value_init_array());
                for (int i = 0; i < req->pages->count; i++)
                {
                    JSON_Value *tmp = json_value_init_object();
                    json_object_set_number(json_value_get_object(tmp), "id", ((bb_page *)bb_vec_get(req->pages, i))->id);
                    json_object_set_string(json_value_get_object(tmp), "id_name", ((bb_page *)bb_vec_get(req->pages, i))->id_name);
                    json_object_set_string(json_value_get_object(tmp), "name", ((bb_page *)bb_vec_get(req->pages, i))->name);
                    if (((bb_page *)bb_vec_get(req->pages, i))->id == p->page_id) {
                        json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
                    } else {
                        json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
                    }
                    json_array_append_value(pages, tmp);
                }
                json_object_set_value(root_object, "pages", json_array_get_wrapping_value(pages));

                // Add users to JSON
                JSON_Array *user_list = json_value_get_array(json_value_init_array());
                for (int i = 0; i < bb_vec_count(users); i++)
                {
                    JSON_Value *tmp = json_value_init_object();
                    json_object_set_number(json_value_get_object(tmp), "id", ((bb_user *)bb_vec_get(users, i))->id);
                    json_object_set_string(json_value_get_object(tmp), "name_id", ((bb_user *)bb_vec_get(users, i))->name_id);
                    json_object_set_string(json_value_get_object(tmp), "name", ((bb_user *)bb_vec_get(users, i))->name);
                    if (((bb_user *)bb_vec_get(users, i))->id == p->user.id) {
                        json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
                    } else {
                        json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
                    }
                    json_array_append_value(user_list, tmp);
                }
                json_object_set_value(root_object, "users", json_array_get_wrapping_value(user_list));
            }
            json_array_append_value(json_posts, tmp_post);
        }
        json_object_set_value(root_object, "posts", json_array_get_wrapping_value(json_posts));
    }
    else
    {
        // Add image list to JSON
        bb_vec *image_list = bb_image_list(req, THUMBNAILS);
        if (image_list->count > 0)
        {
            JSON_Array *images = json_value_get_array(json_value_init_array());
            for (int i = 0; i < image_list->count; i++)
            {
                JSON_Value *tmp = json_value_init_object();
                json_object_set_string(json_value_get_object(tmp), "filename", (char *)bb_vec_get(image_list, i));
                json_array_append_value(images, tmp);
            }
            json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
        }
        bb_vec_free(image_list);

        // Add pages to JSON
        JSON_Array *pages = json_value_get_array(json_value_init_array());
        for (int i = 0; i < req->pages->count; i++)
        {
            JSON_Value *tmp = json_value_init_object();
            json_object_set_number(json_value_get_object(tmp), "id", ((bb_page *)bb_vec_get(req->pages, i))->id);
            json_object_set_string(json_value_get_object(tmp), "id_name", ((bb_page *)bb_vec_get(req->pages, i))->id_name);
            json_object_set_string(json_value_get_object(tmp), "name", ((bb_page *)bb_vec_get(req->pages, i))->name);
            json_array_append_value(pages, tmp);
        }
        json_object_set_value(root_object, "pages", json_array_get_wrapping_value(pages));

        // Add users to JSON
        JSON_Array *user_list = json_value_get_array(json_value_init_array());
        for (int i = 0; i < bb_vec_count(users); i++)
        {
            JSON_Value *tmp = json_value_init_object();
            json_object_set_number(json_value_get_object(tmp), "id", ((bb_user *)bb_vec_get(users, i))->id);
            json_object_set_string(json_value_get_object(tmp), "name_id", ((bb_user *)bb_vec_get(users, i))->name_id);
            json_object_set_string(json_value_get_object(tmp), "name", ((bb_user *)bb_vec_get(users, i))->name);
            json_array_append_value(user_list, tmp);
        }
        json_object_set_value(root_object, "users", json_array_get_wrapping_value(user_list));
    }
}

void bb_pages_to_json_admin(JSON_Object *root_object, bb_page_request *req, int page_id, int action)
{
    switch(action) {
        case VIEW:
            json_object_set_boolean(root_object, "category_pages", 1);
            break;
        case EDIT:
            json_object_set_boolean(root_object, "category_edit_pages", 1);
            break;
        case NEW:
            json_object_set_boolean(root_object, "category_new_pages", 1);
            break;
        default:
            break;
    }

    // Add pages to JSON
    if (action == VIEW || action == EDIT)
    {
        JSON_Array *json_pages = json_value_get_array(json_value_init_array());
        for (int i = 0; i < req->pages->count; i++)
        {
            // Only add the page we want to edit if this is an edit
            if (action == EDIT && ((bb_page *)bb_vec_get(req->pages, i))->id != page_id)
                continue;
            
            JSON_Value *tmp = json_value_init_object();
            json_object_set_number(json_value_get_object(tmp), "id", ((bb_page *)bb_vec_get(req->pages, i))->id);
            json_object_set_string(json_value_get_object(tmp), "id_name", ((bb_page *)bb_vec_get(req->pages, i))->id_name);
            json_object_set_string(json_value_get_object(tmp), "name", ((bb_page *)bb_vec_get(req->pages, i))->name);
            char style[25];
            sprintf(style, "%d", ((bb_page *)bb_vec_get(req->pages, i))->style);
            json_object_set_string(json_value_get_object(tmp), "style", style);
            json_object_set_string(json_value_get_object(tmp), "style_name", style_template[((bb_page *)bb_vec_get(req->pages, i))->style]);
            // Add an array of tags to the page
            bb_vec *tags = ((bb_page *)bb_vec_get(req->pages, i))->tags;
            if (tags != NULL)
            {
                JSON_Array *json_tags = json_value_get_array(json_value_init_array());
                for (int j = 0; j < bb_vec_count(tags); j++)
                {
                    json_array_append_string(json_tags, (char *)bb_vec_get(tags, j));
                }
                json_object_set_value(json_value_get_object(tmp), "tags", json_array_get_wrapping_value(json_tags));
            }

            // Add arrary of Styles to page
            if (action == EDIT)
            {
                JSON_Array *json_styles = json_value_get_array(json_value_init_array());
                for (int j = 0; j < STYLE_LAST; j++)
                {
                    JSON_Value *tmp2 = json_value_init_object();
                    char style[25];
                    sprintf(style, "%d", j);
                    json_object_set_string(json_value_get_object(tmp2), "style", style);
                    json_object_set_string(json_value_get_object(tmp2), "style_name", style_template[j]);
                    if (((bb_page *)bb_vec_get(req->pages, i))->style == j)
                    {
                        json_object_set_boolean(json_value_get_object(tmp2), "selected", 1);
                    }
                    json_array_append_value(json_styles, tmp2);
                }
                json_object_set_value(json_value_get_object(tmp), "styles", json_array_get_wrapping_value(json_styles));
            }
            json_array_append_value(json_pages, tmp);
        }
        json_object_set_value(root_object, "pages", json_array_get_wrapping_value(json_pages));
    }

    if (action == NEW)
    {
        // Add arrary of Styles to page
        JSON_Array *json_styles = json_value_get_array(json_value_init_array());
        for (int j = 0; j < STYLE_LAST; j++)
        {
            JSON_Value *tmp2 = json_value_init_object();
            char style[25];
            sprintf(style, "%d", j);
            json_object_set_string(json_value_get_object(tmp2), "style", style);
            json_object_set_string(json_value_get_object(tmp2), "style_name", style_template[j]);
            json_array_append_value(json_styles, tmp2);
        }
        json_object_set_value(root_object, "styles", json_array_get_wrapping_value(json_styles));
    }
}

void bb_users_to_json_admin(JSON_Object *root_object, bb_page_request *req, bb_vec * users, int action)
{
    switch(action) {
        case VIEW:
            json_object_set_boolean(root_object, "category_users", 1);
            break;
        case EDIT:
            json_object_set_boolean(root_object, "category_edit_users", 1);
            break;
        case NEW:
            json_object_set_boolean(root_object, "category_new_users", 1);
            break;
        default:
            break;
    }

    JSON_Array *json_users = json_value_get_array(json_value_init_array());

    // Add pages to JSON
    if (action == VIEW || action == EDIT)
    {
        for (int i = 0; i < bb_vec_count(users); i++)
        {   
            bb_user *user = (bb_user *)bb_vec_get(users, i);

            JSON_Value *tmp = json_value_init_object();
            json_object_set_number(json_value_get_object(tmp), "id", user->id);
            json_object_set_string(json_value_get_object(tmp), "email", user->email);
            json_object_set_string(json_value_get_object(tmp), "name_id", user->name_id);
            json_object_set_string(json_value_get_object(tmp), "name", user->name);
            json_object_set_string(json_value_get_object(tmp), "about", user->about);

            // Add arrary of Styles to page
            if (action == EDIT)
            {
                // Add thumbnail list to JSON
                bb_vec *image_list = bb_image_list(req, THUMBNAILS);
                if (image_list->count > 0)
                {
                    JSON_Array *images = json_value_get_array(json_value_init_array());
                    for (int i = 0; i < image_list->count; i++)
                    {
                        JSON_Value *tmp = json_value_init_object();
                        json_object_set_string(json_value_get_object(tmp), "filename", (char *)bb_vec_get(image_list, i));
                        if (user->thumbnail != NULL && strcmp((char *)bb_vec_get(image_list, i), user->thumbnail) == 0) {
                            json_object_set_boolean(json_value_get_object(tmp), "selected", 1);
                        } else {
                            json_object_set_boolean(json_value_get_object(tmp), "selected", 0);
                        }
                        json_array_append_value(images, tmp);
                    }
                    json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
                }
                bb_vec_free(image_list);
            }
            json_array_append_value(json_users, tmp);
        }
    }
    else if (action == NEW)
    {
        JSON_Value *tmp = json_value_init_object();

        // Add thumbnail list to JSON
        bb_vec *image_list = bb_image_list(req, THUMBNAILS);
        if (bb_vec_count(image_list) > 0)
        {
            JSON_Array *images = json_value_get_array(json_value_init_array());
            for (int i = 0; i < bb_vec_count(image_list); i++)
            {
                JSON_Value *tmp = json_value_init_object();
                json_object_set_string(json_value_get_object(tmp), "filename", (char *)bb_vec_get(image_list, i));
                json_array_append_value(images, tmp);
            }
            json_object_set_value(root_object, "images", json_array_get_wrapping_value(images));
        }
        bb_vec_free(image_list);
        json_array_append_value(json_users, tmp);
    }
    json_object_set_value(root_object, "users", json_array_get_wrapping_value(json_users));
}