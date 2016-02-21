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

typedef unsigned char      byte;    // Byte ekivalen char
typedef unsigned short int word16;  // 16-bit word ekivalen short int
typedef unsigned int       word32;  // 32-bit word ekivalen int

#define SERVERPORT 8888
#define MAXBUF 1024

//prototype
byte checksum(byte *addr, word32 count);

int main(int argc, char* argv[]){
	int sockudp;
	int counter, confirmation;
	int fd;
	int lenRepeater;
	struct sockaddr_in addrRepeater, addrClient;
	char buf[MAXBUF+2];
	int failedgroup[MAXBUF];
	int retval;
	int numfailed;
	byte check;

	if (argc < 3){
		fprintf(stderr, "Usage: %s <ip address> <filename> [dest filename]\n", argv[0]);
		exit(1);
	}

	sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockudp == -1){
		fprintf(stderr, "Could not create socket!\n");
		exit(1);
	}

	addrRepeater.sin_family = AF_INET;
	addrRepeater.sin_addr.s_addr = inet_addr(argv[1]);
	addrRepeater.sin_port = htons(SERVERPORT);

	retval = sendto(sockudp, argv[2], strlen(argv[2])+1, 0, (struct sockaddr*)&addrRepeater, sizeof(addrRepeater));
	if (retval == -1){
		fprintf(stderr, "Could not send filename to server!\n");
		exit(1);
	}
	
	fd = open(argv[3], O_WRONLY | O_CREAT | O_APPEND);
	if (fd == -1){
		fprintf(stderr, "Could not open destination file, using stdout.\n");
		fd = 1;
	}
	lenRepeater = sizeof(addrRepeater);

	int isReceive = 1;
	numfailed = 0;
	
	while (isReceive){

		memset(buf, 0, (MAXBUF+2));

		//receive data from repeater
		counter = recvfrom(sockudp, buf, (MAXBUF+2), 0, (struct sockaddr*)&addrRepeater, &lenRepeater);
		if (counter == -1){
			fprintf(stderr, "Could not receive data from repeater!\n");
			exit(1);
		}

		check = checksum(buf, (counter-2));
		if(buf[0] == '#' && buf[1] == '#' && buf[2] == '#' && buf[3] == '#' && buf[4] == '#'){
			isReceive = 0;
			printf("End of File! \n");
		} else {
				if(check == buf[counter-1]){
					printf(" data ke - %d terverifikasi.. checksum = %02X \n", buf[counter-2], check);
                } else {
                	numfailed++;
                	printf("Terjadi kesalahan pada data ke %d\n", buf[counter-2]);
                	failedgroup[numfailed] = buf[counter-2];
                }
                write(fd, buf, (counter-2));
		}
	}

	int printfailed = 0;
	printf("Paket data yang salah adalah paket ke: ");
	for (printfailed; printfailed < numfailed; printfailed++){
		printf("%d, ", failedgroup[printfailed]);
	}
		
	char tes[256] = "chmod 777 ";
	strcat(tes, argv[3]);
	tes[strlen(tes)+1] = '\0';
	system(tes);
	close(sockudp);
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
  sum = (sum & 0xFF);
 	
  return(~sum);
}
