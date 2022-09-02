#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "include/audio.h"

#define SIZE 1024
#define TTL 2

int main() {

	
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) { perror("file descriptor error"); exit(1); }
	
	//addr setup
	int port = 1234;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	struct sockaddr_in from;
	socklen_t flen = sizeof(struct sockaddr_in);
	
	//timeout setup
	fd_set read_set;
	struct timeval timeout;
	FD_ZERO(&read_set); /* Clear the read_set */
	FD_SET(fd, &read_set); /* Wait until fd is ready for reading */
	int nb;
	
	char buffer[SIZE];
	pid_t pid;
	
	
	int err = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if(err < 0 ) { perror("bind error"); exit(1); }

	
	
	while(1){
		//new client
		int len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &from, &flen);
		if(len < 0) { perror("recvfrom error"); exit(1); }
		
		port++;
		pid = fork();
		if(pid<0) { perror("fork error"); exit(1); }
		if(pid>0){
			
			//remove the following comments to limit to a single client
			
			/*
			int stat;
			while(pid=waitpid(-1, &stat, WNOHANG) == 0) { //while child alive
				len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &from, &flen);
				sendto(fd, "server in use", SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			}
			*/
		}
		
		else{
			//using a new port
			sprintf(buffer, "%d", port);
			err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			if(err < 0) { perror("sendto error"); exit(1); }
			
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			if(fd < 0) { perror("file descriptor error"); exit(1); }
			
			addr.sin_port = htons(port);
			err = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
			if(err < 0 ) { perror("bind error"); exit(1); }
			
			FD_ZERO(&read_set); /* Clear the read_set */
			FD_SET(fd, &read_set); /* Wait until fd is ready for reading */			
			
			//timeout check
			timeout.tv_sec = TTL;
			nb = select(fd+1, &read_set, NULL, NULL, &timeout);
			
			if(nb == 0) { //timeout
				printf("timeout\n"); 
				return 1;
			}
			
			//receiving filename
			len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &from, &flen);
			if(len < 0) { perror("recvfrom error"); exit(1); }
			char filename[SIZE] = "song/";
			strcat(filename, buffer);
			strcat(filename, ".wav");
			
			
			//readinit
			int sample_rate;
		  	int sample_size;
		   	int channels;
		   	int aud_read = aud_readinit(filename, &sample_rate, &sample_size, &channels);
		   
		   	//sending parameters
			sprintf(buffer, "%d", sample_rate); //int to string
			err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			if(err < 0) { perror("sendto error"); exit(1); }
			sprintf(buffer, "%d", sample_size);
			err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			if(err < 0) { perror("sendto error"); exit(1); }
			sprintf(buffer, "%d", channels);
			err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			if(err < 0) { perror("sendto error"); exit(1); }
			
			//timeout check
			timeout.tv_sec = TTL;
			nb = select(fd+1, &read_set, NULL, NULL, &timeout);
			
			if(nb == 0) { //timeout
				printf("timeout\n"); 
				return 1;
			}
			
			//waiting client's ack
			len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &from, &flen);
			if(len < 0) { perror("recvfrom error"); exit(1); }
			
			int i = 0;  //i simulate a datagram loses
			int nbto = 0;  //3 timeout in a row cut the connection
			
			//reading
			while(read(aud_read, buffer, SIZE) > 0 && nbto < 3) {
				/*if( i!= 400)*/ err = sendto(fd, buffer, SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
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
					//waiting client's ack
					len = recvfrom(fd, buffer, SIZE, 0, (struct sockaddr*) &from, &flen);
					if(len < 0) { perror("recvfrom error"); exit(1); }
				}
				
				i++;
			}
			
			
			err = sendto(fd, "done", SIZE, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
			if(err < 0) { perror("sendto error"); exit(1); }
			close(fd);
			

				  
		  

		  return 0;
		}  
	}
return 0;
}
