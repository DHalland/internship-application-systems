/* Code written by Dylan Halland - April 2020	*/

/* includes support for IPv4 and IPv6, based on the given hostname/ip-address	

/* -- Sources Referenced --
1.	https://www.geeksforgeeks.org/ping-in-c/
2.	https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
3.	https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.hala001/sendto.htm
4	http://sotodayithought.blogspot.com/2010/03/simple-ping-implementation-in-c.html
5. 	https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html

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
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
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

void pingIPAddress(char *dest) {
	/* define all necessary variables*/
	struct icmp *pkt;
	char packet[ICMP_PKT];
	int i, byte, sent, received, skt, src_len, reply;
	double curr_time;
	long double rtt, total_time;

	struct addrinfo res;
	struct sockaddr_in *addr;
	struct sockaddr source;
	struct timespec t0, t1, start, end;

	int icmp_num = 0, icmp_received = 0, failed = 0;

	signal(SIGINT, stopPinging);

	/* initial socket and address info. setup */
	res = getIPAddress(dest);
	addr = (struct sockaddr_in*)res.ai_addr;
	skt = createRawSocket(addr);

	printf("PING %s (%s) %d(%d) bytes of data.\n", dest, inet_ntoa(addr->sin_addr), PINGCONST_1, PINGCONST_2);

	clock_gettime(CLOCK_MONOTONIC, &start);
	while (ping) {

		pkt = (struct icmp *) packet;
		memset(pkt, 0, sizeof(packet));

		pkt->icmp_type = ICMP_ECHO;
		pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

		clock_gettime(CLOCK_MONOTONIC, &t0);

		if ((sent = sendto(skt, pkt, sizeof(pkt), 0,
		   (struct sockaddr*)addr, sizeof(*addr))) < 0) { //3406 error code prevents blocking
			printf("\nRequest packet failed to send\n");
		}
		else
			icmp_num += 1;

		src_len = sizeof(source);

		if ((received = recvfrom(skt, packet, sizeof(packet), 0, 
			&source, &src_len)) <= 0) {
			fprintf(stderr, "Reply packet failed to arrive\n");
		}
		else {
			clock_gettime(CLOCK_MONOTONIC, &t1);
			curr_time = ((t1.tv_nsec - t0.tv_nsec))/1000000.0;
			rtt = ((t1.tv_sec - t0.tv_sec) * 1000.0) + curr_time;

			if (packet[IP_HEADER] != ICMP_ECHOREPLY)
				fprintf(stderr, "Incorrect response packet type received \n");
			else {
				icmp_received += 1;
				printf("%d bytes received from %s: icmp_seq=%d ttl=%d time=%.1Lf ms\n", BYTES, 
					inet_ntoa(addr->sin_addr), icmp_num, (unsigned char)packet[TTL_INDEX], rtt);
			}
		}
		packet[0] = '\0';
		wait(DELAY);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	total_time = ((end.tv_nsec - start.tv_nsec))/1000000.0;

	// subtract 1000ms for the extra call to my delay function - wait()
	total_time += ((end.tv_sec - start.tv_sec) * 1000.0) - 1000; 
	printResults(icmp_received, icmp_num, total_time, dest);
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

void printResults(int icmp_received, int icmp_num, long double time, char *dest) {
	float loss = ((double)(icmp_num - icmp_received) / icmp_num) * 100;
	printf("\n--- %s ping statistics ---\n", dest);
	printf("%d packets transmitted, %d received, %d%% packet loss, time %dms\n", 
		icmp_num, icmp_received, (int)loss, (int)time);
}

int createRawSocket(struct sockaddr_in *addr) {
	int skt = socket(addr->sin_family, SOCK_RAW, 1); 
	if (skt < 0) { 
		fprintf(stderr, "Socket file descriptor not received (run using root privileges)\n"); 
		exit(-1); 
	}
	return skt;
}

struct icmp createPacket(struct icmp *pkt) {
	return *pkt;
}

void wait(int seconds) {
	int us = 1000000 * seconds; //amount to delay by, in microseconds
	clock_t start = clock();

	while (clock() < start + us);
}

void stopPinging(int signum) {
	ping = 0;
}

/*
 * Copyright (c) 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * The following checksum code is derived from software contributed to Berkeley by
 * Mike Muuss.
*/
unsigned short in_cksum(unsigned short *addr, int len) {
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = ~sum;                          /* truncate to 16 bits */
	return(answer);
}
