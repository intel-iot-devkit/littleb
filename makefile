all:
	gcc -g -O0 src/sdbus-client.c -o sdbus-client `pkg-config --cflags --libs libsystemd`
