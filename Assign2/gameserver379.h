#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h> // For ints
#include <stdlib.h> // For atoi
#include <sys/types.h> // For sockaddr
#include <sys/socket.h> // For sockets
#include <netinet/in.h> //For sockaddr_in
#include <string.h> // For strings
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h> //For polling sockets
#include <unistd.h>
#include <signal.h>
