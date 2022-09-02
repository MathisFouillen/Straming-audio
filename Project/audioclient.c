#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include "include/audio.h"
#include "include/filter.h"

#define SIZE 1024
#define TTL 2


int main(int argc, char * argv[]) {
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) { perror("file descriptor error"); exit(1); }
	
	//dest setup
	int port = 1234;
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); //local host id
	socklen_t flen = sizeof(struct sockaddr_in);
	
	//timeout setup
	fd_set read_set;
	struct timeval timeout;
	FD_ZERO(&read_set); /* Clear the read_set */
	FD_SET(fd, &read_set); /* Wait until fd is ready for reading */
	int nb;
	
	char buffer[SIZE];
	char previousBuffer[SIZE];
	
	//filters setup
	bool volume = false;
	double volumeValue;
	bool mono = false;
	bool echo = false;

	for(int i=2; i<argc; i++){
		if(strcmp(argv[i], "volume") == 0){
			volume = true;
			if(i == argc-1)
				volumeValue = 1;
			else
				volumeValue = strtod(argv[i+1], NULL);	
		}
			
		else if(strcmp(argv[i], "mono") == 0)
			mono = true;
		
		else if(strcmp(argv[i], "echo") == 0)
			echo = true;
	}
	
	
	//syn
	int err = sendto(fd, "syn", SIZE, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
	if(err < 0) { perror("sendto error"); exit(1); }
	//timeout check
	timeout.tv_sec = TTL;
	nb = select(fd+1, &read_set, NULL, NULL, &timeout);
	if(nb == 0) { //timeout
		printf("timeout\n");
		return 1;
	}
	//receiving new port
	int len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
	if(len < 0) { perror("recvfrom error"); exit(1); }
	if(strcmp(buffer, "server in use") == 0){ printf("Server already used\n"); return 1;} //use for single client version
	
	port = atoi(buffer);
	
	//using the new port
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) { perror("file descriptor error"); exit(1); }
	dest.sin_port = htons(port);
	FD_ZERO(&read_set); /* Clear the read_set */
	FD_SET(fd, &read_set); /* Wait until fd is ready for reading */
	
	//sending filename
	strcpy(buffer, argv[1]);
	err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
	if(err < 0) { perror("sendto error"); exit(1); }
	
	//setting song parameters
	//timeout check
	timeout.tv_sec = TTL;
	nb = select(fd+1, &read_set, NULL, NULL, &timeout);
	if(nb == 0) { //timeout
		printf("timeout\n");
		return 1;
	}
	len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
	if(len < 0) { perror("recvfrom error"); exit(1); }
	int sample_rate = atoi(buffer); //atoi = string to int
	//timeout check
	timeout.tv_sec = TTL;
	nb = select(fd+1, &read_set, NULL, NULL, &timeout);
	if(nb == 0) { //timeout
		printf("timeout\n");
		return 1;
	}
	len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
	if(len < 0) { perror("recvfrom error"); exit(1); }
	int sample_size = atoi(buffer);
	//timeout check
	timeout.tv_sec = TTL;
	nb = select(fd+1, &read_set, NULL, NULL, &timeout);
	if(nb == 0) { //timeout
		printf("timeout\n");
		return 1;
	}
	len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
	if(len < 0) { perror("recvfrom error"); exit(1); }
	int channels = atoi(buffer);
	
	//writeinit
	int aud_write;
	if(mono)
		aud_write = aud_writeinit(sample_rate, sample_size, 1);
	else
		aud_write = aud_writeinit(sample_rate, sample_size, channels);

	//sending ack
	err = sendto(fd, "ack", SIZE, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
	if(err < 0) { perror("sendto error"); exit(1); }
	
	int i = 0;  //i simulate a datagram loses
	int nbto = 0;  //3 timeout in a row cut the connection
	
	//writing
	//timeout check
	timeout.tv_sec = TTL;
	nb = select(fd+1, &read_set, NULL, NULL, &timeout);
	if(nb == 0) { //timeout
		printf("timeout\n");
		return 1;
	}
	len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
	if(len < 0) { perror("recvfrom error"); exit(1); }
	
	bool start = true;
	
	while(strcmp(buffer, "done" ) != 0 && nbto < 3){
	
	
		//applying filters
		if(volume){
			volumeEdit(buffer, sample_size, volumeValue);
			//write(aud_write, buffer, SIZE);	
		}
		
		if(echo && !start){
			makeEcho(buffer, previousBuffer, sample_size);
			//write(aud_write, buffer, SIZE);
		}
		
		if(mono && channels == 2){
			stereoToMono(buffer, sample_size);
			write(aud_write, buffer, SIZE/2);
		}
		
		else
			write(aud_write, buffer, SIZE);
			
		if(start) start = false;
		
		
		
		//sending ack
		/*if(i != 200)*/ err = sendto(fd, "ack", SIZE, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
		if(err < 0) { perror("sendto error"); exit(1); }
		
		//timeout check
		timeout.tv_sec = TTL;
		nb = select(fd+1, &read_set, NULL, NULL, &timeout);
		
		if(nb == 0) { //timeout
			nbto++;
			printf("timeout\n");
			FD_ZERO(&read_set); /* Clear the read_set */
			FD_SET(fd, &read_set); /* Wait until fd is ready for reading */
		}
			
		if(nb > 0){
			nbto = 0;
			memcpy(previousBuffer, buffer, SIZE);
			len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &dest, &flen);
			if(len < 0) { perror("recvfrom error"); exit(1); }
		}
			
		i++;
		
		
		
	}
	

	close(fd);
	
	
	


  return 0;
}  
