#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>
#include <arpa/inet.h>
#define MAX 80 


void get_time(char* time_str){
	struct timeval now;
	gettimeofday(&now,NULL);
	strcpy(time_str,ctime(&now.tv_sec));
}
