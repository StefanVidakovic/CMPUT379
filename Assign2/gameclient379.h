#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h> // For ints
#include <stdlib.h> // For atoi
#include <sys/types.h> // For sockaddr
#include <sys/socket.h> // For sockets
#include <netinet/in.h> // For sockaddr_in
#include <string.h> // For strings
#include <errno.h>
#include <sys/select.h> //For polling file
#include <curses.h> // For ncurses
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// #include <ncurses.h> // For ncurses
