#!/bin/sh
rm bittyblog.db
sqlite3 bittyblog.db "CREATE TABLE \`pages\` (\`id\` INTEGER NOT NULL PRIMARY KEY UNIQUE, \`name_id\` TEXT NOT NULL UNIQUE, \`name\` TEXT, \`style\` INTEGER);"
sqlite3 bittyblog.db "CREATE TABLE \`posts\` (\`id\` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \`page_id\` INTEGER, \`user_id\` INTEGER, \`title\` TEXT, \`text\` TEXT, \`markdown\` TEXT, \`time\` INTEGER NOT NULL, \`extra\` TEXT, \`byline\` TEXT, \`thumbnail\` TEXT, \`visible\` INTEGER, FOREIGN KEY(\`page_id\`) REFERENCES pages ( id ));"
sqlite3 bittyblog.db "CREATE TABLE \`settings\` (\`name\` TEXT NOT NULL UNIQUE, \`value\` TEXT, PRIMARY KEY(name));"
#sqlite3 bittyblog.db "CREATE TABLE \`users\` (\`email\` TEXT NOT NULL UNIQUE, \`password\` TEXT NOT NULL, \`session\` TEXT UNIQUE, PRIMARY KEY(email));"
sqlite3 bittyblog.db "CREATE TABLE \`users\` (\`id\` INTEGER NOT NULL PRIMARY KEY UNIQUE, \`email\` TEXT NOT NULL UNIQUE, \`password\` TEXT NOT NULL, \`name_id\` TEXT NOT NULL UNIQUE, \`name\` TEXT, \`about\` TEXT, \`thumbnail\` TEXT, \`session\` TEXT UNIQUE);"
sqlite3 bittyblog.db "CREATE TABLE \`tags\` (\`id\` INTEGER NOT NULL PRIMARY KEY UNIQUE,\`tag\` TEXT NOT NULL UNIQUE);"
sqlite3 bittyblog.db "CREATE TABLE \`tags_relate\` (\`tag_id\` INTEGER NOT NULL, \`post_id\` INTEGER NOT NULL, PRIMARY KEY(tag_id,post_id));"
sqlite3 bittyblog.db "CREATE TABLE \`tags_pages_relate\` (\`tag_id\` INTEGER NOT NULL, \`page_id\` INTEGER NOT NULL, PRIMARY KEY(tag_id,page_id));"
sqlite3 bittyblog.db "CREATE TABLE \`last_update\` (\`time\` INTEGER NOT NULL);"

sqlite3 bittyblog.db "INSERT INTO pages (name_id, name, style) VALUES ('blog', 'Blog', 1), ('contact', 'Contact', 2);"
sqlite3 bittyblog.db "INSERT INTO posts (page_id, user_id, title, time, text, markdown, byline, visible) VALUES (1, 1, 'Your First Post', 1551632315, '<p>Here is where your blog post text would go.</p>', 'Here is where your blog post text would go.', 'Describe your blog post in one sentence', 1), (2, 1, 'Contact', 1551632315, '<p>Tell your readers where to contact you</p>', 'Tell your readers where to contact you', '', 1);"
sqlite3 bittyblog.db "INSERT INTO last_update (time) VALUES(CAST(strftime('%s', 'now') AS INT));"

echo Database created
echo Create an administrator account
echo Username:
read username
echo Password:
read password

sqlite3 bittyblog.db "INSERT INTO users (email, name_id, name, password) VALUES ('$username','$username','$username','$password');"

echo .. Creating docroot
mkdir www
mkdir www/cgi-bin
echo .. Compiling bittyblog
sed -i "s@~@${PWD}@g" bittyblog/config.h
make all
echo .. Copying executables
mv bb.cgi www/cgi-bin/bb.cgi
mv bbadmin.cgi www/cgi-bin/bbadmin.cgi
echo .. Copying images, and css
cp -r css www/
cp -r images www/

echo Setup complete. Please configure your webserver.
