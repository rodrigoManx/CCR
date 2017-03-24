  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <pthread.h>

  int SocketFD;
  pthread_t recvThread;
  int chatStatus = 0;

  void chat();
  void *recvMessage(void *ptr);

  int main(int argc, char *argv[])
  {
    struct sockaddr_in stSockAddr;
    int Res;
    SocketFD = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
    int n;
    char buffer[256];
    char message[256];
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    Res = inet_pton(AF_INET, "192.168.201.67", &stSockAddr.sin_addr);

    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    chat();
    return 0;
  }
  void *recvMessage(void *ptr)
  {
    int n;
    char buffer[250];

    while(chatStatus == 1)
    {
      bzero(buffer, 250);
      n = recv(SocketFD, buffer, 250, 0);
      if (n < 0) perror("ERROR reading from socket");
      printf("Server: %s\n", buffer);
    }
  }

  void chat()
  {
    pthread_create(&recvThread, NULL, recvMessage, NULL);
    int n;
    char message[250];
    chatStatus = 1;

    do
    {
      bzero(message, 250);
      fgets(message, 250, stdin);
      int messageSize = strlen(message);
      if (messageSize > 1 && message[messageSize - 1] == '\n') 
        message[messageSize - 1] = '\0';
      n = send(SocketFD, message, strlen(message), 0);
    }
    while(strcmp(message, "END") != 0);
    chatStatus = 0;
    pthread_join(recvThread, NULL);
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
  }
