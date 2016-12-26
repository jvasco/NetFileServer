#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "libnetfiles.h"
#define INVALID_FILE_MODE	42
#define MAXRCVLEN 500
#define PORTNUM 9003

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02

#define unrestricted	00
#define exclusive		01
#define transaction		02

ssize_t netread();
ssize_t netwrite();

char* hostName;

extern int h_errno;

int fileMode = 0;
int initRun = 0;

char servipaddr[20];

int main(int argc, char* argv[])
{
	//char* stuff = "hello!";
	//netserverinit("null.cs.rutgers.edu", 1);
	//int fd = netopen("/.autofs/ilab/ilab_users/jv435/Systems/text.txt", O_RDWR);
	//int fd2 = netopen("/.autofs/ilab/ilab_users/njl78/anything/asst3/text.txt", O_RDWR);
	//netread(fd, stuff, 8);
	//printf("fd: %d and fd2: %d\n", fd, fd2);
	//netread(12389, stuff, 567892);
	//netwrite(fd, stuff, 5);
	//netclose(1234567890);
	return 0;
	
}
int netopen(const char *pathname, int flags)
{
	if(initRun==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	bzero(buffer, sizeof(buffer));
	
	char buffer2[10];
	(int)flags;
	sprintf(buffer2, "%d", flags);
	int len, mysocket;
	len=12;
	struct sockaddr_in dest;
	char fileModeChar[2];
	sprintf(fileModeChar, "%d", fileMode);
	strcat(buffer, fileModeChar);
	strcat(buffer, "open ");
	strcat(buffer, buffer2); //flags
	strcat(buffer, " ");
	strcat(buffer, pathname);
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); /* zero the struct */
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(servipaddr); /* set destination IP number*/
	dest.sin_port = htons(PORTNUM); /* set destination port number*/
	
	errno = 0;
	connect(mysocket, (struct sockaddr *) &dest, sizeof(dest));
	//write(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	int j = send(mysocket, buffer, strlen(buffer), 0);
	char returnVal[40];
	bzero(returnVal, strlen(returnVal));
	recv(mysocket, returnVal, 40, 0);
	returnVal[strlen(returnVal)-2]='\0';
	
	if(strcmp("No such file or directory", returnVal)==0)
	{
		printf("No such file or directory\n");
		close(mysocket);
		return -1;
	}
	else{

	int returnValInt = atoi(returnVal);
	errno = 0;
	close(mysocket);
	return returnValInt;
	}
	
}

ssize_t netread(int fildes, void *buf, size_t nbyte)
{
	if(initRun==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	bzero(buffer, sizeof(buffer));
	char fileModeChar[2];
	sprintf(fileModeChar, "%d", fileMode);
	strcat(buffer, fileModeChar);
	strcat(buffer, "read ");
	char filedes[10];
	sprintf(filedes, "%d", fildes);
	strcat(buffer, filedes);
	char nbytes[MAXRCVLEN+1];
	sprintf(nbytes, "%d", nbyte);
	strcat(buffer, " ");
	strcat(buffer, nbytes);
	int len, mysocket;
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); /* zero the struct */
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(servipaddr); /* set destination IP number*/
	dest.sin_port = htons(PORTNUM); /* set destination port number*/
	
	errno = 0;
	connect(mysocket, (struct sockaddr *) &dest, sizeof(dest));
	int j = send(mysocket, buffer, strlen(buffer), 0);

	
	char returnMessage[500];
	bzero(returnMessage, strlen(returnMessage));
	recv(mysocket, returnMessage, 500, 0);

	
	close(mysocket);
	if(strcmp(returnMessage, "Bad file descriptor")==0)
	{
		return -1;
	}
	return strlen(returnMessage);
}

ssize_t netwrite(int fildes, const void *buf, size_t nbyte)
{
	if(initRun==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	char* charbuf = (char*)buf;
	
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	bzero(buffer, sizeof(buffer));
	char fileModeChar[2];
	sprintf(fileModeChar, "%d", fileMode);
	strcat(buffer, fileModeChar);
	int len, mysocket;
	struct sockaddr_in dest;
	
	strcat(buffer, "writ ");
	
	char temp[MAXRCVLEN];
	sprintf(temp, "%d", fildes);
	strcat(buffer, temp);
	
	strcat(buffer, " ");
	
	char temp2[MAXRCVLEN];
	sprintf(temp2, "%d", nbyte);
	strcat(buffer, temp2);
	
	strcat(buffer, " ");
	
	strcat(buffer, charbuf);
	
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); /* zero the struct */
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(servipaddr); /* set destination IP number*/
	dest.sin_port = htons(PORTNUM); /* set destination port number*/
	errno = 0;
	int c = connect(mysocket, (struct sockaddr *) &dest, sizeof(dest));
	if(c<0)
	{
		printf("Error, no connection\n");
	}
	//write(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	errno = 0;
	int j = send(mysocket, buffer, strlen(buffer), 0);	
	errno = 0;
	//len = recv(mysocket, buffer, MAXRCVLEN, 0);
	char returnMessage[500];
	bzero(returnMessage, strlen(returnMessage));
	recv(mysocket, returnMessage, 500, 0);
	returnMessage[strlen(returnMessage)-1] = '\0';
	if(strcmp(returnMessage, "Bad file descriptor")==0)
	{
		printf("%s\n", returnMessage);
		close(mysocket);
		return -1;
	}
	int numRec = atoi(returnMessage);
	
	errno = 0;
	close(mysocket);
	return numRec;
}

int netclose(int fd)
{
	if(initRun==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
	bzero(buffer, sizeof(buffer));
	char fileModeChar[2];
	sprintf(fileModeChar, "%d", fileMode);
	strcat(buffer, fileModeChar);
	strcat(buffer, "clos ");
	char filed[50];
	sprintf(filed, "%d", fd);
	strcat(buffer, filed);
	int len, mysocket;
	struct sockaddr_in dest;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); /* zero the struct */
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(servipaddr); /* set destination IP number*/
	dest.sin_port = htons(PORTNUM); /* set destination port number*/
	errno = 0;
	connect(mysocket, (struct sockaddr *) &dest, sizeof(dest));
	//write(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	errno = 0;
	int j = write(mysocket, buffer, strlen(buffer));
	if(j<1)
		printf("Error writing\n");
	
	char returnVal[15];
	recv(mysocket, returnVal, 15, 0);
	int returnValInt = atoi(returnVal);
	errno = 0;
	close(mysocket);
	return returnValInt;
}

int netserverinit(char * hostname, int filemode){
	
	initRun = 1;
	if(filemode < 0 || filemode > 2){
		h_errno = INVALID_FILE_MODE;
		printf("Invalid file mode");
		return -1;
	}
	
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		//exit(1);
		return -1;
	}
	
	fileMode = filemode;
	
	char host[256], service[256];
	
	for(p = servinfo; p != NULL; p = p->ai_next) {
		getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST);
		//puts(host);
	}
	
	//servipaddr = host;
	strcpy(servipaddr, host);
	
	return 0;
}