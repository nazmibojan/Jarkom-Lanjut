/*
Angga Pratama Putra
Nazmi Febrian
Topan Pratomo
-Computer Engineering, ITB-
*/

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned char      byte;    // Byte ekivalen char
typedef unsigned short int word16;  // 16-bit word ekivalen short int
typedef unsigned int       word32;  // 32-bit word ekivalen int

#define SERVERPORT 8888
#define MAXBUF 1024

byte checksum(byte *addr, word32 count);

int main() {
	int sockudp;
	int lenRepeater;
	struct sockaddr_in addrServer, addrRepeater;
	int retval;
	int sendercounter;
	char senderreq[1] = "?";

	sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockudp == -1){
		fprintf(stderr, "Could not create socket!\n");
		exit(1);
	}

	//Bind to Socket
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVERPORT);

	//Bind to Repeater Socket
	addrRepeater.sin_family = AF_INET;
	addrRepeater.sin_addr.s_addr = inet_addr("168.235.67.244");
	addrRepeater.sin_port = htons(SERVERPORT);

	retval = bind(sockudp, (struct sockaddr*)&addrServer, sizeof(addrServer));
	if (retval == -1){
		fprintf(stderr, "Could not bind to socket!\n");
		exit(1);
	}
	printf("socket tersedia \n");

	sendercounter = sendto(sockudp, senderreq, 1, 0, (struct sockaddr*)&addrRepeater, sizeof(addrRepeater));

	for(;;){
		int fd;
		int numdat, i, readCounter, writeCounter, confirmation;
		char* bufptr;
		char buf[MAXBUF+2];
		char filename[MAXBUF];
		byte check;
		lenRepeater = sizeof(addrRepeater);
		i = 0;
		
		//receive 1-read request socket//////
		if ((readCounter = recvfrom(sockudp, filename, MAXBUF, 0, (struct sockaddr*)&addrRepeater, &lenRepeater)) > 0){
				i += readCounter;
		}
		if (readCounter == -1){
			fprintf(stderr, "Could not find requested filename from Receiver!\n");
			close(sockudp);
			exit(1);
		}
		
		printf("Reading file %s\n", filename);
		fd = open(filename, O_RDONLY);
		if (fd == -1){
			fprintf(stderr, "Could not open file for reading!\n");
			close(sockudp);
			continue;
		}

		//sending data..
		numdat = 0;
		readCounter = 0;
		while((readCounter = read(fd, buf, MAXBUF)) > 0){
			
			writeCounter = 0;
			
			//Calculate checksum
			check = checksum(buf, readCounter);
			
			numdat++;
			//sprintf(buf[readCounter], "%d", numdat);
			buf[readCounter] = numdat;
			buf[readCounter+1] = check;
			
			//perubahan//
			bufptr = buf;
			while (writeCounter < readCounter){
				readCounter -= writeCounter;
				bufptr += writeCounter;
				writeCounter = sendto(sockudp, bufptr, (MAXBUF+2), 0, (struct sockaddr*)&addrRepeater, sizeof(addrRepeater));
				if (writeCounter == -1){
					fprintf(stderr, "Could not send data to Repeater!\n");
					continue;
				}
				printf("Data ke - %d : ukuran %d byte dengan checksum %02X\n", numdat, readCounter, check);
			}
			memset(buf, 0, MAXBUF+2);
			usleep(100000);
		}

		printf("End of File! \n");
		char finish[5] = "#####";		
		//send2
		sendto(sockudp, finish, strlen(finish), 0, (struct sockaddr*)&addrRepeater, sizeof(addrRepeater));
		
		close(fd);
		close (sockudp);
		exit(1);
	}
	
	return 0;
}

byte checksum(byte *addr, word32 count)
{
  register word32 sum = 0;

  // Main summing loop
  while(count > 0)
  {
    sum = sum + (*addr++);
	count = count - 1;
  }
  
  //bit masking;
  //printf("before mask = %d \n", sum);
  sum = (sum & 0xFF);
  //printf("after mask = %d \n", sum);
	
  return(~sum);
  
}
