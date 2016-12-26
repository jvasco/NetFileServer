#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02


void* clientSocket();
void* serverSocket();

int listenfd = 0;
struct sockaddr_in serverAddr, clientAddr;
int portNumber = 9003;
int connfd = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int fileMode = -1;

typedef struct file{
	char* fileName;
	int transfd;
	int exclfd;
	int fds[10];
	int fdind;
} file;

file files[10];

int fileNum = 0;

int main(int argc, char* argv[])
{
	int f;
	for(f=0;f<10;f++){
		files[f].fileName = 0;
		files[f].transfd = 0;
		files[f].exclfd = 0;
		memset(files[f].fds, 0, sizeof(files[f].fds));
		files[f].fdind = 0;
	}
	
	int socketNum = 0;
	
	pthread_t tid[10];
	
	socklen_t socksize = sizeof(struct sockaddr_in);
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0x0000);
	
	if(listenfd<0)
	{
		printf("error\n");
		//return -1;
	}
	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(portNumber);
	if(bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<0)
	{
		printf("error binding\n");
		
	}
	
	listen(listenfd, 1);
	
	int acceptSocket = accept(listenfd, (struct sockaddr *)&clientAddr, &socksize);
	
	while(acceptSocket){
		pthread_create(&tid[socketNum], NULL, clientSocket, &acceptSocket);
		pthread_detach(tid[socketNum]);
		
		socketNum++;
		pthread_mutex_lock(&m);
		acceptSocket = accept(listenfd, (struct sockaddr *)&clientAddr, &socksize);
		if(socketNum==10)break;
		
	}
	
	
}

void* clientSocket(void *arg){
	
	int acceptSocket = *(int*)arg;
	pthread_mutex_unlock(&m);
	
	char buffer[500];
	
	bzero(buffer, 500);
	
	//socklen_t socksize = sizeof(struct sockaddr_in);
	
	//int acceptSocket = accept(listenfd, (struct sockaddr *)&clientAddr, &socksize);
	
		int k = recv(acceptSocket, buffer, 500, 0);
		
		if(k<0)
		{
			printf("Error reading\n");
		}
		
		char* buff = "read ";
		
		
		if(buffer[0]=='0')
		{
			fileMode = 0;
		}
		else if(buffer[0]=='1')
		{
			fileMode = 1;
		}
		else if(buffer[0]=='2')
		{
			fileMode = 2;
		}
		else
		{
			printf("error: nonvalid filemode\n");
			return ;
		}
		memmove(buffer, buffer+1, strlen(buffer+1) + 1);
		
		if(strncmp(buffer, "open ", 5)==0)
		{
			char* temp = strstr(buffer, " ");
			char* temp2;
			char temp3[2];
			temp++;
			temp =  strstr(buffer, " ");
			strncpy(temp3, temp+1, 1);
			temp3[1]='\0';
			int permission = atoi(temp3);
			temp2 = strstr(temp+1, " ");
			char* filePath = temp2+1;
			
			int mode = 0;
			int i;
			file currentFile;
			int ifPath;
			if(fileNum > 0){
				ifPath = 0;
				for(i=0;i<10;i++){
					if(strcmp(files[i].fileName, filePath)==0){
						ifPath = 1;
						currentFile = files[i];
						break;
					}
				}
				
				if(ifPath==0){
					files[fileNum].fileName = filePath;
					currentFile = files[fileNum];
					fileNum++;
				}
				
			}
			
			if(currentFile.exclfd > 0){
				if(permission == 1 || permission == 2){
					
					char stringFD[10];
					sprintf(stringFD, "%d", -1);
					send(acceptSocket, stringFD, 10, 0);
					errno=0;
					close(acceptSocket);
					pthread_exit(NULL);
				}
			}
			if(currentFile.transfd > 0){
				char stringFD[10];
				sprintf(stringFD, "%d", -1);
				send(acceptSocket, stringFD, 10, 0);
				errno = 0;
				close(acceptSocket);
				pthread_exit(NULL);
			}
			
			
			//int fd = open(filePath, permission);
			//int fd = open("/.autofs/ilab/ilab_users/njl78/anything/asst3/text.txt", O_WRONLY);
			//int fd2 = open("/.autofs/ilab/ilab_users/njl78/anything/asst3/text2.txt", O_RDWR);
			//printf("fd: %d fd2: %d\n", fd, fd2);
			
			//strncpy(buffer2, temp2, strlen(temp2));
			//printf("File path is %s\n", buffer2);
			errno = 0;
			int fd = open(filePath, permission);
			if(fd<0)
			{
				send(acceptSocket, strerror(errno), strlen(strerror(errno)), 0);
				close(acceptSocket);
				return 0;
				
			}
			if(fileMode==1){
				currentFile.exclfd = fd;
			}else if(fileMode==2){
				currentFile.transfd = fd;
			}
			
			currentFile.fds[currentFile.fdind] = fd;
			currentFile.fdind++;
			
			char stringFD[10];
			sprintf(stringFD, "%d", fd);
			send(acceptSocket, stringFD, 10, 0);
			errno = 0;
			close(acceptSocket);
		}else if (strncmp("read ", buffer, 5)==0)//no idea why i have to add +1 in there
		{
			
			char* temp = strstr(buffer, " ");
			char* nbytes = strstr(temp+1, " ");
			char fd[strlen(buffer)];
			strncpy(fd, temp+1, strlen(temp)-strlen(nbytes)-1);
			
			int nbyte = atoi(strstr(temp+1, " ")+1);
			
			int filedes = atoi(fd);
			
			//read from file
			int bytesRead = 0;
			int result;
			char buf[100];
			
			
			//errno = ETIMEDOUT;
			//errno = ECONNRESET;
			
			//test
			//int fd3 = open("/.autofs/ilab/ilab_users/njl78/anything/asst3/text.txt", O_RDONLY);
			//test
			
			errno = 0;
			int isError = 0;
			char errMessage[30];
			bzero(errMessage, strlen(errMessage));
			char message[500];
			bzero(message, strlen(message));
			if(bytesRead = read(filedes, buf, nbyte)<0)
			{
				isError = 1;
				send(acceptSocket, strerror(errno), strlen(strerror(errno)), 0);
				errno = 0;
				close(acceptSocket);
				return 0;
			}
			else{
				strcat(message, buf);
			
			}
			send(acceptSocket, message, strlen(message), 0);
			errno = 0;
			close(acceptSocket);
		}
		else if(strncmp("writ ", buffer, 5)==0)
		{
		
			char* temp = strstr(buffer, " ");
			char* temp2 = strstr(temp+1, " ");
			char* temp3 = strstr(temp2+1, " ");
			
			char filedes[strlen(buffer)];
			filedes[strlen(temp)-strlen(temp2)]='\0';
			strncpy(filedes, temp+1, strlen(temp)-strlen(temp2));
			
			char size[strlen(buffer)];
			size[strlen(temp2)-strlen(temp3)]='\0';
			strncpy(size, temp2+1, strlen(temp2)-strlen(temp3));
			
			char buffString[strlen(temp3)];
			strncpy(buffString, temp3+1, strlen(temp3));
			
			int filedestination = atoi(filedes);
			const char *buf = buffString;
			size_t nbyte = atoi(size);
		
			errno = 0;
			int bytesWritten = write(filedestination, buf, nbyte);
			if(bytesWritten<1)
			{
				send(acceptSocket, strerror(errno), strlen(strerror(errno)), 0);
				close(acceptSocket);
			}
			
			errno = 0;
			close(acceptSocket);
			
		}
		else if(strncmp("clos ", buffer, 5)==0)
		{
			char* temp = strstr(buffer, " ");
			char filedes[strlen(buffer)];
			filedes[strlen(temp)-1]='\0';
			strncpy(filedes, temp + 1, strlen(temp)-1);
			
			int filedestination = atoi(filedes);
			
			errno = 0;
			int closeFile = close(filedestination);
			
			int x;
			int y;
			for(x=0; x<10; x++){
				for(y=0; y<files[x].fdind + 1; y++){
					if(files[x].fds[y]==filedestination){
						if(files[x].exclfd == filedestination){
							files[x].exclfd = 0;
						}
						if(files[x].transfd == filedestination){
							files[x].transfd = 0;
						}
					}
				}
			}
			
			char stringCF[2];
			sprintf(stringCF, "%d", closeFile);
			send(acceptSocket, stringCF, 2, 0);
			errno = 0;
			close(acceptSocket);
		}
		bzero(buffer, strlen(buffer));
		
}