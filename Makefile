
PREFIX ?= /usr/local

all: doin.so

doin.so: doin.c
	$(CC) $(CFLAGS) -shared -fPIC -Wall -o $@ $< -ldl

clean:
	$(RM) doin.so

install: doin.so
	install -Dm755 doin.so $(DESTDIR)$(PREFIX)/bin/doin.so
	install -Dm755 doin    $(DESTDIR)$(PREFIX)/bin/doin

.PHONY: install
