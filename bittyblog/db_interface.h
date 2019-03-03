//
//  db_interface.h
//  bittyblog
//
//  Created by Colin Luoma on 2016-11-19.
//  Copyright © 2016 Colin Luoma. All rights reserved.
//

#ifndef db_interface_h
#define db_interface_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "vec.h"

typedef struct {
    int p_id;
    int page_id;
    int visible;
    char *page;
    char *title;
    char *text;
    char *time;
    char *byline;
    char *extra;
    char *thumbnail;
} Post;

typedef struct {
    Post *p;
    int n;
} vector_p;

// Archives holds a list of months, years, and post counts
// for those archives sidebar element
typedef struct {
    char **month_s;
    int *month;
    int *year;
    int *post_count;
    int row_count;
} Archives;

// Queries for loading posts
#define N_POSTS_QUERY "SELECT title, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE p.visible = 1 ORDER BY time DESC limit @LIMIT offset @OFFSET"
#define POST_ID_QUERY "SELECT title, page_id, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail FROM posts p WHERE p.id = @ID AND p.visible = 1"
#define SEARCH_QUERY "SELECT title, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE lower(text) like lower('%' || @KEYWORD || '%') AND p.visible = 1 ORDER BY time DESC"
#define N_SEARCH_QUERY "SELECT title, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE lower(text) like lower('%' || @KEYWORD || '%') AND p.visible = 1 ORDER BY time DESC limit @LIMIT offset @OFFSET"
#define SEARCH_COUNT_QUERY "SELECT COUNT(*) FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE lower(text) like lower('%' || @KEYWORD || '%') AND p.visible = 1 ORDER BY time DESC"
#define MONTH_YEAR_QUERY "SELECT title, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id WHERE CAST(strftime('%m',time,'unixepoch') AS INT) = @MONTH AND CAST(strftime('%Y',time,'unixepoch') AS INT) = @YEAR AND p.visible = 1 ORDER BY time DESC"
#define ARCHIVES "SELECT strftime('%m',time,'unixepoch') AS month, strftime('%Y',time,'unixepoch') AS year, COUNT(*) AS num_posts FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = 'blog') a ON p.page_id = a.id WHERE p.visible = 1 GROUP BY month, year ORDER BY time DESC"

// Queries for loading posts, for admin page
#define ADMIN_ALL_POSTS_QUERY "SELECT a.name as page, p.id as id, title, datetime(time, 'unixepoch') as time, byline, thumbnail, visible FROM posts p INNER JOIN pages a ON p.page_id = a.id ORDER BY time DESC"
#define ADMIN_POST_ID_QUERY "SELECT title, page_id, p.id as id, text, byline, datetime(time, 'unixepoch') AS time, thumbnail, visible FROM posts p WHERE p.id = @ID"

// Queries for loading pages
#define LOAD_PAGES "SELECT id, name_id, name, style FROM pages"

// Queries for checking user login info and setting session ids
#define CHECK_USER "SELECT 1 FROM users WHERE email = @USER AND password = @PASSWORD"
#define CHECK_SESSION "SELECT 1 FROM users WHERE session = @SESSION"
#define SET_SESSION "UPDATE users SET session = @SESSION WHERE email = @USER AND password = @PASSWORD"

// Queries for settings
#define SETTINGS "SELECT name, value FROM settings"

int db_count(char* page_name_id);
int db_search_count(char* page_name_id, char* keyword);
vector_p * db_search(char* page_name_id, char *keyword);
vector_p * db_nsearch(char* page_name_id, char *keyword, int count, int offset);
vector_p * db_monthyear(char* page_name_id, int month, int year);
vector_p * db_nposts(char* page_name_id, int count, int offset);
vector_p * db_id(int id);

int verify_user(const char* user, const char* password);
int verify_session(const char* session);
int set_user_session(const char* user, const char* password, const char* session);

// Admin interface
vector_p * db_admin_all_posts_preview();
vector_p * db_admin_id(int id);
int db_new_post(Post *p);
int db_update_post(Post *p);

Archives load_archives();

char *nmonth_to_smonth(int month);

// Function to NULL out a Post struct
void Post_init(Post*);

// Vector implementation that holds results
vector_p * vector_p_new();
void vector_p_append(vector_p *vp, Post *p);
void vector_p_free(vector_p *vp);
void free_archives(Archives *archives);

// Load pages from database
bb_vec * db_pages();

#endif /* db_interface_h */