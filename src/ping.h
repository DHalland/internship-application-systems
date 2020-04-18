/* Code written by Dylan Halland - April 2020	*/

#ifndef __PING_H__
#define __PING_H__

#define DELAY 1

// #define ECHO_REPLY 0
// #define ECHO_REQUEST 8
#define TTL_EXPIRED 11  // icmp code for expired ttl 

#define ICMP_PKT 128 // packet size, large enough for IP + ICMP payloads

#define IP_HEADER 20 //num of bytes used for ip portion of reply packet
#define BYTES 64 // num of bytes received from echo reply

#define PINGCONST_1 56 // constant number 1 seen at the beginning of every ping
#define PINGCONST_2 84 // another constant value at the top of every ping

#define TTL_INDEX 8

void checkArgs(int argc, char *argv[]);

void pingIPAddress(char *dest);
struct addrinfo getIPAddress(char *dest);

void printResults(int icmp_received, int icmp_num, long double time, char *dest); 

int createRawSocket(struct sockaddr_in *addr);

void wait(int ms);

void stopPinging(int signum);
unsigned short in_cksum(unsigned short *addr,int len);

#endif