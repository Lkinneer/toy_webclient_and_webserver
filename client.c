#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>


typedef struct{
	int day, hour, minute;
}date;

typedef struct{
	char * hostname, resource_identifier;
	int port_number;
}url;

void parse_date(char * string , date * moddified_since){
	char temp_string[strlen(string)];
	strcpy(temp_string, string);
	moddified_since->day = atoi(strtok(temp_string, ":"));
	moddified_since->hour = atoi(strtok(NULL, ":"));
	moddified_since->minute = atoi(strtok(NULL, ":"));	
}
//todo
void create_url(char * string , url * new_url){

}
void destroy_url(url * old_url){
	//free(old_url->hostname);
	//free(old_url->resource_identifier);
}

int main(int argc, char * argv[]){
	//set when -h is detected
	int header_only = 0;
	//set when -d is detected
	int date_set = 0;
	date moddified_since;
	char * url_string;
	//char * file_path;


	if(argc < 2){
		perror("please provide the url you wish to browse to as an argument.\n");
		exit(0);
	}
	for (int i = 1; i<argc; i++){
		if (strcmp(argv[i],"-d" ) == 0){
			date_set = 1;
			i++;
			parse_date(argv[i], &moddified_since);
		}else if (strcmp(argv[i],"-h") == 0){
			header_only = 1;
		}else{
			url_string = argv[i];
		}
	}

	char file_path[strlen(url_string)];
	strcpy(file_path,&url_string[strcspn(url_string,"/")]);
	if(strcmp("",file_path)==0){
		strcpy(file_path,"/");
	}
	//file_path = &url_string[strcspn(url_string,"/")];

	printf("the file path is %s\n",file_path);
	char real_url[strlen(url_string)];
	strncpy(real_url,url_string,strcspn(url_string,"/"));
	real_url[strcspn(url_string,"/")]='\0';
	printf("the url without the filepath is %s\n",real_url);
	//url_string = real_url;
	printf("header set %d , date set %d\n",header_only , date_set);
	if(date_set){
		printf("day %d hour %d minute %d \n",moddified_since.day,moddified_since.hour,moddified_since.minute);
	}

	//resolve the hostname to an ip address
	//getaddrinfo
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0 , sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	
	rv = getaddrinfo(real_url, "http", &hints, &servinfo); //changed
	if (rv != 0){
		perror("error resolving host name");
		exit(1);
	}
	for(p = servinfo; p!=NULL; p = p->ai_next){
			if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
        	perror("unable to create socket");
	        continue;
    	}
    	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        	perror("unable to connect");
        	close(sockfd);
        	continue;
    	}
    	break; // if we get here, we must have connected successfully
	}
	if (p==NULL){
		perror("failed to create socket or connect");
	}else{
		printf("connection successfull\n");
		size_t size;

		//create text for get request
		char stuff_to_be_sent[500];
		if (header_only){
			strcpy(stuff_to_be_sent,"HEAD ");
		}else{
			strcpy(stuff_to_be_sent,"GET ");
		}
		strcat(stuff_to_be_sent,file_path);
		strcat(stuff_to_be_sent, " HTTP/1.1\r\nHost: ");
		strcat(stuff_to_be_sent,url_string);
		strcat(stuff_to_be_sent,":80\r\n");
		if(date_set){
			int day, hour, min;
			day = moddified_since.day;
			hour = moddified_since.hour;
			min = moddified_since.minute;
			char s_time[30];
			time_t n_time;
			n_time=time(0);
			n_time=n_time-(day*24*3600+hour*3600+min*60);
			strcpy(s_time, ctime(&n_time));

			strcat(stuff_to_be_sent,"If-Modified-Since: ");
			strcat(stuff_to_be_sent, s_time);
			//strcat(stuff_to_be_sent, "\r\n");
			printf("the string s_time is: %s\n",s_time);
		}


		strcat(stuff_to_be_sent, "Accept: */*\r\n\r\n");


		printf("The request sent was:\n%s\n",stuff_to_be_sent);

		//char * stuff_to_be_sent = "GET / HTTP/1.1\r\nHost: www.example.com:80\r\n\r\n";
		//call function to get page
		int sendval = 0;
		sendval = send(sockfd,stuff_to_be_sent,strlen(stuff_to_be_sent),0);
		//printf("the value returned from send is %d", sendval);

		char response[16384];
		int bytes,max_size;
		int recieved = 0;
		memset(response,0,sizeof(response));
		max_size = sizeof(response)-1;
		do {
			bytes = read(sockfd,response+recieved,max_size - recieved);
			if(bytes < 0){
				perror("there was a problem reading from the socket");
			}
			if(bytes == 0){
				break;
			}
			recieved+=bytes;
		}while (recieved < max_size);



		printf("(%s)\n",response);
		close(sockfd);


		//get response

		
	}
	freeaddrinfo(servinfo);

	


}