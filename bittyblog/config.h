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
#define DEFAULT_PAGE "blog"
#define DB_PATH "~/bittyblog.db"
#define IMAGE_PATH "~/www/images"
#define TEMPLATE_PATH "~/templates"

// Site Behaviour
#define POSTS_PER_PAGE 5     // How many blog posts should be shown per pagination

// Cache settings
// Only used when fastCGI is used
// Note: timemout only applies when hash runs out of space
// Note: cache bytes refers to saved data, it does not count memory
// used for the data structures. Multiply bytes by 1.2-1.5 for actual memory usage
#define USE_CACHE 0
#define CACHE_INIT_BUCKETS 10000
#define MAX_CACHE_BYTES 512000000
#define CACHE_TIMEOUT_SECONDS 60

#endif
