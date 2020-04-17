/* Code written by Dylan Halland - April 2020	*/

#ifndef __PING_H__
#define __PING_H__

#define DELAY 1

#define ECHO_REPLY 0
#define ECHO_REQUEST 8
#define TTL_EXPIRED 11

#define HEADER 4 //set a max value for the echo request pkt message

typedef struct packet {
	uint8_t type; // type of ICMP packet (echo reply/request, or neither)
	uint8_t code; // additional type info (TTL expired, etc.)
	uint16_t checksum; // checksum, will be set to 0

	char buf[HEADER]; //the actual message to be sent

}Packet; 

void checkArgs(int argc, char *argv[]);

struct addrinfo getIPAddress(char *dest);

void pingIPAddress(char *dest);

Packet createPacket(Packet pkt);
int createRawSocket(struct sockaddr_in *addr);

void wait(int ms);

void stopPinging(int signum);

#endif