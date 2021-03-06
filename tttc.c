  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <pthread.h>
  #include <signal.h>

  void ticTacToe(int SocketFD);
  void recvMessage(int ConnectFD, char *buffer)
  {
    int n;

    bzero(buffer, 250);
    n = recv(ConnectFD, buffer, 250, 0);
    if (n < 0) perror("ERROR reading from socket");
    if (buffer[0] != '\0')
    {
      printf("Client: %s\n", buffer);
    }
    else
      printf("Client disconnected\n");
  }
  

  void sendMessage(int ConnectFD, char *message)
  {
    int n;
    n = send(ConnectFD, message, strlen(message), 0);
    if (n < 0)
      perror("ERROR sending message");
  }


  int main(int argc, char *argv[])
  {
    int SocketFD;
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
    Res = inet_pton(AF_INET, "192.168.0.20", &stSockAddr.sin_addr);

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

    ticTacToe(SocketFD);
    return 0;
  }

  void ticTacToe(int SocketFD)
  {
    pid_t childPID = fork();

    if (childPID < 0)
    {
      perror("ERROR creating message sending process"),
      exit(1);
    }
    else if (childPID > 0)
    {
      int n;
      char buffer[250];

      do
      {
        bzero(buffer, 250);
        n = recv(SocketFD, buffer, 250, 0);
        if (n < 0) perror("ERROR reading from socket");
        if (buffer[0] != '\0')
        {
          printf("%s\n", buffer);
        }
        else
          printf("Server ofline\n");
      }
      while(n != 0);
      kill(childPID, SIGKILL);
      close(SocketFD);
    }

    else
    {
      int n;
      char message[250];

      for(;;)
      {
        bzero(message, 250);
        fgets(message, 250, stdin);
        int messageSize = strlen(message);
        if (messageSize > 1 && message[messageSize - 1] == '\n') 
          message[messageSize - 1] = '\0';
        n = send(SocketFD, message, strlen(message), 0);
      }
    }
  }