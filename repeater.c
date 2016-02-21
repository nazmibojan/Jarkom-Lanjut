#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVERPORT 8888
#define MAXBUF 1024

int main() {
	int sockudp;
	int lenReceiver, lenSender;
	struct sockaddr_in addrServer, addrReceiver, addrSender;
	int retval;

	sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockudp == -1){
		fprintf(stderr, "Could not create socket!\n");
		exit(1);
	}

	//Bind to Socket
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVERPORT);

	retval = bind(sockudp, (struct sockaddr*)&addrServer, sizeof(addrServer));
	if (retval == -1){
		fprintf(stderr, "Could not bind to socket!\n");
		exit(1);
	}
	
	printf("Socket Tersedia \n");

	struct timeval tv;
	tv.tv_sec = 30;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	
	for(;;){
		int i, senderCounter, readCounter, writeCounter, sendName, confirmation, ack;
		int numpack;
		char* bufptr;
		char buf[MAXBUF+2];
		char filename[MAXBUF];
		char senderreq[1];
		lenReceiver = sizeof(addrReceiver);
		lenSender = sizeof(addrSender);
		i = 0;
		
		//Receive announcement from sender
		if ((senderCounter = recvfrom(sockudp, senderreq, 1, 0, (struct sockaddr*)&addrSender, &lenSender)) > 0){
				i += senderCounter;
				printf("Sender Ready to Transmit Data! \n");
		}
		if (senderCounter == -1){
			fprintf(stderr, "Could not recognize request from Sender!\n");
			close(sockudp);
			exit(1);
		}

		//receive request from receiver
		if ((readCounter = recvfrom(sockudp, filename, MAXBUF, 0, (struct sockaddr*)&addrReceiver, &lenReceiver)) > 0){
				i += readCounter;
		}
		if (readCounter == -1){
			fprintf(stderr, "Could not recognize request from Receiver!\n");
			close(sockudp);
			continue;
		}
		
		printf("Receiver requests file %s\n from Receiver", filename);
				
		//Forward Filename to Sender
		sendName = sendto(sockudp, filename, MAXBUF, 0, (struct sockaddr*)&addrSender, sizeof(addrSender));
		if (sendName == -1){
			fprintf(stderr, "Could not forward filename to Sender!\n");
			close(sockudp);
			continue;
		}

		//Start as Repeater until EOF
		readCounter = 0;
		writeCounter = 0;
		
		int isSend = 0;
		while (!isSend){
	
			memset(buf, 0, (MAXBUF+2));

			//Receive data from Sender
			readCounter = recvfrom(sockudp, buf, (MAXBUF+2), 0, (struct sockaddr*)&addrSender, &lenSender);
			if (readCounter == -1){
				fprintf(stderr, "Could not receive data from Sender!\n");
				close(sockudp);
				continue;
			}

			numpack++;
			
			//Forward Data to Receiver
			printf("Forward paket ke - %d to Receiver! \n", numpack);
			writeCounter = sendto(sockudp, buf, (MAXBUF+2), 0, (struct sockaddr*)&addrReceiver, sizeof(addrReceiver));
			if (writeCounter == -1){
				fprintf(stderr, "Could not send data to Receiver!\n");
				close(sockudp);
				continue;
			}

			// Check the EOF
			if(buf[0] == '#' && buf[1] == '#' && buf[2] == '#' && buf[3] == '#' && buf[4] == '#'){
					printf("End of File! \n");
					printf("Data selesai dikirim!");
					isSend = 1;
			}
		}
	close (sockudp);
	exit(1);
	return 0;
	}
}