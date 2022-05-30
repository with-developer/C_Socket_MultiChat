#include "lib.h"
#define PORT 8080 
#define MAX_CLIENTS 10

sem_t mutex; 
char ShareM[MAX]; // Share memory
int num_client=0; // total number of clients
int num_sent=0; // number of clients that the message has been sent
int new_message=0; // set to 1 when there is a new message, reset when all threads have sent
int sent_clientfd; //the client that sent the message will not receive the message it sent 
int exit_clientfd=-1;

void* fsend(void* sockfd) 
{
	char buff[MAX]; 
	int sent=0; // set to one when this thread has already sent the new message
	for (;;) { 
		if(exit_clientfd == *(int*)sockfd){
			exit_clientfd=-1;
			break;
		}	
		if(new_message==0 && sent==1)	sent=0;	// reset sent
		else if(new_message==1 && sent==0 && sent_clientfd!=*(int*)sockfd){

			send(*(int*)sockfd, ShareM, sizeof(ShareM), 0); 
			bzero(buff, MAX);
			num_sent++;
			sent=1;	
			if(num_sent == num_client-1){ // last thread that hasn't sent runs
				bzero(ShareM, MAX); // reset Share memory
				num_sent=0;
				new_message=0;
			}
		}
		else if(sent_clientfd==*(int*)sockfd && num_client==1 && new_message==1){ 
		// reset when there is only one client
			bzero(ShareM, MAX);
			num_sent=0;
			new_message=0;		
		}
		else{}
	}
}

void* frecv(void* sockfd) 
{
	char buff[MAX];
	char name[MAX];
	char* time_str;
	time_str=(char*)malloc(50);
	bzero(name,MAX);
    	int first=0;

	for(;;){		
		bzero(buff, MAX);
		recv(*(int*)sockfd, buff, sizeof(buff), 0); 
		if(buff[0]=='\0'){
			exit_clientfd= *(int*)sockfd; // record exited clientfd
			get_time(time_str);
			printf("\n%s-------%s exit-------\n",time_str,name);			
			break;
		}
		if(!first){//fisrt message is the name of client
			strcpy(name,buff);	
			get_time(time_str);
			printf("\n%s-------Server accept the client %s-------\n",time_str,name); 
			first=1;
			strcpy(buff, "-------");
			strcat(buff, name);
			strcat(buff, " enters the chatroom-------\n");			
			//continue;
		}
		else{
			get_time(time_str);
			printf("\n%s----%s",time_str, buff);
		}

		sem_wait(&mutex); // semaphore wait
		while(new_message) // wait until all the threads sent the message (new_message=0)
			;
		sent_clientfd= *(int*)sockfd; // remember the client that sent the message
		strcpy(ShareM, buff); // Put the received message into Share memory
		new_message=1;  
		sem_post(&mutex); // semaphore signal
	}

	--num_client;	
//	printf("--num_client\n");
}

// Driver function 
int main() 
{ 
	int sockfd,  len; 
	int clientfds[MAX_CLIENTS];
	struct sockaddr_in servaddr, cliaddr; 
	
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("-------socket creation failed-------\n"); 
		exit(0); 
	} 
	else
		printf("-------Socket successfully created-------\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("-------Socket bind failed-------\n"); 
		exit(0); 
	} 
	else
		printf("-------Socket successfully binded-------\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, MAX_CLIENTS)) != 0) { 
		printf("-------Listen failed-------\n"); 
		exit(0); 
	} 
	else
		printf("-------Server listening-------\n"); 
	len = sizeof(cliaddr); 
	
	pthread_t thr_send, thr_recv;
	sem_init(&mutex, 0, 1); 

	while(1){
		// Accept the data packet from client and verification 
		int* clientfd = clientfds + num_client;
		
		*clientfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len); 
		if (*clientfd < 0) { 
			printf("-------Server acccept failed-------\n"); 
			exit(0); 
		} 
		else{
			
			if(num_client >= MAX_CLIENTS){
				printf("-------Reach the max number of clients!-------\n");
			}
			else{
				++num_client;
				
				pthread_create(&thr_send, NULL, fsend, (void*) clientfd);
				pthread_create(&thr_recv, NULL, frecv, (void*) clientfd);
			}
			
		}
	}

	bzero(ShareM, MAX);

	sem_destroy(&mutex);
	pthread_join(thr_send, NULL);
	pthread_join(thr_recv, NULL);
	
	// After chatting close the socket 
	close(sockfd); 
} 

