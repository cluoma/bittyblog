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
    char *db_location = DB_PATH;

    int err;
    if( (err = sqlite3_open(db_location, &db)) )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        fprintf(stderr, "ERRNO: %d\n", err);
        sqlite3_close(db);
        return NULL;
    }
    else
    {
        //fprintf(stderr, "Opened database successfully\n");
        return db;
    }
}

/*
 *  Execute query function takes arguments to prepare a statement then
 *  executes the query. A callback must be provided to deal with the results
 */
int execute_query(void (*callback)(sqlite3_stmt*, void*), void* data,
                  const char* query, const char* types, ...) {
    sqlite3 *db;
    sqlite3_stmt *results;
    va_list ap;
    int param_count;
    char type;

    if (( db = open_database() ) == NULL) {
        return 0;
    }

    if (sqlite3_prepare_v2(db, query, -1, &results, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }

    // Bind all arguments to prepared query
    param_count = sqlite3_bind_parameter_count(results);
    va_start(ap, types);
    int i = 1;
    while ((type = *types++) && i <= param_count) {
        switch (type) {

            case 's':
            if (sqlite3_bind_text(results, i, va_arg(ap, char*), -1, SQLITE_TRANSIENT) != SQLITE_OK)
                return 0;
            break;

            case 'i':
            if (sqlite3_bind_int(results, i, va_arg(ap, int)) != SQLITE_OK)
                return 0;
            break;

            default:
                return 0;
        }
        i++;
    }

    // Call callback function to deal with query results
    (*callback)(results, data);

    sqlite3_finalize(results);
    sqlite3_close(db);

    return 1;
}

// Load post data from database
void load_posts_cb(sqlite3_stmt *results, void* data) {
    vector_p *vp = (vector_p*)data;

    while(sqlite3_step(results) == SQLITE_ROW)
    {
        // Realloc memory for a new post
        Post *post = (Post *) realloc(vp->p, sizeof(Post)*(1+vp->n));
        int n = vp->n;
        Post_init(&post[n]);

        int col_count = sqlite3_column_count(results);
        for (int i = 0; i < col_count; i++) {
            const char* col_name = sqlite3_column_name(results, i);
            if (strcmp(col_name, "title") == 0) {
                const char *title = (char *) sqlite3_column_text(results, i);
                const int title_len = sqlite3_column_bytes(results, i);
                post[n].title = calloc(title_len + 1, 1);
                memcpy(post[n].title, title, title_len);
            } else if (strcmp(col_name, "id") == 0) {
                post[n].p_id = sqlite3_column_int(results, i);
            } else if (strcmp(col_name, "page_id") == 0) {
                post[n].page_id = sqlite3_column_int(results, i);
            } else if (strcmp(col_name, "page") == 0) {
                const char *page_name = (char *) sqlite3_column_text(results, i);
                const int page_name_len = sqlite3_column_bytes(results, i);
                post[n].page = calloc(page_name_len + 1, 1);
                memcpy(post[n].page, page_name, page_name_len);
            } else if (strcmp(col_name, "text") == 0) {
                const char *text = (char *) sqlite3_column_text(results, i);
                const int text_len = sqlite3_column_bytes(results, i);
                post[n].text = calloc(text_len + 1, 1);
                memcpy(post[n].text, text, text_len);
            } else if (strcmp(col_name, "time") == 0) {
                const char *time = (char *) sqlite3_column_text(results, i);
                const int time_len = sqlite3_column_bytes(results, i);
                post[n].time = calloc(time_len + 1, 1);
                memcpy(post[n].time, time, time_len);
            } else if (strcmp(col_name, "byline") == 0) {
                const char *byline = (char *) sqlite3_column_text(results, i);
                const int byline_len = sqlite3_column_bytes(results, i);
                post[n].byline = calloc(byline_len + 1, 1);
                memcpy(post[n].byline, byline, byline_len);
            }
            // else if (strcmp(col_name, "extra") == 0) {
                
            // }
            else if (strcmp(col_name, "thumbnail") == 0) {
                const char *thumbnail   = (char *) sqlite3_column_text(results, i);
                const int thumbnail_len = sqlite3_column_bytes(results, i);
                post[n].thumbnail = calloc(thumbnail_len + 1, 1);
                memcpy(post[n].thumbnail, thumbnail, thumbnail_len);
            } else if (strcmp(col_name, "visible") == 0) {
                post[n].visible = sqlite3_column_int(results, i);
            }
            else if (strcmp(col_name, "tags") == 0) {
                if (sqlite3_column_type(results, i) != SQLITE_NULL) {
                    const char *tags = (char *) sqlite3_column_text(results, i);
                    post[n].tags = tokenize_tags(tags, ",");
                    for (int j = 0; j < bb_vec_count(post[n].tags); j++) {
                        fprintf(stderr, "TAGE:: %s\n", (char*)bb_vec_get(post[n].tags, j));
                    }
                }
            }
        }
        vp->p = post;
        vp->n ++;
    }
}

vector_p * db_search(char* page_name_id, char *keyword)
{   
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                SEARCH_QUERY,
                "ss", page_name_id, keyword);
    return all_posts;
}
vector_p * db_nsearch(char* page_name_id, char *keyword, int count, int offset)
{
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                N_SEARCH_QUERY,
                "ssii", page_name_id, keyword, count, offset);
    return all_posts;
}
vector_p * db_monthyear(char* page_name_id, int month, int year)
{
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                MONTH_YEAR_QUERY,
                "sii", page_name_id,  month, year);
    return all_posts;
}
vector_p * db_nposts(char* page_name_id, int count, int offset)
{
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                N_POSTS_QUERY,
                "sii", page_name_id,  count, offset);
    return all_posts;
}
vector_p * db_id(int id)
{
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                POST_ID_QUERY,
                "i", id);
    return all_posts;
}

// Get the number of posts
void db_count_cb(sqlite3_stmt* st, void* a) {
    while(sqlite3_step(st) == SQLITE_ROW)
    {
        *(int*)a = sqlite3_column_int(st, 0);
        break;
    }
}
int db_count(char* page_name_id)
{
    int success = 0;
    int count = 0;

    success = execute_query(db_count_cb, &count,
        "SELECT COUNT(1) FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE p.visible = 1 ",
        "s", page_name_id);

    if (success && count)
        return count;
    
    return 0;
}
int db_search_count(char* page_name_id, char* keyword) {
    int success = 0;
    int count = 0;

    success = execute_query(db_count_cb, &count,
        SEARCH_COUNT_QUERY,
        "ss", page_name_id, keyword);

    if (success && count)
        return count;
    
    return 0;
}

// Get list of Pages from database
void db_pages_cb(sqlite3_stmt* st, void* a) {
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
        page->name = calloc(sqlite3_column_bytes(st, 1)+1, 2);
        memcpy(page->name,
                sqlite3_column_text(st, 2),
                sqlite3_column_bytes(st, 2));
        page->style = sqlite3_column_int(st, 3);

        bb_vec_add(pages, page);
    }
}
void db_pages_free_cb(void *d) {
    bb_page *page = (bb_page *)d;
    free(page->name);
    free(page->id_name);
    free(page);
}
bb_vec * db_pages()
{
    bb_vec *pages = malloc(sizeof(bb_vec));
    bb_vec_init(pages, db_pages_free_cb);

    execute_query(db_pages_cb, pages, LOAD_PAGES, "");

    return pages;
}


/*
 * Admin interface functions
 */
vector_p * db_admin_all_posts_preview() {
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                ADMIN_ALL_POSTS_QUERY,
                "");
    return all_posts;
}
vector_p * db_admin_id(int id) {
    vector_p * all_posts = vector_p_new();
    execute_query(load_posts_cb, all_posts,
                ADMIN_POST_ID_QUERY,
                "i", id);
    return all_posts;
}

int db_update_tags(Post *p) {
    int rc;
    sqlite3_stmt *results;
    bb_vec *tags = p->tags;

    if (bb_vec_count(tags) < 1) return 1;

    /* Update Tags table */
    // Build query string, max 20 tags
    char sql[150]; sql[0] = '\0';
    strcat(sql, "INSERT OR IGNORE INTO tags (tag) VALUES");
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        strcat(sql, "(?)");
        if (i+1 < 20 && i+1 < bb_vec_count(tags)) strcat(sql, ",");
    }

    /* Connect to DB */
    sqlite3 *db = open_database();
    if (db == NULL) {
        fprintf(stderr, "Failed to open DB connection.\n");
        return 0;
    }

    /* Prepare sql statement */
    rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);

    // Bind input data to sql statement
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to update table: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    // Bind all tags to prepared statement
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        sqlite3_bind_text(results, i+1, (char*)bb_vec_get(tags, i), (int)strlen((char*)bb_vec_get(tags, i)), SQLITE_TRANSIENT);
    }
    // Execute statement
    sqlite3_step(results);
    // Free statement, close DB
    sqlite3_finalize(results);

    /* Update relationship table */
    // Delete old relationships
    sql[0] = '\0';
    strcat(sql, "DELETE FROM tags_relate WHERE post_id = ?");
    rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);
    sqlite3_bind_int(results, 1, p->p_id);
    // Execute statement
    sqlite3_step(results);
    // Free statement, close DB
    sqlite3_finalize(results);

    // Insert new relationships
    sql[0] = '\0';
    strcat(sql, "INSERT OR IGNORE INTO tags_relate (tag_id, post_id) SELECT id, (SELECT id FROM posts WHERE id = ? LIMIT 1) FROM tags WHERE tag IN (");
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        strcat(sql, "?");
        if (i+1 < 20 && i+1 < bb_vec_count(tags)) strcat(sql, ",");
    }
    strcat(sql, ")");
    rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);
    sqlite3_bind_int(results, 1, p->p_id);
    for (int i = 0; i < 20 && i < bb_vec_count(tags); i++) {
        sqlite3_bind_text(results, i+2, (char*)bb_vec_get(tags, i), (int)strlen((char*)bb_vec_get(tags, i)), SQLITE_TRANSIENT);
    }
    // Execute statement
    sqlite3_step(results);
    // Free statement, close DB
    sqlite3_finalize(results);

    sqlite3_close(db);

    return 1;
}

int db_update_post(Post *p) {
   /* SQLite variable declarations */
   int rc;
   sqlite3_stmt *results;

   char *sql = "UPDATE posts SET page_id = ?, title = ?, text = ?, byline = ?, thumbnail = ?, visible = ? WHERE id = ?";

   /* Connect to DB */
   sqlite3 *db = open_database();
   if (db == NULL) {
       fprintf(stderr, "Failed to open DB connection.\n");
       return 0;
   }

   /* Prepare sql statement */
   rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);

   // Bind input data to sql statement
   if (rc != SQLITE_OK) {
       fprintf(stderr, "Failed to update table: %s\n", sqlite3_errmsg(db));
       return 0;
   }
   sqlite3_bind_int(results, 1, p->page_id);
   sqlite3_bind_text(results, 2, p->title, (int)strlen(p->title), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 3, p->text, (int)strlen(p->text), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 4, p->byline, (int)strlen(p->byline), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 5, p->thumbnail, (int)strlen(p->thumbnail), SQLITE_TRANSIENT);
   sqlite3_bind_int(results, 6, p->visible);
   sqlite3_bind_int(results, 7, p->p_id);

   // Execute statement
   sqlite3_step(results);

   // Free statement, close DB
   sqlite3_finalize(results);
   sqlite3_close(db);

   // Update tags
   db_update_tags(p);

   return 1;
}

// Used to create a new post, post_id is automatically generated by the DB
int db_new_post(Post *p) {
   /* SQLite variable declarations */
   int rc;
   sqlite3_stmt *results;

   char *sql = "INSERT INTO posts (page_id, title, text, time, byline, thumbnail, visible) VALUES(?, ?, ?, (strftime('%s', 'now')), ?, ?, ?)";

   /* Connect to DB */
   sqlite3 *db = open_database();
   if (db == NULL) {
       fprintf(stderr, "Failed to open DB connection.\n");
       return 0;
   }

   /* Prepare sql statement */
   rc = sqlite3_prepare_v2(db, sql, -1, &results, 0);

   // Bind input data to sql statement
   if (rc != SQLITE_OK) {
       fprintf(stderr, "Failed to update table: %s\n", sqlite3_errmsg(db));
       return 0;
   }
   sqlite3_bind_int(results, 1, p->page_id);
   sqlite3_bind_text(results, 2, p->title, (int)strlen(p->title), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 3, p->text, (int)strlen(p->text), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 4, p->byline, (int)strlen(p->byline), SQLITE_TRANSIENT);
   sqlite3_bind_text(results, 5, p->thumbnail, (int)strlen(p->thumbnail), SQLITE_TRANSIENT);
   sqlite3_bind_int(results, 6, p->visible);

   // Execute statement
   sqlite3_step(results);

   // Free statement, close DB
   sqlite3_finalize(results);
   sqlite3_close(db);

   return 1;
}

//// Used to remove a post
//int delete_post(int post_id) {
//    /* SQLite variable declarations */
//    int rc;
//    char sql[200];
//    char *zErrMsg = 0;
//
//    /* Create SQL statement */
//    sprintf(sql, "DELETE FROM posts WHERE post_id = %d", post_id);
//
//    /* Connect to DB */
//    sqlite3 *db = open_database();
//
//    /* Execute SQL statement */
//    int success = 0;
//    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
//    if( rc != SQLITE_OK ){
//        fprintf(stderr, "SQL error: %s\n", zErrMsg);
//        sqlite3_free(zErrMsg);
//    }else{
//        fprintf(stderr, "Operation done successfully\n");
//        success = 1;
//    }
//    sqlite3_close(db);
//
//    return success;
//}

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

void verify_user_cb(sqlite3_stmt* st, void* a) {
    if(sqlite3_step(st) == SQLITE_ROW) {
        *(int*)a = 1;
    }
}
int verify_user(const char* user, const char* password) {
    int success = 0;
    int exists = 0;

    success = execute_query(verify_user_cb, &exists, CHECK_USER, "ss", user, password);

    if (success && exists)
        return 1;
    
    return 0;
}

int verify_session(const char* session) {
    int success = 0;
    int exists = 0;

    success = execute_query(verify_user_cb, &exists, CHECK_SESSION, "s", session);

    if (success && exists)
        return 1;
    
    return 0;
}

void set_user_session_cb(sqlite3_stmt* st, void* a) {
    if(sqlite3_step(st) == SQLITE_DONE ) {
        *(int*)a = 1;
    }
}
int set_user_session(const char* user, const char* password, const char* session) {
    int success = 0;
    int exists = 0;

    success = execute_query(set_user_session_cb, &exists, SET_SESSION, "sss", session, user, password);

    if (success && exists)
        return 1;
    
    return 0;
}

void Post_init(Post* p) {
    p->p_id = -1;
    p->page_id = -1;
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

vector_p * vector_p_new()
{
    vector_p *vp = malloc(sizeof(vector_p));
    vp->p = NULL;
    vp->n = 0;

    return vp;
}

void vector_p_append(vector_p *vp, Post *p)
{
    if( p == NULL ) return;

    vp->p = realloc(vp->p, sizeof(Post) * (vp->n+1) );
    vp->p[vp->n] = *p;
    vp->n = vp->n + 1;
}

void vector_p_free(vector_p *vp)
{
    for( int i = 0; i < vp->n; i++ )
    {
        if (vp->p[i].page != NULL) free(vp->p[i].page);
        if (vp->p[i].title != NULL) free(vp->p[i].title);
        if (vp->p[i].text != NULL) free(vp->p[i].text);
        if (vp->p[i].time != NULL) free(vp->p[i].time);
        if (vp->p[i].byline != NULL) free(vp->p[i].byline);
        if (vp->p[i].extra != NULL) free(vp->p[i].extra);
        if (vp->p[i].thumbnail != NULL) free(vp->p[i].thumbnail);
        if (vp->p[i].tags != NULL) bb_vec_free(vp->p[i].tags);
    }
    free( vp->p );
    free( vp );
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
