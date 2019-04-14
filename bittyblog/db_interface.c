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
int close_database_transaction(sqlite3 *db)
{
    // Commit transaction
    if (sqlite3_exec(db, "COMMIT;", 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Couldn't commit transaction.\n");
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);

    return 1;
}


/*
 *  Execute query function takes arguments to prepare a statement then
 *  executes the query. A callback must be provided to deal with the results.
 * 
 *  If NULL is supplied as the first argument, then a connection is opened
 *  to execute the given query.
 * 
 *  Arguments will be bound to the query in order, according to the types string
 *  types is a string consisting of:
 *  s: string char*
 *  i: int
 *  eg. "ssis" will bind arguments as string, string, int, string
 * 
 *  Callbacks MUST return 0, otherwise resources will be freed and NULL returned.
 * 
 *  If no callback is supplied, the query will be executed once using sqlite3_step
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

// Load post data from database
int load_posts_cb(sqlite3_stmt *results, void* data) {
    bb_vec *vp = (bb_vec*)data;

    while(sqlite3_step(results) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        Post *post = malloc(sizeof(Post));
        Post_init(post);

        int col_count = sqlite3_column_count(results);
        for (int i = 0; i < col_count; i++) {
            const char* col_name = sqlite3_column_name(results, i);
            if (strcmp(col_name, "title") == 0) {
                const char *title = (char *) sqlite3_column_text(results, i);
                const int title_len = sqlite3_column_bytes(results, i);
                post->title = calloc(title_len + 1, 1);
                memcpy(post->title, title, title_len);
            } else if (strcmp(col_name, "id") == 0) {
                post->p_id = sqlite3_column_int(results, i);
            } else if (strcmp(col_name, "page_id") == 0) {
                post->page_id = sqlite3_column_int(results, i);
            } else if (strcmp(col_name, "page") == 0) {
                const char *page_name = (char *) sqlite3_column_text(results, i);
                const int page_name_len = sqlite3_column_bytes(results, i);
                post->page = calloc(page_name_len + 1, 1);
                memcpy(post->page, page_name, page_name_len);
            } else if (strcmp(col_name, "text") == 0) {
                const char *text = (char *) sqlite3_column_text(results, i);
                const int text_len = sqlite3_column_bytes(results, i);
                post->text = calloc(text_len + 1, 1);
                memcpy(post->text, text, text_len);
            } else if (strcmp(col_name, "time") == 0) {
                const char *time = (char *) sqlite3_column_text(results, i);
                const int time_len = sqlite3_column_bytes(results, i);
                post->time = calloc(time_len + 1, 1);
                memcpy(post->time, time, time_len);
            } else if (strcmp(col_name, "time_r") == 0) {
                post->time_r = sqlite3_column_int64(results, i);
            } else if (strcmp(col_name, "byline") == 0) {
                const char *byline = (char *) sqlite3_column_text(results, i);
                const int byline_len = sqlite3_column_bytes(results, i);
                post->byline = calloc(byline_len + 1, 1);
                memcpy(post->byline, byline, byline_len);
            }
            // else if (strcmp(col_name, "extra") == 0) {
                
            // }
            else if (strcmp(col_name, "thumbnail") == 0) {
                const char *thumbnail   = (char *) sqlite3_column_text(results, i);
                const int thumbnail_len = sqlite3_column_bytes(results, i);
                post->thumbnail = calloc(thumbnail_len + 1, 1);
                memcpy(post->thumbnail, thumbnail, thumbnail_len);
            } else if (strcmp(col_name, "visible") == 0) {
                post->visible = sqlite3_column_int(results, i);
            }
            else if (strcmp(col_name, "tags") == 0) {
                if (sqlite3_column_type(results, i) != SQLITE_NULL) {
                    const char *tags = (char *) sqlite3_column_text(results, i);
                    post->tags = tokenize_tags(tags, ",");
                }
            }
        }
        bb_vec_add(vp, post);
    }
    return 0;
}

bb_vec * db_ntag(char* tag, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_TAG_QUERY,
                "sii", tag, count, offset);
    return all_posts;
}
bb_vec * db_search(char* page_name_id, char *keyword)
{   
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                SEARCH_QUERY,
                "ss", page_name_id, keyword);
    return all_posts;
}
bb_vec * db_nsearch(char* page_name_id, char *keyword, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_SEARCH_QUERY,
                "ssii", page_name_id, keyword, count, offset);
    return all_posts;
}
bb_vec * db_monthyear(char* page_name_id, int month, int year)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                MONTH_YEAR_QUERY,
                "sii", page_name_id,  month, year);
    return all_posts;
}
bb_vec * db_nposts(char* page_name_id, int count, int offset)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                N_POSTS_QUERY,
                "ssii", page_name_id, page_name_id,  count, offset);
    return all_posts;
}
bb_vec * db_id(int id)
{
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
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
        //"SELECT COUNT(1) FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE p.visible = 1 ",
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

/*
 * Get a list of pages from the database
 */
int db_pages_cb(sqlite3_stmt* st, void* a) {
    bb_vec *pages = (bb_vec*)a;
    while(sqlite3_step(st) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        bb_page *page = malloc(sizeof(bb_page));

        page->id = sqlite3_column_int(st, 0);
        page->id_name = calloc(sqlite3_column_bytes(st, 1)+1, 1);
        
        memcpy(page->id_name,
                sqlite3_column_text(st, 1),
                sqlite3_column_bytes(st, 1));

        page->name = calloc(sqlite3_column_bytes(st, 2)+1, 2);
        memcpy(page->name,
                sqlite3_column_text(st, 2),
                sqlite3_column_bytes(st, 2));
        page->style = sqlite3_column_int(st, 3);

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
 * Admin interface functions
 */
bb_vec * db_admin_all_posts_preview() {
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
    execute_query(NULL, load_posts_cb, all_posts,
                ADMIN_ALL_POSTS_QUERY,
                "");
    return all_posts;
}
bb_vec * db_admin_id(int id) {
    bb_vec * all_posts = malloc(sizeof(bb_vec));
    bb_vec_init(all_posts, Post_free);
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
int db_new_post(Post *p) {
    int rc; // return values from sqlite3 functions
    int new_id; // rowid of new post

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Add post to database
    rc = execute_query(db, NULL, NULL, ADMIN_NEW_POST, "ississi",
                p->page_id, p->title, p->text, p->time_r, p->byline, p->thumbnail, p->visible);
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

int db_update_post(Post *p) {
    int rc;

    // Connect to DB
    sqlite3 *db = open_database_transaction();
    if (db == NULL) {
        return 0;
    }

    // Update post to database
    rc = execute_query(db, NULL, NULL, ADMIN_UPDATE_POST, "isisssii",
                p->page_id, p->title, p->time_r, p->text, p->byline, p->thumbnail, p->visible, p->p_id);
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

int verify_session(const char* session) {
    int success = 0;
    int exists = 0;

    success = execute_query(NULL, verify_user_cb, &exists, CHECK_SESSION, "s", session);

    if (success && exists)
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

void Post_init(Post* p) {
    p->p_id = -1;
    p->page_id = -1;
    p->time_r = 0;
    p->page = NULL;
    p->title = NULL;
    p->text = NULL;
    p->time = NULL;
    p->byline = NULL;
    p->extra = NULL;
    p->thumbnail = NULL;
    p->visible = 0;
    p->tags = NULL;
}

void Post_free(Post* p) {
    if (p->page != NULL)        free(p->page);
    if (p->title != NULL)       free(p->title);
    if (p->text != NULL)        free(p->text);
    if (p->time != NULL)        free(p->time);
    if (p->byline != NULL)      free(p->byline);
    if (p->extra != NULL)       free(p->extra);
    if (p->thumbnail != NULL)   free(p->thumbnail);
    if (p->tags != NULL)        bb_vec_free(p->tags);
    free(p);
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
