#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

int ConnectFD;
pthread_t recvThread;
int chatStatus = 0;

void *recvMessage(void *ptr);
void chatService();

  int main(void)
  {
    struct sockaddr_in stSockAddr;
    struct sockaddr_in cli_addr;
    int client;
    int SocketFD;
    char buffer[256];
    char message[256];
    int Res;
    int n;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(SocketFD,SOL_SOCKET,SO_REUSEADDR,"1",sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    Res = inet_pton(AF_INET, "192.168.201.67", &stSockAddr.sin_addr);
    //stSockAddr.sin_addr.s_addr = 192.168.0.20;

    if (bind(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr))  == -1) {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(SocketFD, 5) == -1) {
            perror("Listen");
            exit(1);
        }

    for(;;)
    {

    	client = sizeof(struct sockaddr_in);

    	ConnectFD = accept(SocketFD, (struct sockaddr *)&cli_addr,&client);

        chatService();
    }

    close(SocketFD);
    return 0;
  }

void *recvMessage(void *ptr)
  {
    int n;
    char message[250];

    do
    {
      bzero(message, 250);
      fgets(message, 250, stdin);
      int messageSize = strlen(message);
      if (messageSize > 1 && message[messageSize - 1] == '\n') 
        message[messageSize - 1] = '\0';
      n = send(ConnectFD, message, strlen(message), 0);
    }
    while(chatStatus == 1);
  }

  void chatService()
  {
    pthread_create(&recvThread, NULL, recvMessage, NULL);


	chatStatus = 1;
    int n;
    char buffer[250];

    do
    {
      bzero(buffer, 250);
      n = recv(ConnectFD, buffer, 250, 0);
      if (n < 0) perror("ERROR reading from socket");
      printf("Client: %s\n", buffer);
    }
    while(strcmp(buffer, "END") != 0 && n != 0);

    chatStatus = 0;

    //pthread_join(recvThread, NULL);
    close(ConnectFD);
  }
