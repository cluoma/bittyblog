# bittyblog

A blogging platform using C+SQLite3 that runs as a CGI app.

# Installation

1. Open bittyblog/config.h and fill in the appropriate information
    - COPYRIGHT_OWNER: appears in the page footer next to a copywrite message
    - NAVBAR_TITLE: is the text that appears in the brand of the navbar
    - HTML_TITLE: is the text that appears in the browser tab
    - POSTS_PER_PAGE: this is how many post preview will be shown during pagination
2. run setup.sh
    - supply a username and password
    - bittyblog is compiled and all necessary files are moved to a newly created www/ folder in the current directory
3. configure you webserver to point to the created www directory and give it read/write access
4. customize layout with the template and css files

# Credits

Code and inspiration from the following projects:
 * [parson](https://github.com/kgabis/parson) -- [MIT License](https://opensource.org/licenses/mit-license.php)
 * [magnum](https://github.com/fletcher/magnum) -- [MIT License](https://opensource.org/licenses/mit-license.php)
 * [qdecoder](https://github.com/wolkykim/qdecoder) -- [qDecoder License](https://github.com/wolkykim/qdecoder/blob/master/COPYING)
 * [tinydir](https://github.com/cxong/tinydir) -- [tinydir License](https://github.com/cxong/tinydir/blob/master/COPYING)
 * [bootstrap](https://getbootstrap.com/docs/3.4/css/) -- [MIT License](https://github.com/twbs/bootstrap/blob/v3-dev/LICENSE)
