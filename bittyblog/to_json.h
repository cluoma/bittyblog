//
//  to_json.h
//  bittyblog
//
//  Created by Colin Luoma on 2018-07-25.
//  Copyright © 2018 Colin Luoma. All rights reserved.
//

#ifndef to_json_h
#define to_json_h

#include <parson.h>
#include "bittyblog.h"
#include "db_interface.h"

enum actions {
    VIEW = 0,
    EDIT,
    NEW,
};

int bb_default_to_json(JSON_Object*, bb_page_request*);
int bb_nav_buttons_to_json(JSON_Object *, bb_page_request *);
void bb_posts_to_json(JSON_Object *, bb_page_request *);
void bb_special_info_box_to_json(JSON_Object *root_object, bb_page_request *req);

/*
 * For Admin stuff
 */
void bb_users_to_json_admin(JSON_Object *root_object, bb_page_request *req, bb_vec * users, int action);
void bb_posts_to_json_admin(JSON_Object *root_object, bb_page_request *req, bb_vec *posts, bb_vec *users, int action);
void bb_pages_to_json_admin(JSON_Object *root_object, bb_page_request *req, int page_id, int action);

#endif