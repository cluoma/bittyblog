//
//  db_interface.c
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#include "db_interface.h"
#include "bittyblog.h"
#include "config.h"

#define COPY_SQLITE3_STRING(X, Y, Z) \
    if (sqlite3_column_type((Y), (Z)) == SQLITE_TEXT) { \
        (X) = calloc(sqlite3_column_bytes((Y), (Z)) + 1, 1); \
        memcpy((X), (char *)sqlite3_column_text((Y), (Z)), sqlite3_column_bytes((Y), (Z))); \
    }
#define COPY_SQLITE3_INT(X, Y, Z) \
    if (sqlite3_column_type((Y), (Z)) == SQLITE_INTEGER) { \
        (X) = sqlite3_column_int((Y), (Z)); \
    }
#define COPY_SQLITE3_INT64(X, Y, Z) \
    if (sqlite3_column_type((Y), (Z)) == SQLITE_INTEGER) { \
        (X) = sqlite3_column_int((Y), (Z)); \
    }


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

// Helper function to open SQLite3 DB
sqlite3 *open_database()
{
    sqlite3 *db;

    int err;
    if( (err = sqlite3_open(DB_PATH, &db)) )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    else
    {
        return db;
    }
}


/*
 *  Execute query function takes arguments to prepare a statement then
 *  executes the query. A callback must be provided to deal with the results.
 * 
 *  If NULL is supplied as the db argument, then a connection is opened
 *  to execute the given query.
 * 
 *  If callback is NULL, the query will be executed once using sqlite3_step
 * 
 *  If callback is NULL, and data is NOT NULL, first column of first row of results
 *  will be stored as an int in data.
 * 
 *  Arguments will be bound to the query in order, according to the types string
 *  types is a string consisting of:
 *  s: string char*
 *  i: int
 *  eg. "ssis" will bind arguments as string, string, int, string
 * 
 *  Callbacks MUST return 0, otherwise resources will be freed and NULL returned.
 */
int execute_query(sqlite3* db, int (*callback)(sqlite3_stmt*, void*), void* data,
                  const char* query, const char* types, ...) {
    sqlite3_stmt *results;
    va_list ap;
    int param_count;
    char type;
    int db_con_supplied = db == NULL ? 0 : 1;

    if (db == NULL && ( db = open_database() ) == NULL) {
        return 0;
    }

    if (sqlite3_prepare_v2(db, query, -1, &results, 0) != SQLITE_OK) {
        fprintf(stderr, "Could not prepare query: %s\n", sqlite3_errmsg(db));
        if (!db_con_supplied) sqlite3_close(db);
        return 0;
    }

    // Bind all arguments to prepared query
    param_count = sqlite3_bind_parameter_count(results);
    va_start(ap, types);
    int i = 1;
    while ((type = *types++) && i <= param_count) {
        switch (type) {

            case 's':
            if (sqlite3_bind_text(results, i, va_arg(ap, char*), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
                fprintf(stderr, "Could not bind string parameter: %s\n", sqlite3_errmsg(db));
                va_end(ap);
                goto bad;
            }
            break;

            case 'i':
            if (sqlite3_bind_int64(results, i, va_arg(ap, long)) != SQLITE_OK) {
                fprintf(stderr, "Could not bind int parameter: %s\n", sqlite3_errmsg(db));
                va_end(ap);
                goto bad;
            }
            break;

            default:
                fprintf(stderr, "Incorrect character found in type string: %c\n", type);
                va_end(ap);
                goto bad;
        }
        i++;
    }
    va_end(ap);

    // Call callback function to deal with query results
    if (callback) {
        if ((*callback)(results, data)) {
            fprintf(stderr, "SQL Execute Callback return bad: %s\n", sqlite3_errmsg(db));
            goto bad;
        }
    } else {
        int rv = sqlite3_step(results);
        if (rv == SQLITE_ROW && data != NULL) {
            *(int*)data = sqlite3_column_int(results, 0);
        } else if (rv == SQLITE_DONE) {

        } else {
            fprintf(stderr, "SQL Execute Step return bad: %s\n", sqlite3_errmsg(db));
            goto bad;
        }
    }

    sqlite3_finalize(results);
    if (!db_con_supplied) sqlite3_close(db);
    return 1;

bad:
    sqlite3_finalize(results);
    if (!db_con_supplied) sqlite3_close(db);
    return 0;
}
// Set the time of the last database change
int db_update_time(sqlite3 *db)
{
    if (execute_query(db, NULL, NULL, "DELETE FROM last_update", "") &&
        execute_query(db, NULL, NULL, "INSERT INTO last_update (time) VALUES(CAST(strftime('%s', 'now') AS INT))", "") )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
// Open data and begin a transaction so that it can be rolled back
sqlite3 *open_database_transaction()
{
    // Connect to DB
    sqlite3 *db = open_database();
    if (db == NULL) {
        fprintf(stderr, "Failed to open DB connection.\n");
        return NULL;
    }

    // Begin transaction so we can rollback if things go wrong
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Couldn't begin transaction.\n");
        sqlite3_close(db);
        return NULL;
    }

    return db;
}
// Update update timestamp, commit transaction, and close database
int close_database_transaction(sqlite3 *db)
{   
    // Update timestamp
    int rc = db_update_time(db);
    if (!rc) {
        fprintf(stderr, "Couldn't update update timestamp\n");
        sqlite3_close(db);
        return 0;
    }

    // Commit transaction
    if (sqlite3_exec(db, "COMMIT;", 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Couldn't commit transaction.\n");
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);

    return 1;
}

// Load post data from database
int load_posts_cb(sqlite3_stmt *results, void* data) {
    bb_vec *vp = (bb_vec*)data;

    while(sqlite3_step(results) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        bb_post *post = malloc(sizeof(bb_post));
        if (post == NULL) return 1;
        if (bb_post_init(post) != 0)
            return 1;

        int col_count = sqlite3_column_count(results);
        for (int i = 0; i < col_count; i++)
        {
            const char* col_name = sqlite3_column_name(results, i);
            if (strcmp(col_name, "title") == 0) {
                COPY_SQLITE3_STRING(post->title, results, i);
            } else if (strcmp(col_name, "id") == 0) {
                COPY_SQLITE3_INT(post->p_id, results, i);
            } else if (strcmp(col_name, "page_id") == 0) {
                COPY_SQLITE3_INT(post->page_id, results, i);
            } else if (strcmp(col_name, "page") == 0) {
                COPY_SQLITE3_STRING(post->page, results, i);
            } else if (strcmp(col_name, "text") == 0) {
                COPY_SQLITE3_STRING(post->text, results, i);
            } else if (strcmp(col_name, "time") == 0) {
                COPY_SQLITE3_STRING(post->time, results, i);
            } else if (strcmp(col_name, "time_r") == 0) {
                COPY_SQLITE3_INT64(post->time_r, results, i);
            } else if (strcmp(col_name, "byline") == 0) {
                COPY_SQLITE3_STRING(post->byline, results, i);
            } else if (strcmp(col_name, "thumbnail") == 0) {
                COPY_SQLITE3_STRING(post->thumbnail, results, i);
            } else if (strcmp(col_name, "visible") == 0) {
                COPY_SQLITE3_INT(post->visible, results, i);
            }
            else if (strcmp(col_name, "tags") == 0 &&
                     sqlite3_column_type(results, i) == SQLITE_TEXT) {
                const char *tags = (char *) sqlite3_column_text(results, i);
                post->tags = tokenize_tags(tags, ",");
            } else if (strcmp(col_name, "user_id") == 0) {
                COPY_SQLITE3_INT(post->user.id, results, i);
            } else if (strcmp(col_name, "user_name_id") == 0) {
                COPY_SQLITE3_STRING(post->user.name_id, results, i);
            } else if (strcmp(col_name, "user_name") == 0) {
                COPY_SQLITE3_STRING(post->user.name, results, i);
            } else if (strcmp(col_name, "user_about") == 0) {
                COPY_SQLITE3_STRING(post->user.about, results, i);
            } else if (strcmp(col_name, "user_thumbnail") == 0) {
                COPY_SQLITE3_STRING(post->user.thumbnail, results, i);
            }
        }
        bb_vec_add(vp, post);
    }
    return 0;
}

void vec_post_free(bb_post *p)
{
    bb_post_free(p);
    free(p);
}
bb_vec * db_ntag(char* tag, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_TAG_QUERY,
                "sii", tag, count, offset);
    return all_posts;
}
bb_vec * db_nauthor(char* name_id, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_AUTHOR_QUERY,
                "sii", name_id, count, offset);
    return all_posts;
}
bb_vec * db_search(char* page_name_id, char *keyword)
{   
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                SEARCH_QUERY,
                "ss", page_name_id, keyword);
    return all_posts;
}
bb_vec * db_nsearch(char* page_name_id, char *keyword, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_SEARCH_QUERY,
                "ssii", page_name_id, keyword, count, offset);
    return all_posts;
}
bb_vec * db_monthyear(char* page_name_id, int month, int year)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                MONTH_YEAR_QUERY,
                "sii", page_name_id,  month, year);
    return all_posts;
}
bb_vec * db_nposts(char* page_name_id, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_POSTS_QUERY,
                "ssii", page_name_id, page_name_id,  count, offset);
    return all_posts;
}
bb_vec * db_id(int id)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                POST_ID_QUERY,
                "i", id);
    return all_posts;
}

// Get the number of posts
int db_count_cb(sqlite3_stmt* st, void* a) {
    while(sqlite3_step(st) == SQLITE_ROW)
    {
        *(int*)a = sqlite3_column_int(st, 0);
        break;
    }
    return 0;
}
int db_count(char* page_name_id)
{
    int success = 0;
    int count = 0;

    success = execute_query(NULL, db_count_cb, &count,
        N_POSTS_COUNT_QUERY,
        "ss", page_name_id, page_name_id);

    if (success && count)
        return count;
    
    return 0;
}
int db_search_count(char* page_name_id, char* keyword) {
    int success = 0;
    int count = 0;

    success = execute_query(NULL, db_count_cb, &count,
        SEARCH_COUNT_QUERY,
        "ss", page_name_id, keyword);

    if (success && count)
        return count;
    
    return 0;
}
int db_tag_count(char* tag) {
    int success = 0;
    int count = 0;

    success = execute_query(NULL, db_count_cb, &count,
        TAG_COUNT_QUERY,
        "s", tag);

    if (success && count)
        return count;
    
    return 0;
}
int db_author_count(char* name_id) {
    int success = 0;
    int count = 0;

    success = execute_query(NULL, db_count_cb, &count,
        N_AUTHOR_COUNT_QUERY,
        "s", name_id);

    if (success && count)
        return count;
    
    return 0;
}

/*
 * Get a list of pages from the database
 */
int db_pages_cb(sqlite3_stmt* st, void* a) {
    bb_vec *pages = (bb_vec*)a;
    while(sqlite3_step(st) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        bb_page *page = malloc(sizeof(bb_page));

        COPY_SQLITE3_INT(page->id, st, 0);
        COPY_SQLITE3_STRING(page->id_name, st, 1);
        COPY_SQLITE3_STRING(page->name, st, 2);
        COPY_SQLITE3_INT(page->style, st, 3);

        if (sqlite3_column_type(st, 4) != SQLITE_NULL) {
            const char *tags = (char *) sqlite3_column_text(st, 4);
            page->tags = tokenize_tags(tags, ",");
        } else {
            page->tags = NULL;
        }

        bb_vec_add(pages, page);
    }
    return 0;
}
void db_pages_free_cb(void *d) {
    bb_page *page = (bb_page *)d;
    free(page->name);
    free(page->id_name);
    if (page->tags != NULL) bb_vec_free(page->tags);
    free(page);
}
bb_vec * db_pages()
{
    bb_vec *pages = malloc(sizeof(bb_vec));
    bb_vec_init(pages, db_pages_free_cb);

    execute_query(NULL, db_pages_cb, pages, LOAD_PAGES, "");

    return pages;
}

/*
 * Get a list of users from the database
 */
int db_users_cb(sqlite3_stmt* st, void* a) {
    bb_vec *users = (bb_vec *)a;
    while(sqlite3_step(st) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        bb_user *user = malloc(sizeof(bb_user));
        bb_user_init(user);

        COPY_SQLITE3_INT(user->id, st, 0);
        COPY_SQLITE3_STRING(user->email, st, 1);
        COPY_SQLITE3_STRING(user->name_id, st, 2);
        COPY_SQLITE3_STRING(user->name, st, 3);
        COPY_SQLITE3_STRING(user->about, st, 4);
        COPY_SQLITE3_STRING(user->thumbnail, st, 5);

        bb_vec_add(users, user);
    }
    return 0;
}
void db_users_free_cb(void *d) {
    bb_user *user = (bb_user *)d;
    bb_user_free(user);
    free(user);
}
bb_vec * db_admin_all_users()
{
    bb_vec *users = malloc(sizeof(bb_vec));
    bb_vec_init(users, db_users_free_cb);
    
    execute_query(NULL, db_users_cb, users, LOAD_USERS, "");
    
    return users;
}
bb_vec * db_admin_user(int id)
{
    bb_vec *users = malloc(sizeof(bb_vec));
    bb_vec_init(users, db_users_free_cb);
    
    execute_query(NULL, db_users_cb, users, LOAD_USER_ID, "i", id);
    
    return users;
}
bb_vec * db_author(char *name_id)
{
    bb_vec *users = malloc(sizeof(bb_vec));
    bb_vec_init(users, db_users_free_cb);
    
    execute_query(NULL, db_users_cb, users, USER_INFO_FROM_NAME_ID, "s", name_id);
    
    return users;
}

/*
 * Admin interface functions
 */
bb_vec * db_admin_all_posts_preview() {
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                ADMIN_ALL_POSTS_QUERY,
                "");
    return all_posts;
}
bb_vec * db_admin_id(int id) {
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, vec_post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                ADMIN_POST_ID_QUERY,
                "i", id);
    return all_posts;
}

/*
 * Updates tags and relationships for a given page or post ID.
 * First removes relationships, adds new tags, then builds new relationships
 */
int db_update_tags(sqlite3 *db, bb_vec *tags, int id, int relation) {
    int rc;
    sqlite3_stmt *results;

    // Delete old relationships
    char sql[255]; sql[0] = '\0';
    if (relation == POST) {
        strcat(sql, "DELETE FROM tags_relate WHERE post_id = ?");
    } else if (relation == PAGE) {
        strcat(sql, "DELETE FROM tags_pages_relate WHERE page_id = ?");
    } else {
        fprintf(stderr, "Incorrect relationship supplied for Tag update\n");
        return 0;
    }
    rc = execute_query(db, NULL, NULL, sql, "i", id);
    if (!rc) {
        fprintf(stderr, "Couldn't remove tag relationships\n");
        return 0;
    }

    // If we didn't get any new tags then we're done
    if (tags == NULL || bb_vec_count(tags) < 1) {
        return 1;
    }
    
    /* Update Tags table */
    // Build query string, max 20 tags
    sql[0] = '\0';
    strcat(sql, "INSERT OR IGNORE INTO tags (tag) VALUES");
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        strcat(sql, "(?)");
        if (i+1 < 20 && i+1 < bb_vec_count(tags))
            strcat(sql, ",");
    }

    // Prepare sql statement
    rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    // Bind all tags to prepared statement
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        if ( sqlite3_bind_text(results, i+1, (char*)bb_vec_get(tags, i), (int)strlen((char*)bb_vec_get(tags, i)), SQLITE_TRANSIENT) != SQLITE_OK) {
            fprintf(stderr, "Failed binding tags to query\n");
            sqlite3_finalize(results);
            return 0;
        }
    }
    rc = sqlite3_step(results);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        fprintf(stderr, "Couldn't add tags to database\n");
        sqlite3_finalize(results);
        return 0;
    }
    sqlite3_finalize(results);

    /* Update relationship table */
    // Build query
    sql[0] = '\0';
    if (relation == POST) {
        strcat(sql, "INSERT OR IGNORE INTO tags_relate (tag_id, post_id) SELECT id, (SELECT id FROM posts WHERE id = ? LIMIT 1) FROM tags WHERE tag IN (");
    } else if (relation == PAGE) {
        strcat(sql, "INSERT OR IGNORE INTO tags_pages_relate (tag_id, page_id) SELECT id, (SELECT id FROM pages WHERE id = ? LIMIT 1) FROM tags WHERE tag IN (");
    } else {
        fprintf(stderr, "Incorrect relationship supplied for");
        return 0;
    }
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        strcat(sql, "?");
        if (i+1 < 20 && i+1 < bb_vec_count(tags))
            strcat(sql, ",");
    }
    strcat(sql, ")");
    // Bind and execute
    rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    if (sqlite3_bind_int(results, 1, id) != SQLITE_OK) {
        fprintf(stderr, "Failed binding tags to query\n");
        sqlite3_finalize(results);
        return 0;
    }
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        if ( sqlite3_bind_text(results, i+2, (char*)bb_vec_get(tags, i), (int)strlen((char*)bb_vec_get(tags, i)), SQLITE_TRANSIENT) != SQLITE_OK ) {
            fprintf(stderr, "Failed binding tags to query\n");
            sqlite3_finalize(results);
            return 0;
        }
    }
    rc = sqlite3_step(results);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        fprintf(stderr, "Couldn't add tag relationships to database\n");
        sqlite3_finalize(results);
        return 0;
    }
    sqlite3_finalize(results);

    return 1;
}

// Used to create a new post, post_id is automatically generated by the DB
int db_new_post(bb_post *p) {
    int rc; // return values from sqlite3 functions
    int new_id; // rowid of new post

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_NEW_POST, "iississi",
                p->page_id, p->user.id, p->title, p->text, p->time_r, p->byline, p->thumbnail, p->visible);
    if (!rc) {
        fprintf(stderr, "Failed to add post to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Get the Post ID of the newly added post
    rc = execute_query(db, NULL, &new_id, ADMIN_ROWID_LAST_POST, "");
    if (!rc) {
        fprintf(stderr, "Couldn't get ROWID of newly added post\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, p->tags, new_id, POST);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

int db_update_post(bb_post *p) {
    int rc;

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Update post to database
    rc = execute_query(db, NULL, NULL, ADMIN_UPDATE_POST, "iisisssii",
                p->page_id, p->user.id, p->title, p->time_r, p->text, p->byline, p->thumbnail, p->visible, p->p_id);
    if (!rc) {
        fprintf(stderr, "Failed to update post to database\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, p->tags, p->p_id, POST);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

// Used to remove a post
int db_delete_post(int post_id) {
    int rc;

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Remove post from database
    rc = execute_query(db, NULL, NULL, ADMIN_DELETE_POST, "i", post_id);
    if (!rc) {
        fprintf(stderr, "Failed to delete post to database\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, NULL, post_id, POST);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

int db_new_page(bb_page *p) {
    int rc; // return values from sqlite3 functions
    int new_id; // rowid of new post

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_NEW_PAGE, "ssi",
                p->id_name, p->name, p->style);
    if (!rc) {
        fprintf(stderr, "Failed to add page to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Get the Post ID of the newly added post
    rc = execute_query(db, NULL, &new_id, ADMIN_ROWID_LAST_PAGE, "");
    if (!rc) {
        fprintf(stderr, "Couldn't get ROWID of newly added post\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, p->tags, new_id, PAGE);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

int db_update_page(bb_page *p) {
    int rc;

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Update post to database
    rc = execute_query(db, NULL, NULL, ADMIN_UPDATE_PAGE, "ssii",
                p->id_name, p->name, p->style, p->id);
    if (!rc) {
        fprintf(stderr, "Failed to update page to database\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, p->tags, p->id, PAGE);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

int db_delete_page(int page_id) {
    int rc;

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Remove post from database
    rc = execute_query(db, NULL, NULL, ADMIN_DELETE_PAGE, "i", page_id);
    if (!rc) {
        fprintf(stderr, "Failed to delete page to database\n");
        sqlite3_close(db);
        return 0;
    }
    rc = db_update_tags(db, NULL, page_id, PAGE);
    if (!rc) {
        sqlite3_close(db);
        return 0;
    }

    // NULL out Page Ids in Posts
    rc = execute_query(db, NULL, NULL, ADMIN_DELETE_PAGE_NULL_POSTS, "i", page_id);
    if (!rc) {
        fprintf(stderr, "Failed to NULLify page ids in posts to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

int db_admin_new_user(bb_user *u, const char *password) {
    int rc; // return values from sqlite3 functions

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_NEW_USER, "ssssss",
                u->email, password, u->name_id, u->name, u->about, u->thumbnail);
    if (!rc) {
        fprintf(stderr, "Failed to add user to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}
int db_admin_update_user(bb_user *u) {
    int rc; // return values from sqlite3 functions

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_UPDATE_USER, "sssssi",
                u->email, u->name_id, u->name, u->about, u->thumbnail, u->id);
    if (!rc) {
        fprintf(stderr, "Failed to update user to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}
int db_admin_delete_user(int id) {
    int rc; // return values from sqlite3 functions

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_DELETE_USER, "i", id);
    if (!rc) {
        fprintf(stderr, "Failed to update user to database\n");
        sqlite3_close(db);
        return 0;
    }

    // NULL out user_ids in Posts
    rc = execute_query(db, NULL, NULL, ADMIN_DELETE_USER_NULL_POSTS, "i", id);
    if (!rc) {
        fprintf(stderr, "Failed to NULLify user_ids in posts to database\n");
        sqlite3_close(db);
        return 0;
    }

    // Close connection
    if (!close_database_transaction(db)) {
        return 0;
    }
    return 1;
}

Archives load_archives() {

    Archives archives;
    archives.month_s = NULL;
    archives.month = NULL;
    archives.year = NULL;
    archives.post_count = NULL;
    archives.row_count = 0;

    // Open db
    sqlite3 *db;
    sqlite3_stmt *results;
    char *sql = ARCHIVES;

    // Open database
    if (( db = open_database() ) == NULL) {
        sqlite3_close(db);
        return archives;
    }

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
    sqlite3_close(db);

    return archives;
}

int verify_user_cb(sqlite3_stmt* st, void* a) {
    if(sqlite3_step(st) == SQLITE_ROW) {
        *(int*)a = 1;
    }
    return 0;
}
int verify_user(const char* user, const char* password) {
    int success = 0;
    int exists = 0;

    success = execute_query(NULL, verify_user_cb, &exists, CHECK_USER, "ss", user, password);

    if (success && exists)
        return 1;
    
    return 0;
}

int verify_session_cb(sqlite3_stmt* st, void* a) {
    bb_user *u = (bb_user *)a;

    if(sqlite3_step(st) == SQLITE_ROW) {
        if (u == NULL) return 0;

        u->id = sqlite3_column_int(st, 0);

        u->email = calloc(sqlite3_column_bytes(st, 1)+1, 1);
        memcpy(u->email,
                sqlite3_column_text(st, 1),
                sqlite3_column_bytes(st, 1));
        
        u->name_id = calloc(sqlite3_column_bytes(st, 2)+1, 1);
        memcpy(u->name_id,
                sqlite3_column_text(st, 2),
                sqlite3_column_bytes(st, 2));
        
        u->name = calloc(sqlite3_column_bytes(st, 3)+1, 1);
        memcpy(u->name,
                sqlite3_column_text(st, 3),
                sqlite3_column_bytes(st, 3));
    } else {
        return 1;
    }
    return 0;
}
int verify_session(const char* session, bb_user *u) {
    int success = 0;

    success = execute_query(NULL, verify_session_cb, u, CHECK_SESSION, "s", session);

    if (success)
        return 1;
    
    return 0;
}

int set_user_session_cb(sqlite3_stmt* st, void* a) {
    if(sqlite3_step(st) == SQLITE_DONE ) {
        *(int*)a = 1;
    }
    return 0;
}
int set_user_session(const char* user, const char* password, const char* session) {
    int success = 0;
    int exists = 0;

    success = execute_query(NULL, set_user_session_cb, &exists, SET_SESSION, "sss", session, user, password);

    if (success && exists)
        return 1;
    
    return 0;
}

void free_archives(Archives *archives)
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

time_t db_get_last_update()
{   
    time_t time = 0;
    execute_query(NULL, NULL, &time, "SELECT time FROM last_update", "");
    return time;
}
