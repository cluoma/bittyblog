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
#include "bittyblog.h"

/*
 * General interface for opening database connections
 */
db_conn * db_open_conn(int dbtype, int mode);
int db_close_conn(db_conn *c);

/*
 * Queries for loading posts
 */
#define N_POSTS_QUERY "SELECT title, a.name_id as page, p.id as id, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
	FROM tags t \
	INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
	GROUP BY post_id \
) t2 \
ON p.id = t2.post_id \
LEFT JOIN pages a ON p.page_id = a.id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
AND (p.id IN (SELECT post_id \
	FROM tags t \
	INNER JOIN tags_relate t3 ON t.id = t3.tag_id \
	INNER JOIN tags_pages_relate t2 ON t.id = t2.tag_id \
	INNER JOIN pages p ON t2.page_id = p.id \
	WHERE name_id = ?) \
	OR page_id IN (SELECT id FROM pages WHERE name_id = ?)) \
ORDER BY time DESC \
limit ? offset ?"
#define N_POSTS_COUNT_QUERY "SELECT COUNT(1) \
FROM posts p \
WHERE p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
AND (id IN (SELECT post_id \
	FROM tags t \
	INNER JOIN tags_relate t3 ON t.id = t3.tag_id \
	INNER JOIN tags_pages_relate t2 ON t.id = t2.tag_id \
	INNER JOIN pages p ON t2.page_id = p.id \
	WHERE name_id = ?) \
	OR page_id IN (SELECT id FROM pages WHERE name_id = ?))"
#define POST_ID_QUERY "SELECT title, page_id, a.name_id as page, p.id as id, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.about AS user_about, u.thumbnail AS user_thumbnail \
FROM posts p \
LEFT JOIN pages a ON p.page_id = a.id \
LEFT JOIN users u ON p.user_id = u.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
WHERE p.id = @ID AND p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now')"
#define SEARCH_QUERY "SELECT title, p.id as id, a.name_id as page, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p \
INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE lower(text) like lower('%' || @KEYWORD || '%') \
AND p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC"
#define N_SEARCH_QUERY "SELECT title, p.id as id, a.name_id as page, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p \
INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE lower(text) like lower('%' || @KEYWORD || '%') \
AND p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC \
limit @LIMIT offset @OFFSET"
#define SEARCH_COUNT_QUERY "SELECT COUNT(*) \
FROM posts p \
INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
WHERE lower(text) like lower('%' || @KEYWORD || '%') AND \
p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC"
#define N_TAG_QUERY "SELECT title, p.id as id, a.name_id as page, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p \
LEFT JOIN pages a ON p.page_id = a.id \
INNER JOIN (SELECT DISTINCT tr.post_id \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
WHERE t.`tag` = ? \
) t1 \
ON p.id = t1.post_id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t2 \
ON p.id = t2.post_id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC \
limit ? offset ?"
#define TAG_COUNT_QUERY "SELECT COUNT(*) \
FROM posts p \
INNER JOIN (SELECT DISTINCT tr.post_id \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
WHERE t.`tag` = ? \
) t \
ON p.id = t.post_id \
WHERE p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now')"
#define N_AUTHOR_QUERY "SELECT title, a.name_id as page, p.id as id, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
	FROM tags t \
	INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
	GROUP BY post_id \
) t2 \
ON p.id = t2.post_id \
LEFT JOIN pages a ON p.page_id = a.id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE p.visible = 1 \
AND u.name_id = ? \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC \
limit ? offset ?"
#define N_AUTHOR_COUNT_QUERY "SELECT COUNT(1) \
FROM posts p \
LEFT JOIN users u ON p.user_id = u.id \
WHERE p.visible = 1 \
AND u.name_id = ? \
AND datetime(time, 'unixepoch') <= datetime('now')"
#define MONTH_YEAR_QUERY "SELECT title, p.id as id, a.name_id as page, text, byline, time as time_r, datetime(time, 'unixepoch') AS time, p.thumbnail, tags, \
COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name, u.thumbnail AS user_thumbnail \
FROM posts p INNER JOIN (SELECT * FROM pages WHERE name_id = @NAMEID) a ON p.page_id = a.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE CAST(strftime('%m',time,'unixepoch') AS INT) = @MONTH \
AND CAST(strftime('%Y',time,'unixepoch') AS INT) = @YEAR \
AND p.visible = 1 \
AND datetime(time, 'unixepoch') <= datetime('now') \
ORDER BY time DESC"
#define USER_INFO_FROM_NAME_ID "SELECT id, email, name_id, name, about, thumbnail \
FROM users \
WHERE name_id = ?"

/*
 * Queries for loading posts, for admin page
 */
#define ADMIN_ALL_POSTS_QUERY "SELECT a.name as page, p.id as id, title, datetime(time, 'unixepoch') as time, byline, p.thumbnail, visible, tags, \
p.user_id AS user_id, COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name \
FROM posts p \
LEFT JOIN pages a ON p.page_id = a.id \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
LEFT JOIN users u ON p.user_id = u.id \
ORDER BY time DESC"
#define ADMIN_POST_ID_QUERY "SELECT title, page_id, p.id as id, text, byline, datetime(time, 'unixepoch') as time, time as time_r, p.thumbnail, visible, tags, \
p.user_id AS user_id, COALESCE(u.name_id, 'Unknown') AS user_name_id, COALESCE(u.name, 'Unknown') AS user_name \
FROM posts p \
LEFT JOIN (SELECT tr.post_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_relate tr on (tr.tag_id = t.id) \
GROUP BY post_id \
) t \
ON p.id = t.post_id \
LEFT JOIN users u ON p.user_id = u.id \
WHERE p.id = @ID"

/*
 * Queries for adding, updating, and removing posts, pages, and users
 */
#define ADMIN_NEW_POST "INSERT INTO posts (page_id, user_id, title, text, time, byline, thumbnail, visible) VALUES(?, ?, ?, ?, ?, ?, ?, ?)"
#define ADMIN_ROWID_LAST_POST "SELECT last_insert_rowid() FROM posts"
#define ADMIN_UPDATE_POST "UPDATE posts SET page_id = ?, user_id = ?, title = ?, time = ?, text = ?, byline = ?, thumbnail = ?, visible = ? WHERE id = ?"
#define ADMIN_DELETE_POST "DELETE FROM posts WHERE id = ?"

#define ADMIN_NEW_PAGE "INSERT INTO pages (name_id, name, style) VALUES(?, ?, ?)"
#define ADMIN_ROWID_LAST_PAGE "SELECT last_insert_rowid() FROM pages"
#define ADMIN_UPDATE_PAGE "UPDATE pages SET name_id = ?, name = ?, style = ? WHERE id = ?"
#define ADMIN_DELETE_PAGE "DELETE FROM pages WHERE id = ?"
#define ADMIN_DELETE_PAGE_NULL_POSTS "UPDATE posts SET page_id = NULL WHERE page_id = ?"

#define ADMIN_NEW_USER "INSERT INTO users (email, password, name_id, name, about, thumbnail) VALUES(?, ?, ?, ?, ?, ?)"
#define ADMIN_UPDATE_USER "UPDATE users SET email = ?, name_id = ?, name = ?, about = ?, thumbnail = ? WHERE id = ?"
#define ADMIN_DELETE_USER "DELETE FROM users WHERE id = ?"
#define ADMIN_DELETE_USER_NULL_POSTS "UPDATE posts SET user_id = NULL WHERE page_id = ?"

/*
 * Queries for loading pages
 */
#define LOAD_PAGES "SELECT id, name_id, name, style, tags FROM pages p \
LEFT JOIN (SELECT tr.page_id, group_concat(t.tag, ', ') `tags` \
FROM tags t \
INNER JOIN tags_pages_relate tr on (tr.tag_id = t.id) \
GROUP BY page_id \
) t \
ON p.id = t.page_id"

/*
 * Queries for loading user data
 */
#define LOAD_USERS "SELECT id, email, name_id, name, about, thumbnail FROM users"
#define LOAD_USER_ID "SELECT id, email, name_id, name, about, thumbnail FROM users WHERE id = ?"

/*
 * Queries for checking user login info and setting session ids
 */
#define CHECK_USER "SELECT 1 FROM users WHERE email = @USER AND password = @PASSWORD"
#define CHECK_SESSION "SELECT id, email, name_id, name FROM users WHERE session = @SESSION"
#define SET_SESSION "UPDATE users SET session = @SESSION WHERE email = @USER AND password = @PASSWORD"

/*
 * Queries for settings
 */
#define SETTINGS "SELECT name, value FROM settings"

/*
 * Post query functions for viewing
 */
int 		db_count		(db_conn* c, char* page_name_id);
int 		db_search_count	(db_conn* c, char* page_name_id, char* keyword);
int 		db_tag_count	(db_conn* c, char* tag);
int 		db_author_count	(db_conn* c, char* name_id);
bb_vec * 	db_search		(db_conn* c, char* page_name_id, char* keyword);
bb_vec * 	db_nsearch		(db_conn* c, char* page_name_id, char* keyword, int count, int offset);
bb_vec * 	db_ntag			(db_conn* c, char* tag, int count, int offset);
bb_vec * 	db_nauthor		(db_conn* c, char* name_id, int count, int offset);
bb_vec * 	db_monthyear	(db_conn* c, char* page_name_id, int month, int year);
bb_vec * 	db_nposts		(db_conn* c, char* page_name_id, int count, int offset);
bb_vec * 	db_id			(db_conn* c, int id);

/*
 * Author query functions for viewing
 */
bb_vec * db_author(db_conn* c, char* name_id);

// Load pages from database
bb_vec * db_pages(db_conn* c);
// Get the timestamp of when the database was last updated
time_t db_get_last_update(db_conn* c);

/*
 * Admin DB functions: add, remove, etc.
 */
// Posts
bb_vec * db_admin_all_posts_preview();
bb_vec * db_admin_id(int id);
int db_new_post(bb_post *p);
int db_update_post(bb_post *p);
int db_delete_post(int post_id);
// Pages
int db_new_page(bb_page *p);
int db_update_page(bb_page *p);
int db_delete_page(int page_id);
// Users
bb_vec * db_admin_all_users();
bb_vec * db_admin_user(int id);
int db_admin_new_user(bb_user *u, const char* password);
int db_admin_update_user(bb_user *u);
int db_admin_delete_user(int id);

// Login session interface
// Verify a user by their email and password
// if sid is supplied, a unique session key is set and stored in sid
int verify_user(const char* user, const char* password, char* sid);
// Verify a user by their session key
// if u is supplied, user information is stored in u
int verify_session(const char* sid, bb_user *u);

#endif /* db_interface_h */
