all:
	gcc -Wall -g main.c ./lib/httpd.c -o httpd

clean:
	rm -rf httpd*
