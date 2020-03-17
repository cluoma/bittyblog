//
//  mod_archives.h
//  bittyblog
//
//  Created by Colin Luoma on 2018-07-25.
//  Copyright © 2018 Colin Luoma. All rights reserved.
//

#ifndef mod_archives_h
#define mod_archives_h

#include "bittyblog.h"
#include <parson.h>

#define ARCHIVES "SELECT strftime('%m',time,'unixepoch') AS month, strftime('%Y',time,'unixepoch') AS year, COUNT(*) AS num_posts \
FROM posts p \
INNER JOIN (SELECT * FROM pages WHERE name_id = 'blog') a ON p.page_id = a.id \
WHERE p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
GROUP BY month, year \
ORDER BY time DESC"

// Archives holds a list of months, years, and post counts
// for those archives sidebar element
typedef struct {
    char **month_s;
    int *month;
    int *year;
    int *post_count;
    int row_count;
} Archives;

// Required functions
Archives mod_archives_load(db_conn* c);
void mod_archives_free(Archives *archives);
void mod_archives_to_json(JSON_Object *root_object, Archives *a);

#endif