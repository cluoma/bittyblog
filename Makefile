CC=gcc
LIB=-L .
INC=-Imagnum/src -Isqlite3
WARN=-Wall -Wvla -Wno-format-truncation
FCGI=y

all: bb admin

bb: libMagnum.a libsqlite3.a
ifeq ($(FCGI),y)
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE -D_FCGI $(WARN) -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c bittyblog/cachemap.c bittyblog/mod_archives.c $(LIB) $(INC) -lsqlite3 -ldl -lz -lfcgi -lMagnum
else
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE $(WARN) -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c bittyblog/mod_archives.c $(LIB) $(INC) -lsqlite3 -ldl -lz -lMagnum
endif

admin: libMagnum.a libsqlite3.a
ifeq ($(FCGI),y)
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE -D_FCGI $(WARN) -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c bittyblog/cachemap.c $(LIB) $(INC) -lsqlite3 -ldl -lz -lfcgi -lMagnum
else
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE $(WARN) -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c $(LIB) $(INC) -lsqlite3 -ldl -lz -lMagnum
endif

install: bb admin
	mv bb.cgi www/cgi-bin/bb.cgi
	mv bbadmin.cgi www/cgi-bin/bbadmin.cgi

bbinstall: bb
	mv bb.cgi www/cgi-bin/bb.cgi

admininstall: admin
	mv bbadmin.cgi www/cgi-bin/bbadmin.cgi

.PHONY: test
test:
	gcc -c -o d_string.o -pg magnum/src/d_string.c
	gcc -c -o file.o -pg magnum/src/file.c
	gcc -c -o json.o -pg magnum/src/json.c
	gcc -c -o parson.o -pg magnum/src/parson.c
	gcc -c -o magnum.o -pg magnum/src/magnum.c
	ar rcs libMagnum.a d_string.o file.o json.o parson.o magnum.o
	$(CC) -o test_p2 -D_DEFAULT_SOURCE -Wall -Wvla -std=c99 -O3 test/test.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c sqlite3/sqlite3.c $(LIB) $(INC) -lpthread -ldl -lz -lMagnum -Isqlite3
	rm d_string.o file.o json.o parson.o magnum.o

libsqlite3.a:
	gcc -c -o libsqlite3.o -O3 -DSQLITE_THREADSAFE=0 sqlite3/sqlite3.c
	ar rcs libsqlite3.a libsqlite3.o
	rm libsqlite3.o

libMagnum.a: d_string.o file.o json.o parson.o magnum.o
	ar rcs libMagnum.a d_string.o file.o json.o parson.o magnum.o
	rm d_string.o file.o json.o parson.o magnum.o

d_string.o:
	gcc -c -o d_string.o -O3 magnum/src/d_string.c

file.o:
	gcc -c -o file.o -O3 magnum/src/file.c

json.o:
	gcc -c -o json.o -O3 magnum/src/json.c

parson.o:
	gcc -c -o parson.o -O3 magnum/src/parson.c

magnum.o:
	gcc -c -o magnum.o -O3 magnum/src/magnum.c

.PHONY : clean
clean:
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a libsqlite3.a
