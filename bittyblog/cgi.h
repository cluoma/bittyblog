//
//  cgi.h
//  bittyblog
//
//  Created by Colin Luoma on 2017-03-08.
//  Copyright © 2017 Colin Luoma. All rights reserved.
//

#ifndef cgi_h
#define cgi_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <d_string.h>

// Linked list to hold URI query variables
typedef struct query_var {
    char *key, *val;
    long val_len;
    struct query_var *next;
} query_var;

// Return a linked list of query string key-value pairs
query_var * bb_cgi_get_query(const char *query_s); // add query string variables to list
query_var * bb_cgi_get_post(query_var *); // add variables from post data to list
query_var * bb_cgi_get_uri(query_var *, const char *); // add uri path to list as uripathX X in [0,MAX_INT]

char * bb_cgi_get_var(query_var *qv, const char* key);
long bb_cgi_get_var_len(query_var *qv, const char* key);
int bb_cgi_remove_var(query_var *qv, const char* key);
int bb_cgi_remove_all_var(query_var **);
int bb_cgi_add_var(query_var **qv, const char* key, const char* val, long val_len);

char * bb_cgi_query_string(query_var *qv);
char * bb_cgi_query_string_wo(query_var *qv, const char* key);

// Utility functions
void html_to_text(char *source, char *dest);    // Decodes URL strings to text (eg '+' -> ' ' and % hex codes)
char * newline_to_html(const char* string);     // Changes all '\n' characters into <br> for use in html
char *url_encode(char *str);                    // Encodes URL returns a url-decoded version of str
char * html_escape(char *str, size_t len);      // Sanitizes text for use in HTML
unsigned long hash(unsigned char *str);         // djb2 hash function
char *strtrim(char *str);                       // Removes leading and trailing: spaces, tabs, carriage ret, and newlines

// Check if a buffer looks like an image using magic numbers (jpg, png, gif)
int is_image_file(char *buf);

#endif /* cgi_h */
