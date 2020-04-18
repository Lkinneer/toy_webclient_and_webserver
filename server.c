#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>


void *handle_connection(void *void_ptr){
	//read the client request into the buffer response
	char request[16384];
	int recv_output;
	memset(request,0,sizeof(request));
	recv_output = recv(*(int *)void_ptr,request,16384,0);
	printf("%s\n",request);
	//determine if this is a get request
	if(request[0] == 'G' && request[1] == 'E' && request[2] == 'T'){
		char date[40];
		char filename[strlen(request)];
		int iterator = 0;
		for( int i = 4; request[i]!=' '; i++){
			filename[iterator]=request[i];
			iterator++;
		}
		filename[iterator]='\0';

		printf("The filename requested is %s \n",filename);

		char* modify_string;
		if(modify_string = strstr(request,"If-Modified-Since:")){
			//pointss result at the character after the last character in the date string
			char * result = strstr(&modify_string[19],"\n");
			strncpy(date , &modify_string[19], 24);
			date[24]='\0';
			printf("date is:%s\n",date);
			if(access(&filename[1], F_OK)!= -1){
				struct stat attrib;
				stat(&filename[1],&attrib);
				printf("the last time the file was modified was %ld\n",attrib.st_mtime);
				struct tm t;
				time_t epoch_time;
				strptime(date,"%a %b %d %H:%M:%S %Y",&t);
				epoch_time = mktime(&t);

				printf("the time in the request is %ld\n",(long)epoch_time);

				if(attrib.st_mtime > epoch_time){

					char buffer[1000000];
				FILE *fp = fopen(&filename[1],"r");
				if(fp != NULL){
					size_t bytesWritten = fread(buffer, sizeof(char), 1000000, fp);
    				if ( ferror( fp ) != 0 ) {
        				perror("Error reading file");
    				} else {
        				buffer[bytesWritten++] = '\0'; // Null terminate the string
    				}
    				fclose(fp);
    				char buffer2[1000500];
    				char *some_response = "HTTP/1.1 200 OK\n\n";
    				strcpy(buffer2,some_response);
    				strcat(buffer2,buffer);
    				send(*(int *)void_ptr, buffer2, strlen(buffer2),0);
				}else{
					perror("error opening file");
					exit(1);
				}

				}else{
					send(*(int *)void_ptr, "HTTP/1.1 304 Not Modified\n\n", 27, 0);
				}


			}else{
				send(*(int *)void_ptr, "HTTP/1.1 404 Not Found\n\n", 23, 0);
			}


			
		}else{
			//there is no Modified-since header
			if(access(&filename[1], F_OK)!= -1){
				//send(*(int *)void_ptr, "HTTP/1.1 200 OK\n\n", 23, 0);
				char buffer[1000000];
				FILE *fp = fopen(&filename[1],"r");
				if(fp != NULL){
					size_t bytesWritten = fread(buffer, sizeof(char), 1000000, fp);
    				if ( ferror( fp ) != 0 ) {
        				perror("Error reading file");
    				} else {
        				buffer[bytesWritten++] = '\0'; // Null terminate the string
    				}
    				fclose(fp);
    				char buffer2[1000500];
    				char *some_response = "HTTP/1.1 200 OK\n\n";
    				strcpy(buffer2,some_response);
    				strcat(buffer2,buffer);
    				send(*(int *)void_ptr, buffer2, strlen(buffer2),0);
				}else{
					perror("error opening file");
					exit(1);
				}

			}else{
				send(*(int *)void_ptr, "HTTP/1.1 404 Not Found\n\n", 23, 0);
			}


		}

		

		send(*(int *)void_ptr, "Hello There\n\n", 13, 0);
	}else if(request[0] == 'H' && request[1] == 'E' && request[2] == 'A' && request[3] == 'D'){
		char filename[strlen(request)];
		int j = 0;
		for (int i = 6; request[i]!=' '; i++){
			filename[j] = request[i];
			j++;
		}
		filename[j]='\0';

		printf("the filename is %s\n",filename);

		if(access(filename, F_OK)!= -1){
			send(*(int *)void_ptr, "HTTP/1.1 200 OK\n\n", 23, 0);
		}else{
			send(*(int *)void_ptr, "HTTP/1.1 404 Not Found\n\n", 23, 0);
		}
		
	}else{
		send(*(int *)void_ptr, "Bad request this server only responds to GET and HEAD requests\n\n", 64, 0);
	}


	//respond to the client request

	//close the session file descriptor
	close(*(int *)void_ptr);

}

int main(int argc , char * argv[]){

	pthread_t thread[100];
	int threadcounter = 0;

	const char* hostname = 0;
	const char* portname = "80";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
	struct addrinfo* res=0;
	int error=getaddrinfo(hostname,portname,&hints,&res);
	if(error!=0){
		perror("get addrinfo has failed");
		exit(1);
	}

	//set the SO_REUSEADDR socket option
	int fd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(fd == -1){
		perror("There was an error setting up the socket");
		exit(1);
	}

	//bind the address to the socket
	if(bind(fd,res->ai_addr,res->ai_addrlen)==-1){
		perror("error binding the address to the socket");
		exit(1);
	}

	freeaddrinfo(res);
	//now listen for connections
	if(listen(fd,SOMAXCONN)){
		perror("failed to listen");
		exit(1);
	}
	printf("have succeded in listening");
	while(1){
		//will hang here until a client attempts to connect
		int session = accept(fd,0,0);
		//error handeling for accept
		if (session == -1){
			if (errno==EINTR){
				continue;	
			}
			perror("failed to accept connection");
			exit(1);
		}
		printf("session created\n");

		if(pthread_create(&thread[threadcounter],NULL,handle_connection,&session)){
			perror("error creating thead");
			exit(1);
		}
		printf("a thread has been created\n");
		threadcounter++;
		threadcounter = threadcounter%100;
		if (threadcounter == 5){
			printf("exiting");
			exit(1);	
		}
	}
}
