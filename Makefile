all:
	gcc -Wall -g main.c ./lib/httpd.c -o server

clean:
	rm -rf server*
