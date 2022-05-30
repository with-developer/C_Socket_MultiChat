#include "lib.h"

char client_name[MAX];

void* fsend(void* sockfd) 
{
	char buff[MAX]; 
	int n;
    	int first=0;	
	for (;;) { 
		strcpy(buff, client_name); // put client name into buff
		if(!first){
			send(*(int*)sockfd, buff, sizeof(buff), 0); 
			first=1;
		}
		else{
        		strcat(buff, ": ");
			n= strlen(buff); 

			while ((buff[n++] = getchar()) != '\n') 
			; 
			if(!strncmp(buff+strlen(client_name)+2,"exit",4)){
				bzero(buff, sizeof(buff)); 
				exit(0);
			}

			send(*(int*)sockfd, buff, sizeof(buff), 0); 
			bzero(buff, sizeof(buff)); 
		}
	}
}

void* frecv(void* sockfd) 
{
	char buff[MAX];
	char* time_str;
	time_str=(char*)malloc(50);
	for(;;){		
		recv(*(int*)sockfd, buff, sizeof(buff), 0);

		if(buff[0]=='\0'){
			get_time(time_str);			
			printf("\n%s-------Server shut down-------\n", time_str);			
			break;
		}
		get_time(time_str);
		printf("\n%s%s",time_str, buff); 
		bzero(buff, sizeof(buff)); 
	}
}

int main(int argc, char *argv[]) 
{ 
	int sockfd, clientfd; 
	struct sockaddr_in clientaddr; 
	struct hostent *host;

	bzero(client_name, sizeof(client_name));
	strcpy(client_name, argv[2]);
	
	if (argc !=3)  
   	{  
        	printf("-------wrong usage-------");  
        	exit(1);  
    	}  
	
	
	// socket create and varification 
	clientfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (clientfd == -1) { 
		printf("-------socket creation failed-------\n"); 
		exit(0); 
	} 
	else
		printf("-------Socket successfully created-------\n"); 

	bzero(&clientaddr, sizeof(clientaddr)); 

	// assign IP, PORT 
	clientaddr.sin_family = AF_INET; 
	clientaddr.sin_port = htons((uint16_t)8080); 
	inet_pton(AF_INET, argv[1], &clientaddr.sin_addr);

	// connect the client socket to server socket 
	if (connect(clientfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) != 0) { 
		printf("-------connection with the server failed-------\n"); 
		exit(0); 
	} 
	else
		printf("-------connected to the server-------\n"); 


	pthread_t thr_send, thr_recv;

	pthread_create(&thr_send, NULL, fsend, (void*) &clientfd);
	pthread_create(&thr_recv, NULL, frecv, (void*) &clientfd);

	pthread_join(thr_send, NULL);
	pthread_join(thr_recv, NULL);

	// close the socket 
	close(clientfd); 
} 

