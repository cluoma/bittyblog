#!/bin/sh
sqlite3 bittyblog.db "CREATE TABLE \`pages\` (\`id\` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \`name_id\` TEXT NOT NULL UNIQUE, \`name\` TEXT, \`style\` INTEGER);"
sqlite3 bittyblog.db "CREATE TABLE \`posts\` (\`id\` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \`page_id\` INTEGER NOT NULL, \`title\` TEXT, \`text\` TEXT, \`time\` INTEGER NOT NULL, \`extra\` TEXT, \`byline\` TEXT, \`thumbnail\` TEXT, \`visible\` INTEGER, FOREIGN KEY(\`page_id\`) REFERENCES pages ( id ));"
sqlite3 bittyblog.db "CREATE TABLE \`settings\` (\`name\` TEXT NOT NULL UNIQUE, \`value\` TEXT, PRIMARY KEY(name));"
sqlite3 bittyblog.db "CREATE TABLE \`users\` (\`email\` TEXT NOT NULL UNIQUE, \`password\` TEXT NOT NULL, \`session\` TEXT UNIQUE, PRIMARY KEY(email));"

sqlite3 bittyblog.db "INSERT INTO pages (name_id, name, style) VALUES ('blog', 'Blog', 1), ('contact', 'Contact', 2);"

sqlite3 bittyblog.db "INSERT INTO posts (page_id, title, time, text, byline, visible) VALUES (1, 'Your First Post', 1551632315, 'Here is where your blog post text would go.', 'Describe your blog post in one sentence', 1), (2, 'Contact', 1551632315, 'Tell your readers where to contact you', '', 1);"

echo Database created
echo Create an administrator account
echo Username:
read username
echo Password:
read password

sqlite3 bittyblog.db "INSERT INTO users (email, password) VALUES ('$username','$password');"

echo .. Creating docroot
mkdir www
mkdir www/cgi-bin
echo .. Compiling bittyblog
sed -i "s@~@${PWD}@g" bittyblog/config.h
make all
echo .. Copying executables
mv bb.cgi www/cgi-bin/bb.cgi
mv bbadmin.cgi www/cgi-bin/bbadmin.cgi
echo .. Copying fonts, images, and css
cp -r fonts www/
cp -r css www/
cp -r images www/

echo Setup complete. Please configure your webserver.