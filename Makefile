all: doin.so

doin.so: doin.c
	$(CC) -shared -fPIC -Wall -o $@ $< -ldl

clean:
	$(RM) doin.so
