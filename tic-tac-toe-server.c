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
#include <signal.h>

int ConnectFD;
int *board;
int boardSize;
pthread_t recvThread;


void *communicationService(void *ptr);
void *ticTacToe(void *ptr);
int symbols = 2;
int turn = 0;


  int main(int argc, char const *argv[])
  {
    
    boardSize = atoi(argv[1]);
    board = (int *)malloc(boardSize * boardSize * sizeof(int)); 
    for (int i = 0; i < boardSize * boardSize; ++i)
    {
      board[i] = 2;
    }   

    struct sockaddr_in stSockAddr;
    
    //int client;
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
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(SocketFD, (struct sockaddr * )&stSockAddr, sizeof(struct sockaddr))  == -1) {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(SocketFD, 5) == -1) {
            perror("Listen");
            exit(1);
        }

    for(;;)
    {

      struct sockaddr_in cli_addr;
      int client = sizeof(struct sockaddr_in);
      int client_sock = accept(SocketFD, (struct sockaddr *)&cli_addr, &client);
      pthread_t clithread;
      pthread_create(&clithread, NULL, ticTacToe, (void *)&client_sock);

    }

    close(SocketFD);
    return 0;
  }


  void recvMessage(int ConnectFD, char *buffer)
  {
    int n;

    bzero(buffer, 250);
    n = recv(ConnectFD, buffer, 250, 0);
    if (n < 0) perror("ERROR reading from socket");
    if (buffer[0] != '\0')
    {
      //printf("size: %d\n", strlen(buffer));
      //printf("Client: |%s|\n", buffer);
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


  void boardToMessage(char *boardBuffer)
  {
      bzero(boardBuffer, 250);
      for(int i = 0; i < boardSize; i++)
      {
        for(int j = 0; j < boardSize; j++)
        {
          if (board[i * boardSize + j] == 2) strcat(boardBuffer, "_\t");
          else if (board[i * boardSize + j] == 0) strcat(boardBuffer, "x\t");
          else strcat(boardBuffer, "o\t");
        }
        strcat(boardBuffer, "\n");
      }
      printf("%s\n", boardBuffer);
  }

  int *protocolForMovements(char *move)
  {
    int *moveData = malloc(2 * sizeof(int));
    char *subcad;
    subcad = strtok (move," ,.-\n");
    moveData[0] = atoi(subcad);
    int i = 1;
    while (subcad != NULL)
    {
      moveData[i] = atoi(subcad);
      subcad = strtok (NULL, " ,.-\n");
    }
    return moveData;
  } 

  void updateBoard(int simbolClient, char *move)
  {
    int * moveData = malloc(2 * sizeof(int));
    moveData = protocolForMovements(move);
    board[moveData[0] * boardSize + moveData[1]] = simbolClient;
  }

  int checkVictory(int *board)
  {
    return 0; 
  }
  void displayBoard()
  {
    for(int i = 0; i < boardSize; i++)
      {
        for(int j = 0; j < boardSize; j++)
        {
          printf("%d\t", board[i * boardSize + j]);
        }
        printf("\n");
      }
  }
  void *ticTacToe(void *ptr)
  {
    int simbolClient;

    int ConnectFD = *(int *)ptr;

    char buffer[250];

    sendMessage(ConnectFD, "Welcome to the tic-tac-toe game\nselect your symbol:\n\"x\"\t\t\"o\"\n");


    if(symbols == 2)
    {     
      do
      {
        recvMessage(ConnectFD, buffer);
      }
      while(strcmp(buffer, "x") != 0 && strcmp(buffer, "o") != 0 && symbols == 2);
      
      if (strcmp(buffer, "x") == 0) simbolClient = symbols = 0;
      else simbolClient = symbols = 1;
    }
    else simbolClient = 1 - symbols;

    if (simbolClient == 0) sendMessage(ConnectFD, "your symbol is: x\n");
    else sendMessage(ConnectFD, "your symbol is: o\n");

    do
    {
      while(turn != simbolClient)
      {

      }
      boardToMessage(buffer);
      sendMessage(ConnectFD, buffer);
      if (turn == simbolClient)
      {
        sendMessage(ConnectFD, "Is your turn");
        recvMessage(ConnectFD, buffer);
        updateBoard(simbolClient, buffer);
        displayBoard();
        turn = 1 - turn;
        printf("%d\n", turn);
      }
      //else {while(turn != simbolClient){}}
      //sleep(4);
    }
    while(1);
  }





