# bittyblog

A tiny blogging platform with the following features:
* compiles to 2 executables and a single sqlite3 database for easy backups
* write blog posts in Markdown or html
* permalinks using URL rewrites
* fastCGI or classic CGI modes
* customize pages with mustache HTML templates
* simple cache for fast responses

# Installation

## Quick Setup

1. Open bittyblog/config.h and fill in the appropriate information
    - COPYRIGHT_OWNER: appears in the page footer next to a copywrite message
    - NAVBAR_TITLE: is the text that appears in the brand of the navbar
    - HTML_TITLE: is the text that appears in the browser tab
    - POSTS_PER_PAGE: this is how many post preview will be shown during pagination
2. run setup.sh
    - supply a username and password
    - bittyblog is compiled and all necessary files are moved to a newly created www/ folder in the current directory
3. configure you webserver to point to the created www directory and give it read/write access
    - Also give read/write access to template and database directory
4. customize layout with the template and css files

## Step-by-step Guide for lighttpd+fastCGI+caching

1. Ensure all pre-requisites are installed on your system:
    - `sqlite3 libsqlite3-dev zlib1g-dev libfastcgi lighttpd`

2. Clone bittyblog
    - `git clone https://github.com/cluoma/bittyblog`
    - `cd bittyblog/bittyblog`

3. Configure `config.h`
    - `nano config.h`

    * Basic Stuff
        - `COPYRIGHT_OWNER` this is intended to be the full name of your orginization, to be displayed on the footer next to copyright information
        - `NAVBAR_TITLE` will be displayed as the title of your blog in the navbar
        - `HTML_TITLE` this is the head title of your page, will be shown as title in the browser
    
    * Important directories
        - Leave this alone for default settings. Only change these if you want to store the sqlite3 database, templates, or images in a different location
        - `DB_PATH` file path of sqlite3 database
        - `IMAGE_PATH` file path of directory for storing uploaded images
        - `TEMPLATE_PATH` file path of page mustache page templates

    * Site Behaviour
        - `POSTS_PER_PAGE` how many posts will be shown per page with pagination
    
    * Cache Settings
        - bittyblog has the option to enable an in-memory cache. Please pay attention to your machines available memory when setting the cache size.
        - `USE_CACHE` set to 0 to disable, anything else will enable the cache
        - `CACHE_INIT_BUCKETS` the initial size of the cache. The cache will be expanded if needed, set to a low value.
        - `MAX_CACHE_BYTES` the maximum number of bytes of stored payload. Note that this does not count the memory needed for data structures. Actual memory usage will be 1.2-1.5 times higher than the number set here when the cache is full.
        - `CACHE_TIMEOUT_SECONDS` cache entries older than this will be removed if the cache starts to run out available space.

4. Enable fastCGI
    - `cd .. && nano Makefile`
    - set `FCGI` to `y` for fastCGI, anything else will compile without fastCGI support

5. Run Setup script
    - `./setup.sh`
    - Creates `bittyblog.db` with the given username and password in the current directory
    - Compiles `bb.cgi` and `bbadmin.cgi`
    - Moves everything into a new `www/` directory inside the current directory

6. Give `www-data` access to bittyblog
    - `cd .. && sudo sudo chown :www-data -R bittyblog/`

7. Configure lighttpd
    ```
    # Set document root to you bittyblog path
    server.document-root = "/path/to/bittyblog/www"
    ```
    ```
    # Add this if you want permalinks
    url.rewrite-if-not-file = (
        "^([^?]*)\??(.*)?$" => "/cgi-bin/bb.cgi?rewrite=$1&$2"
    )
    ```
    ```
    # Add this if you are using fastCGI
    fastcgi.server = (
        "bb.cgi" =>
        ((
	    "bin-path" => "/path/to/www/cgi-bin/bb.cgi",
            "socket" => "/var/run/lighttpd/bb.cgi.socket",
            "kill-signal" => 10,
            "max-procs" => 2
        )),
        "bbadmin.cgi" =>
        ((
            "bin-path" => "/path/to/www/cgi-bin/bbadmin.cgi",
            "socket" => "/var/run/lighttpd/bbadmin.cgi.socket",
            "kill-signal" => 10,
            "max-procs" => 1
        ))
    )
    ```
# Websites using bittyblog
* [cluoma.com](https://www.cluoma.com/) - personal blog
* [LinuxGameNetwork](https://www.linuxgame.net/) - Linux gaming news blog


# Credits

Code and inspiration from the following projects:
 * [parson](https://github.com/kgabis/parson) -- [MIT License](https://opensource.org/licenses/mit-license.php)
 * [magnum](https://github.com/fletcher/magnum) -- [MIT License](https://opensource.org/licenses/mit-license.php)
 * [MD4C](https://github.com/mity/md4c) -- [MIT License](https://opensource.org/licenses/mit-license.php)
 * [qdecoder](https://github.com/wolkykim/qdecoder) -- [qDecoder License](https://github.com/wolkykim/qdecoder/blob/master/COPYING)
 * [tinydir](https://github.com/cxong/tinydir) -- [tinydir License](https://github.com/cxong/tinydir/blob/master/COPYING)
