//
//  config.h
//  bittyblog
//
//  Created by Colin Luoma on 2018-07-25.
//  Copyright © 2018 Colin Luoma. All rights reserved.
//
// Store global variables specific to your site here

#ifndef config_h
#define config_h

// Basic stuff
#define COPYRIGHT_OWNER "bittyblog"
#define NAVBAR_TITLE "bittyblog"
#define HTML_TITLE "bittyblog - a sample bittyblog"
#define DB_PATH "~/bittyblog.db"
#define IMAGE_PATH "~/www/images"
#define TEMPLATE_PATH "~/templates"

// Site Behaviour
#define POSTS_PER_PAGE 5     // How many blog posts should be shown per pagination

// Cache settings
#define USE_CACHE 0
#define CACHE_BUCKETS 5000
#define MAX_CACHE_BYTES 256000000
#define CACHE_TIMEOUT_SECONDS 60

#endif
