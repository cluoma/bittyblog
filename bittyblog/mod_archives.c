//
//  mod_archives.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include "mod_archives.h"
#include "bittyblog.h"
#include <sqlite3.h>

// String months
char * ENG_MONTH[] = {
    "",
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

Archives mod_archives_load(db_conn* c) {

    Archives archives;
    archives.month_s = NULL;
    archives.month = NULL;
    archives.year = NULL;
    archives.post_count = NULL;
    archives.row_count = 0;

    // Open db
    sqlite3 *db = c->con;
    if (db == NULL)
        return archives;

    sqlite3_stmt *results;
    char *sql = ARCHIVES;

    int rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);

    // Bind input data to sql statement
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to update table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return archives;
    }

    // Execute statement
    while(sqlite3_step(results) == SQLITE_ROW)
    {
        // Realloc memory for a new row
        archives.month_s = (char **) realloc(archives.month_s, sizeof(char *)*(1+archives.row_count));
        archives.month = (int *) realloc(archives.month, sizeof(int)*(1+archives.row_count));
        archives.year = (int *) realloc(archives.year, sizeof(int)*(1+archives.row_count));
        archives.post_count = (int *) realloc(archives.post_count, sizeof(int)*(1+archives.row_count));

        archives.month_s[archives.row_count]    = ENG_MONTH[sqlite3_column_int(results, 0)];
        archives.month[archives.row_count]      = sqlite3_column_int(results, 0);
        archives.year[archives.row_count]       = sqlite3_column_int(results, 1);
        archives.post_count[archives.row_count] = sqlite3_column_int(results, 2);
        archives.row_count++;
    }
    sqlite3_finalize(results);

    return archives;
}

void mod_archives_free(Archives *archives)
{
    if( archives->month_s != NULL)
        free(archives->month_s);
    if( archives->month != NULL )
        free(archives->month);
    if( archives->year != NULL )
        free(archives->year);
    if( archives->post_count != NULL)
        free(archives->post_count);
}

void mod_archives_to_json(JSON_Object *root_object, Archives *a)
{
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