//
//  cgi.c
//  bittyblog
//
//  Created by Colin Luoma on 2017-03-08.
//  Copyright © 2017 Colin Luoma. All rights reserved.
//

#include <unistd.h>
#include "cgi.h"
#include "bittyblog.h"

#ifdef _FCGI
#include <fcgi_stdio.h>
#endif


#define CONST_STRLEN(x)     (sizeof(x) - 1)
#define MAX_LINEBUF (1023+1)

query_var * bb_cgi_get_query(const char *query_s)
{
    // Copy string
    char *str_cpy = calloc(1, strlen(query_s)+1);
    char *for_free = str_cpy;
    strcpy(str_cpy, query_s);

    char *var, *key, *val;
    query_var *first = NULL, *q_var = NULL;
    while ((var = strsep(&str_cpy, "&")) != NULL)
    {
        key = strsep(&var, "=");
        val = strsep(&var, "=");

        if (key != NULL && val != NULL &&
            strcmp(key, "") != 0 && strcmp(val, "") != 0)
        {
            // Different behaviour for first element
            if (q_var == NULL)
            {
                q_var = malloc(sizeof(query_var));
                q_var->next = NULL;
                first = q_var;
            }
            else
            {
                q_var->next = malloc(sizeof(query_var));
                q_var = q_var->next;
                q_var->next = NULL;
            }

            q_var->key = calloc(1, strlen(key)+1);
            q_var->val = calloc(1, strlen(val)+1);
            strcpy(q_var->key, key);
            html_to_text( val, q_var->val );
        }
    }
    free(for_free);

    return first;
}

query_var * bb_cgi_get_post_simple(query_var *qv)
{
    int size = atoi(GET_ENV_VAR("CONTENT_LENGTH"));
    char *query_s = calloc(size+1, 1);
    //if (read(0, query_s, size) < size) {
    if (fread(query_s, 1, size, stdin) < size) {
        free(query_s);
        return NULL;
    }
    // Copy string
    char *str_cpy = calloc(1, strlen(query_s)+1);
    char *for_free = str_cpy;
    strcpy(str_cpy, query_s);

    char *var, *key, *val;
    while ((var = strsep(&str_cpy, "&")) != NULL)
    {
        key = strsep(&var, "=");
        val = strsep(&var, "=");

        if (key != NULL && val != NULL &&
            strcmp(key, "") != 0 && strcmp(val, "") != 0)
        {
            char *tmp = calloc(1, strlen(val)+1);
            html_to_text( val, tmp );
            bb_cgi_add_var(&qv, key, tmp, strlen(tmp)+1);
            free(tmp);
        }
    }
    free(for_free);
    free(query_s);

    return qv;
}

query_var * bb_cgi_get_uri(query_var *qv, const char *uri_str)
{
    // Copy string
    char *str_cpy = calloc(1, strlen(uri_str)+1);
    char *for_free = str_cpy;
    strcpy(str_cpy, uri_str);

    char *val;
    char key[18];
    int depth = 0;
    while ((val = strsep(&str_cpy, "/")) != NULL)
    {
        if (val != NULL && strcmp(val, "") != 0)
        {
            key[0] = '\0';
            sprintf(key, "uripath%d", depth);
            char *tmp = calloc(1, strlen(val)+1);
            html_to_text( val, tmp );
            bb_cgi_add_var(&qv, key, tmp, strlen(tmp)+1);
            free(tmp);
            depth++;
        }
    }
    free(for_free);

    return qv;
}


/*
 * User interface to get query variables
 */
char * bb_cgi_get_var(query_var *qv, const char *key)
{
    query_var *curr = qv;
    while(curr != NULL)
    {
        if(strcmp(key, curr->key) == 0)
        {
            return curr->val;
        }
        curr = curr->next;
    }
    return NULL;
}

long bb_cgi_get_var_len(query_var *qv, const char* key)
{
    query_var *curr = qv;
    while(curr != NULL)
    {
        if(strcmp(key, curr->key) == 0)
        {
            return curr->val_len;
        }
        curr = curr->next;
    }
    return 0;
}

int bb_cgi_add_var(query_var **qv, const char* key, const char* val, long val_len) {
    query_var *new_var = malloc(sizeof(query_var));
    if (new_var == NULL)
        return 1;

    new_var->key = calloc(1, strlen(key)+1);
    if (new_var->key != NULL) {
        strcpy(new_var->key, key);
    }

    new_var->val = calloc(1, val_len);
    if (new_var->val != NULL) {
        memcpy(new_var->val, val, val_len-1);
        new_var->val[val_len-1] = '\0';
    }

    new_var->val_len = val_len-1;

    new_var->next = *qv;
    *qv = new_var;

    return 0;
}

int bb_cgi_remove_var(query_var *qv, const char* key)
{
    query_var *curr = qv;
    query_var *prev = NULL;
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            free(curr->key);
            free(curr->val);

            if (prev == NULL) {
                qv = curr->next;
                free(curr);
                return 0;
            }
            else {
                prev->next = curr->next;
                free(curr);
                return 0;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    return 1;
}

int bb_cgi_remove_all_var(query_var **qv)
{
    query_var *curr = *qv;
    query_var *prev = NULL;
    while (curr != NULL) {
        free(curr->key);
        free(curr->val);
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    *qv = NULL;
    return 1;
}

// Return query string
char * bb_cgi_query_string(query_var *qv)
{
    char *q_string = calloc(1, 1);
    query_var *var = qv;
    int first = 1;
    while (var != NULL)
    {
        if (!first) strcat(q_string, "&");
        char *key, *val;
        key = var->key;
        val = url_encode( var->val );

        q_string = realloc(q_string, strlen(q_string)+strlen(key)+strlen(val)+3);

        strcat(q_string, key);
        strcat(q_string, "=");
        strcat(q_string, val);

        free( val );
        var = var->next;
        first = 0;
    }
    return q_string;
}

// Return the query string without the selected variable
char * bb_cgi_query_string_wo(query_var *qv, const char* wo_key)
{
    char *q_string = calloc(1, 1);
    query_var *var = qv;
    int first = 1;
    while (var != NULL)
    {
        char *key, *val;
        key = var->key;

        if (strcmp(key, wo_key) != 0) {
            val = url_encode( var->val );

            q_string = realloc(q_string, strlen(q_string)+strlen(key)+strlen(val)+3);

            if (!first) strcat(q_string, "&");
            strcat(q_string, key);
            strcat(q_string, "=");
            strcat(q_string, val);

            free( val );
            first = 0;
        }
        var = var->next;
    }
    return q_string;
}

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* Returns a url-decoded version of str */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

// Decodes URL strings to text (eg '+' -> ' ' and % hex codes)
void html_to_text(char *source, char *dest)
{
    while (*source != '\0') {
        if (*source == '+') {
            *dest = ' ';
        }
        else if (*source == '%') {
            int hex_char;
            sscanf(source+1, "%2x", &hex_char);
            *dest = hex_char;
            source += 2;
        } else {
            *dest = *source;
        }
        source++;
        dest++;
    }
    *dest = '\0';
}

//
char * html_escape(char *str, size_t len)
{
    char *ret = malloc(len + 1);
    size_t ret_len = len;
    size_t ret_ind = 0;

    for( int i = 0; i < len; i++ )
    {
        if( ret_ind + 7 > ret_len )
        {
            ret = realloc( ret, ret_len * 2 + 1 );
            ret_len = ret_len * 2;
        }
        switch( str[i] )
        {
            case '\"':
                memcpy( ret+ret_ind, "&quot;", 6 );
                ret_ind += 6;
                break;
            case '&':
                memcpy( ret+ret_ind, "&amp;", 5 );
                ret_ind += 5;
                break;
            case '<':
                memcpy( ret+ret_ind, "&lt;", 4 );
                ret_ind += 4;
                break;
            case '>':
                memcpy( ret+ret_ind, "&gt;", 4 );
                ret_ind += 4;
                break;
            default:
                ret[ret_ind] = str[i];
                ret_ind += 1;
                break;
        }
    }
    ret[ret_ind] = '\0';
    return ret;
}

// Returns a new string with all '\n' characters replaced by "<br>" for use in HTML
char * newline_to_html(const char* string)
{

    if (string == NULL) return NULL;
    
    int length = 0;
    int newlines = 0;
    const char *strp = string;
    while (*strp != '\0') {
        length++;
        if (*strp == '\n')
            newlines++;
        strp++;
    }

    // Allocate space for new string with enough room for the <br>'s
    char *new_string = malloc( length + newlines*3 + 1 );

    int pre_tags_open = 0;
    int new_index = 0;
    int state = 0;
    int state2 = 0;
    for (int old_index = 0; old_index < length; old_index++) {

        switch (state) {
            case 0: if (string[old_index] == '<') {state = 1;} else {state = 0;}
                break;
            case 1: if (string[old_index] == 'p') {state = 2;} else {state = 0;}
                break;
            case 2: if (string[old_index] == 'r') {state = 3;} else {state = 0;}
                break;
            case 3: if (string[old_index] == 'e') {pre_tags_open++;}
            default:
                state = 0;
        }
        switch (state2) {
            case 0: if (string[old_index] == '<') {state2 = 1;} else {state2 = 0;}
                break;
            case 1: if (string[old_index] == '/') {state2 = 2;} else {state2 = 0;}
                break;
            case 2: if (string[old_index] == 'p') {state2 = 3;} else {state2 = 0;}
                break;
            case 3: if (string[old_index] == 'r') {state2 = 4;} else {state2 = 0;}
                break;
            case 4: if (string[old_index] == 'e') {state2 = 5;} else {state2 = 0;}
                break;
            case 5: if (string[old_index] == '>') {pre_tags_open--;}
            default:
                state2 = 0;
        }

        if (string[old_index] == '\n' && pre_tags_open == 0)
        {
            new_string[new_index] = '<';
            new_index++;
            new_string[new_index] = 'b';
            new_index++;
            new_string[new_index] = 'r';
            new_index++;
            new_string[new_index] = '>';
        }
        else
        {
            new_string[new_index] = string[old_index];
        }
        new_index++;
    }
    new_string[new_index] = '\0';
    return new_string;
}

unsigned long hash(unsigned char *str) {
    // djb2 hash function, taken from http://www.cse.yorku.ca/~oz/hash.html
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/*
 *  Code for multipart-form decoding
 */
char *fgets_s(char *str, size_t size, FILE *fp)  // From qDecoder
{
    int c;
    char *ptr;

    for (ptr = str; size > 1; size--) {
        c = fgetc(fp);
        if (c == EOF) break;
        *ptr++ = (char)c;
        if (c == '\n') break;
    }

    *ptr = '\0';
    if (strlen(str) == 0) return NULL;

    return str;
}

char *strcpy_s(char *dst, size_t size, const char *src)  // From qDecoder
{
    if (dst == NULL || size == 0 || src == NULL) return dst;

    size_t copylen = strlen(src);
    if (copylen >= size) copylen = size - 1;
    memmove((void *)dst, (void *)src, copylen);
    dst[copylen] = '\0';

    return dst;
}

char *strtrim(char *str)  // From qDecoder
{
    int i, j;

    if (str == NULL) return NULL;
    for (j = 0; str[j] == ' ' || str[j] == '\t' || str[j] == '\r' || str[j] == '\n'; j++);
    for (i = 0; str[j] != '\0'; i++, j++) str[i] = str[j];
    for (i--; (i >= 0) && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'); i--);
    str[i+1] = '\0';

    return str;
}

char *strunchar(char *str, char head, char tail)  // From qDecoder
{
    if (str == NULL) return NULL;

    int len = strlen(str);
    if (len >= 2 && str[0] == head && str[len-1] == tail) {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }

    return str;
}


#define CONST_STRLEN(x) (sizeof(x) - 1)
#define _Q_MULTIPART_CHUNK_SIZE     (16 * 1024)
static char * parse_multipart_value_into_memory(char *boundary, int *valuelen,
        bool *finish)  // From qDecoder
{
    char boundaryEOF[256], rnboundaryEOF[256];
    char boundaryrn[256], rnboundaryrn[256];
    int  boundarylen, boundaryEOFlen;

    char *value;
    int  length;
    int  c, c_count, mallocsize;

    // set boundary strings
    snprintf(boundaryEOF, sizeof(boundaryEOF), "%s--", boundary);
    snprintf(rnboundaryEOF, sizeof(rnboundaryEOF), "\r\n%s--", boundary);
    snprintf(boundaryrn, sizeof(boundaryrn), "%s\r\n", boundary);
    snprintf(rnboundaryrn, sizeof(rnboundaryrn), "\r\n%s\r\n", boundary);

    boundarylen    = strlen(boundary);
    boundaryEOFlen = strlen(boundaryEOF);

    for (value = NULL, length = 0, mallocsize = _Q_MULTIPART_CHUNK_SIZE, c_count = 0;
         (c = fgetc(stdin)) != EOF; ) {
        if (c_count == 0) {
            value = (char *)malloc(sizeof(char) * mallocsize);
            if (value == NULL) {
                *finish = true;
                return NULL;
            }
        } else if (c_count == mallocsize - 1) {
            char *valuetmp;

            mallocsize *= 2;

            // Here, we do not use realloc(). Because sometimes it is unstable.
            valuetmp = (char *)malloc(sizeof(char) * mallocsize);
            if (valuetmp == NULL) {
                free(value);
                *finish = true;
                return NULL;
            }
            memcpy(valuetmp, value, c_count);
            free(value);
            value = valuetmp;
        }
        value[c_count++] = (char)c;

        // check end
        if ((c == '\n') || (c == '-')) {
            value[c_count] = '\0';

            if ((c_count - (2 + boundarylen + 2)) >= 0) {
                if (!strcmp(value + (c_count - (2 + boundarylen + 2)), rnboundaryrn)) {
                    value[c_count - (2 + boundarylen + 2)] = '\0';
                    length = c_count - (2 + boundarylen + 2);
                    break;
                }
            }
            if ((c_count - (2 + boundaryEOFlen)) >= 0) {
                if (!strcmp(value + (c_count - (2 + boundaryEOFlen)), rnboundaryEOF)) {
                    value[c_count - (2 + boundaryEOFlen)] = '\0';
                    length = c_count - (2 + boundaryEOFlen);
                    *finish = true;
                    break;
                }
            }

            // For MS Explore on MAC
            if ((c_count - (boundarylen + 2)) == 0) {
                if (!strcmp(value, boundaryrn)) {
                    value[0] = '\0';
                    length = 0;
                    break;
                }
            }
            if ((c_count - boundaryEOFlen) == 0) {
                if (!strcmp(value, boundaryEOF)) {
                    value[0] = '\0';
                    length = 0;
                    *finish = true;
                    break;
                }
            }
        }
    }

    if (c == EOF) {
        // DEBUG("Broken stream.");
        if (value != NULL) free(value);
        *finish = true;
        return NULL;
    }

    *valuelen = length;
    return value;
}

// static char *_parse_multipart_value_into_disk(const char *boundary,
//         const char *savedir, const char *filename, int *filelen, bool *finish)  // From qDecoder
// {
//     char boundaryEOF[256], rnboundaryEOF[256];
//     char boundaryrn[256], rnboundaryrn[256];
//     int  boundarylen, boundaryEOFlen;

//     // input
//     char buffer[_Q_MULTIPART_CHUNK_SIZE];
//     int  bufc;
//     int  c;

//     // set boundary strings
//     snprintf(boundaryEOF, sizeof(boundaryEOF), "%s--", boundary);
//     snprintf(rnboundaryEOF, sizeof(rnboundaryEOF), "\r\n%s--", boundary);
//     snprintf(boundaryrn, sizeof(boundaryrn), "%s\r\n", boundary);
//     snprintf(rnboundaryrn, sizeof(rnboundaryrn), "\r\n%s\r\n", boundary);

//     boundarylen    = strlen(boundary);
//     boundaryEOFlen = strlen(boundaryEOF);

//     // open temp file
//     char upload_path[PATH_MAX];
//     snprintf(upload_path, sizeof(upload_path), "%s/q_XXXXXX", savedir);

//     int upload_fd = mkstemp(upload_path);
//     if (upload_fd < 0) {
//         DEBUG("Can't open file %s", upload_path);
//         *finish = true;
//         return NULL;
//     }

//     // change permission
//     fchmod(upload_fd, DEF_FILE_MODE);

//     // read stream
//     bool ioerror = false;
//     int upload_length;
//     for (upload_length = 0, bufc = 0; (c = fgetc(stdin)) != EOF; ) {
//         if (bufc == sizeof(buffer) - 1) {
//             // save
//             ssize_t leftsize = boundarylen + 8;
//             ssize_t savesize = bufc - leftsize;
//             ssize_t saved = write(upload_fd, buffer, savesize);
//             if (saved <= 0) {
//                 ioerror = true; 
//                 break;
//             }
//             leftsize = bufc - saved;
//             memcpy(buffer, buffer+saved, leftsize);
//             bufc = leftsize;
//         }
//         buffer[bufc++] = (char)c;
//         upload_length++;

//         // check end
//         if ((c == '\n') || (c == '-')) {
//             buffer[bufc] = '\0';

//             if ((bufc - (2 + boundarylen + 2)) >= 0) {
//                 if (!strcmp(buffer + (bufc - (2 + boundarylen + 2)), rnboundaryrn)) {
//                     bufc          -= (2 + boundarylen + 2);
//                     upload_length -= (2 + boundarylen + 2);
//                     break;
//                 }
//             }
//             if ((bufc - (2 + boundaryEOFlen)) >= 0) {
//                 if (!strcmp(buffer + (bufc - (2 + boundaryEOFlen)), rnboundaryEOF)) {
//                     bufc          -= (2 + boundaryEOFlen);
//                     upload_length -= (2 + boundaryEOFlen);
//                     *finish = true;
//                     break;
//                 }
//             }

//             // For MS Explore on MAC
//             if (upload_length == bufc) {
//                 if ((bufc - (boundarylen + 2)) == 0) {
//                     if (!strcmp(buffer, boundaryrn)) {
//                         bufc = 0;
//                         upload_length = 0;
//                         break;
//                     }
//                 }
//                 if ((bufc - boundaryEOFlen) == 0) {
//                     if (!strcmp(buffer, boundaryEOF)) {
//                         bufc = 0;
//                         upload_length = 0;
//                         *finish = true;
//                         break;
//                     }
//                 }
//             }
//         }
//     }

//     // save rest
//     while (bufc > 0) {
//         ssize_t saved = write(upload_fd, buffer, bufc);
//         if (saved <= 0) {
//             ioerror = true;
//             break;
//         }
//         bufc -= saved;
//     }
//     close(upload_fd);

//     // error occured
//     if (c == EOF || ioerror == true) {
//         DEBUG("I/O error. (errno=%d)", (ioerror == true) ? errno : 0);
//         *finish = true;
//         return NULL;
//     }

//     // succeed
//     *filelen = upload_length;
//     return strdup(upload_path);
// }

query_var * parse_multipart(query_var *qv)  // From qDecoder
{
    char buf[MAX_LINEBUF];
    int  amount = 0;

    /*
     * For parse multipart/form-data method
     */
    char boundary_orig[256];
    char boundary[256], boundaryEOF[256];

    // Force to check the boundary string length to defense overflow attack
    int maxboundarylen = CONST_STRLEN("--");
    maxboundarylen += strlen(strstr(getenv("CONTENT_TYPE"), "boundary=")
                             + CONST_STRLEN("boundary="));
    maxboundarylen += CONST_STRLEN("--");
    maxboundarylen += CONST_STRLEN("\r\n");
    if (maxboundarylen >= sizeof(boundary)) {
        return qv;
    }

    // find boundary string - Hidai Kenichi made this patch for handling quoted boundary string
    strcpy_s(boundary_orig, sizeof(boundary_orig),
              strstr(getenv("CONTENT_TYPE"), "boundary=") + CONST_STRLEN("boundary="));
    strtrim(boundary_orig);
    strunchar(boundary_orig, '"', '"');
    snprintf(boundary, sizeof(boundary), "--%s", boundary_orig);
    snprintf(boundaryEOF, sizeof(boundaryEOF), "--%s--", boundary_orig);

    // check boundary
    do {
        if (fgets_s(buf, sizeof(buf), stdin) == NULL) {
            return qv;
        }
        strtrim(buf);
    } while (!strcmp(buf, "")); // skip blank lines

    // check starting boundary mark
    if (strcmp(buf, boundaryEOF) == 0) {
        // empty contents
        return qv;
    } else if (strcmp(buf, boundary) != 0) {
        return qv;
    }

    // // check file save mode
    // bool upload_filesave = false; // false: into memory, true: into file
    // const char *upload_basepath = request->getstr(request, "_Q_UPLOAD_BASEPATH", false);
    // if (upload_basepath != NULL) upload_filesave = true;

    bool finish;
    for (finish = false; finish == false; amount++) {
        char *name = NULL, *value = NULL, *filename = NULL, *contenttype = NULL;
        int valuelen = 0;

        // parse header
        while (fgets_s(buf, sizeof(buf), stdin)) {
            strtrim(buf);
            if (!strcmp(buf, "")) break;
            else if (!strncasecmp(buf, "Content-Disposition: ", CONST_STRLEN("Content-Disposition: "))) {
                int c_count;

                // get name field
                name = strdup(buf + CONST_STRLEN("Content-Disposition: form-data; name=\""));
                for (c_count = 0; (name[c_count] != '\"') && (name[c_count] != '\0'); c_count++);
                name[c_count] = '\0';

                // get filename field
                if (strstr(buf, "; filename=\"") != NULL) {
                    int erase;
                    filename = strdup(strstr(buf, "; filename=\"") + CONST_STRLEN("; filename=\""));
                    for (c_count = 0; (filename[c_count] != '\"') && (filename[c_count] != '\0'); c_count++);
                    filename[c_count] = '\0';
                    // remove directory from path, erase '\'
                    for (erase = 0, c_count = strlen(filename) - 1; c_count >= 0; c_count--) {
                        if (erase == 1) filename[c_count] = ' ';
                        else {
                            if (filename[c_count] == '\\') {
                                erase = 1;
                                filename[c_count] = ' ';
                            }
                        }
                    }
                    strtrim(filename);

                    // empty attachment
                    if (!strcmp(filename, "")) {
                        free(filename);
                        filename = NULL;
                    }
                }
            } else if (!strncasecmp(buf, "Content-Type: ", CONST_STRLEN("Content-Type: "))) {
                contenttype = strdup(buf + CONST_STRLEN("Content-Type: "));
                strtrim(contenttype);
            }
        }

        // check
        if (name == NULL) {
            continue;
        }

        // get value
        // if (filename != NULL && upload_filesave == true) {
        //     char *tp, *savename = strdup(filename);
        //     for (tp = savename; *tp != '\0'; tp++) {
        //         if (*tp == ' ') *tp = '_'; // replace ' ' to '_'
        //     }
        //     value = _parse_multipart_value_into_disk(
        //                 boundary, upload_basepath, savename, &valuelen, &finish);
        //     free(savename);

        //     if (value != NULL) request->putstr(request, name, value, false);
        //     else request->putstr(request, name, "(parsing failure)", false);
        // } else {
            value = parse_multipart_value_into_memory(boundary, &valuelen, &finish);

            //printf("%s :: %s\n", name, value);

            bb_cgi_add_var(&qv, name, value, valuelen+1);

            // if (value != NULL) request->put(request, name, value, valuelen+1, false);
            // else request->putstr(request, name, "(parsing failure)", false);
        // }

        // store additional information
        if (value != NULL && filename != NULL) {
            char ename[255+10+1];

            // store data length, 'NAME.length'
            snprintf(ename, sizeof(ename), "%s.length", name);
            char vasi[50];
            sprintf(vasi, "%d", valuelen);
            bb_cgi_add_var(&qv, ename, vasi, strlen(vasi)+1);

            // store filename, 'NAME.filename'
            snprintf(ename, sizeof(ename), "%s.filename", name);
            bb_cgi_add_var(&qv, ename, filename, strlen(filename)+1);

            // store contenttype, 'NAME.contenttype'
            snprintf(ename, sizeof(ename), "%s.contenttype", name);
            bb_cgi_add_var(&qv, ename, ((contenttype!=NULL)?contenttype:""), strlen(((contenttype!=NULL)?contenttype:""))+1);

            // if (upload_filesave == true) {
            //     snprintf(ename, sizeof(ename), "%s.savepath", name);
            //     bb_cgi_add_var(&qv, ename, value);
            // }
        }

        // free resources
        if (name != NULL) free(name);
        if (value != NULL) free(value);
        if (filename != NULL) free(filename);
        if (contenttype != NULL) free(contenttype);
    }

    return qv;
}

query_var * bb_cgi_get_post(query_var *qv) {
    if (strcmp(GET_ENV_VAR("REQUEST_METHOD"), "POST") != 0) {
        return qv;
    }

    char *ct = GET_ENV_VAR("CONTENT_TYPE");
    if (ct == NULL)
    {
        return qv;
    }

    char *t = strstr(ct, "application/x-www-form-urlencoded");
    if (t != NULL)
    {
        qv = bb_cgi_get_post_simple(qv);
    } else {
        t = strstr(ct, "multipart/form-data");
        if (t != NULL) {
            qv = parse_multipart(qv);
        }
    }

    return qv;
}

/*
 * Magic numbers for checking filetypes
 */
int is_image_file(char *buf)
{   
    const unsigned char magic_numbers[6][4] = {
        // JPG
        {0xFF, 0xD8, 0xFF, 0xDB},
        {0xFF, 0xD8, 0xFF, 0xE0},
        {0xFF, 0xD8, 0xFF, 0xEE},
        {0xFF, 0xD8, 0xFF, 0xE1},
        // PNG
        {0x89 ,0x50 ,0x4E, 0x47},
        // GIF
        {0x47, 0x49, 0x46, 0x38}
    };

    for (int i = 0; i < 6; i++) {
        if (memcmp(buf, magic_numbers[i], 4) == 0) {
            return 1;
        }
    }

    return 0;
}
