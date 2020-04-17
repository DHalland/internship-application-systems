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

#include "ping.h"

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
	Packet pkt;
	int i, byte, sent, received, skt, src_len;
	double total_time;
	long double rtt;

	struct addrinfo res;
	struct sockaddr_in *addr;
	struct sockaddr source;
	struct timespec t0, t1;

	int icmp_seq = 1;

	signal(SIGINT, stopPinging);

	res = getIPAddress(dest);
	addr = (struct sockaddr_in*)res.ai_addr;
	skt = createRawSocket(addr);

	printf("%u\n", addr->sin_addr.s_addr);

	while (ping) {
		pkt = createPacket(pkt);
		// printf("X bytes from %s: icmp_seq:%d\n", inet_ntoa(addr->sin_addr), icmp_seq);

		clock_gettime(CLOCK_MONOTONIC, &t0);
		sent = sendto(skt, pkt.buf, sizeof(pkt.buf), 0,
		   (struct sockaddr*)addr, sizeof(*addr));
		if (sent <= 0) {
			fprintf(stderr, "Packet failed to send\n");
		}

		src_len = sizeof(source);

		received = recvfrom(skt, pkt.buf, sizeof(pkt.buf), 0, 
			&source, &src_len);
		if (received <= 0) {
			printf("Packet failed to recv\n");
		}
		else {
			clock_gettime(CLOCK_MONOTONIC, &t1);
			total_time = ((double)(t1.tv_nsec - t0.tv_nsec))/1000000.0;
			rtt = ((t1.tv_sec - t0.tv_sec) * 1000.0) + total_time; 


			if (pkt.buf[0] != 0) {
				printf("wrong type of icmp pkt received\n");
			}
			else {
				printf("echo reply received\n");
			}

		}

		printf("received: %c\n", pkt.buf[0]);
		icmp_seq += 1;
		pkt.buf[0] = '\0';
		wait(DELAY);
		// fflush(stdout);
	}
	printf("\n--- %s ping statistics ---\n", dest);
	// printf("family: %d\n", res.ai_family);
	// printf("socktype: %d\n", res.ai_socktype);
	// printf("protocol: %d\n", res.ai_protocol);
	// printf("addr len: %d\n", res.ai_addrlen);
	return;
}

int createRawSocket(struct sockaddr_in *addr) {
	int skt = socket(addr->sin_family, SOCK_STREAM, 0); //IPPROTO_ICMP); 
	if(skt < 0) { 
		fprintf(stderr, "\nSocket file descriptor not received!!\n"); 
		return 0; 
	} 
		printf("\nSocket file descriptor %d received\n", skt); 
	return skt;
}

Packet createPacket(Packet pkt) {
	pkt.type = ECHO_REQUEST;
	pkt.code = TTL_EXPIRED;
	pkt.checksum = 0;

	memcpy(pkt.buf, &pkt.type, 1);
	memcpy(&pkt.buf[1], &pkt.code, 1);
	memcpy(&pkt.buf[2], &pkt.checksum, 2);

	printf("%s\n", pkt.buf);

	return pkt;
}

void wait(int seconds) {
	int us = 1000000 * seconds; //amount to delay by, in microseconds
	clock_t start = clock();

	while (clock() < start + us);
}

void stopPinging(int signum) {
	ping = 0;


}
