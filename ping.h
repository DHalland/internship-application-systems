/* Code written by Dylan Halland - April 2020	*/

#ifndef __PING_H__
#define __PING_H__

#define DELAY 1

void checkArgs(int argc, char *argv[]);

struct addrinfo getIPAddress(char *dest);

void pingIPAddress(char *dest);

void wait(int ms);

void stopPinging(int signum);

#endif