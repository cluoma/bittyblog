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

int bb_default_to_json(JSON_Object*, bb_page_request*);
int bb_nav_buttons_to_json(JSON_Object *, bb_page_request *);
void bb_posts_to_json(JSON_Object *, bb_page_request *, int format);
void bb_archives_to_json(JSON_Object *, Archives *);

#endif