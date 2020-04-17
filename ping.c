/* Code written by Dylan Halland - April 2020	*/

/* -- Sources Referenced --
1.	https://www.geeksforgeeks.org/ping-in-c/
2.	https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
3.	https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.hala001/sendto.htm

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "cloudflare.h"

int ping = 1; // global var used to exit main ping loop after Ctrl-C interrupt

int main(int argc, char *argv[]) {
	char *destination;
	char *address;

	checkArgs(argc, argv);

	destination = argv[1];
	pingIPAddress(destination);

	return 0; 
}

void checkArgs(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s [host-name OR ip-address] \n", argv[0]);
		exit(1);
	}
	return;
}

struct addrinfo getIPAddress(char *dest) {
	int error = 0; // used to check if hostname not available
	struct addrinfo hints;	
	struct addrinfo *res = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	// check if the "dest" input is a valid address (hostname or ip address)
	if ((error = getaddrinfo(dest, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "Error getaddrinfo (destination: %s): %s\n", dest, gai_strerror(error));
		exit(-1);
	}

	return *res;
}

void pingIPAddress(char *dest) {
	int i, byte;
	struct addrinfo res;
	struct sockaddr_in *addr;
	int icmp_seq = 1;

	signal(SIGINT, stopPinging);

	res = getIPAddress(dest);
	addr = (struct sockaddr_in*)res.ai_addr;
	printf("%u\n", addr->sin_addr.s_addr);

	while (ping) {
		wait(DELAY);
		printf("X bytes from %s: icmp_seq:%d\n", inet_ntoa(addr->sin_addr), icmp_seq);
		icmp_seq += 1;
		// fflush(stdout);
	}
	printf("\n--- %s ping statistics ---\n", dest);
	// printf("family: %d\n", res.ai_family);
	// printf("socktype: %d\n", res.ai_socktype);
	// printf("protocol: %d\n", res.ai_protocol);
	// printf("addr len: %d\n", res.ai_addrlen);
	return;
}

void wait(int seconds) {
	int us = 1000000 * seconds; //amount to delay by, in microseconds
	clock_t start = clock();

	while (clock() < start + us);
}

void stopPinging(int signum) {
	ping = 0;


}
