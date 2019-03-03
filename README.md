# bittyblog

A blogging platform using C+SQLite3 that runs as a CGI app.

# Installation

1. Open bittyblog/config.h and fill in the appropriate information
    - COPYRIGHT_OWNER: appears in the page footer next to a copywrite message
    - NAVBAR_TITLE: is the text that appears in the brand of the navbar
    - HTML_TITLE: is the text that appears in the browser tab
    - ABOUT: this is a short text describing your blog that is displayed on the homepage
    - POSTS_PER_PAGE: this is how many post preview will be shown during pagination
2. run setup.sh
    - supply a username and password
    - bittyblog is compiled and all necessary files are moved to a newly created www/ folder in the current directory
3. configure you webserver to point to the created www directory and give it read/write access
4. customize layout with the template and css files

# Credits

Code and inspiration from the following projects:
 * [parson](https://github.com/kgabis/parson)
 * [magnum](https://github.com/fletcher/magnum)
 * [qdecoder](https://github.com/wolkykim/qdecoder)
 * [tinydir](https://github.com/cxong/tinydir)
