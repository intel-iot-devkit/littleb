all:
	gcc -g -O0 src/littleb.c src/hello_littleb.c -o hello_littleb `pkg-config --cflags --libs libsystemd` -lpthread
