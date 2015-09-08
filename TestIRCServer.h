#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


char * user;
char * password;
char * host;
char * sport;
int port;

#define MAX_RESPONSE (10 * 1024)

int open_client_socket(char * host, int port);
int sendCommand(char *  host, int port, char * command, char * response);
void printUsage();
