#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <pthread.h>
#include <cstdio>
#include <sstream>


#define NPLAYERS 2
#define DIM 3
#define BUFFSIZE 250

using namespace std;

int playerTurn = 0;
int turn = 0;
string winner = "-";
pthread_t handlers[NPLAYERS];
char board[DIM*DIM];
char marks[NPLAYERS];
//*****************************************************************************
void displayBoard() {
	for (int i = 0; i < DIM; ++i) {
		for (int j = 0; j < DIM; ++j) {
			cout << board[i * DIM + j] << "\t";
		}
		cout << endl;
	}
}

string boardToMessage()
{	
	string aux;
	for(int i = 0; i < DIM; i++) {
		for(int j = 0; j < DIM; j++) {
			aux = aux + board[i * DIM + j] + "\t";
		}
		aux = aux + "\n";
	}
	return aux;	
}

void setMarks() {
	for (int i = 0, j = 0; i < NPLAYERS; ++i) {

		if(i == 26) j = 6;
		marks[i] = i + j + 65;
	}
}

bool boardControl(string playerBoard) {
	string currentBoard = boardToMessage();
	if (playerBoard == currentBoard) return true;
	return false;
}

bool rows(int col, char mark) {
	for (int i = 0; i < DIM; ++i) {
		if (board[i * DIM + col] != mark) return false;
	}
	return true;
}

bool cols(int row, char mark) {
	for (int i = 0; i < DIM; ++i) {
		if (board[row * DIM + i] != mark) return false;
	}
	return true;
}

bool nDiagonal(int row, int col, char mark) {
	if (row != col) return false;
	for (int i = 0; i < DIM; ++i) {
		if (board[i * DIM + i] != mark) return false;
	}
	return true;
}

bool pDiagonal(int row, int col, char mark) {
	if (row + col != DIM - 1) return false;
	for (int i = 0, j = DIM - 1; i < DIM; ++i, --j) {
		if (board[i * DIM + j] != mark) return false;
	}
	return true;
}

bool gameOver(int row, int col, char mark) {
	if(rows(col, mark) || cols(row, mark) || nDiagonal(row, col, mark) || pDiagonal(row, col, mark)) {
		return true;
	}
	return false;
}

void updateBoard(int row, int col, char mark) {
	board[row * DIM + col] = mark;
	if (mark != '_' && gameOver(row, col, mark)) {
		winner = string(1, mark);
		cout << "winner: " << winner << endl;
	}
}

void setBoard() {
	for(int i = 0; i < DIM; i++) {
		for(int j = 0; j < DIM; j++) {
			updateBoard(i, j, '_');
		}
	}
}

//*****************************************************************************
struct prmt{
	int SocketFD;
	socklen_t size;
	sockaddr_in server_addr;
};

void sendTurn(int ConnectFD, char *message){
	char buff[BUFFSIZE];
  	bzero(buff, BUFFSIZE);
  	string TAB; //turna and board
  	TAB = TAB + to_string(playerTurn++) + " " + boardToMessage();
	const char *pTurn = TAB.c_str();
	send(ConnectFD, pTurn, strlen(pTurn), 0);
  	recv(ConnectFD, buff, BUFFSIZE, 0);
  	if(strcmp(buff, "read") != 0) {	
		printf("No lo recibio");
	}
	else printf("correcto!\n");
}

bool turnControl(int ConnectFD, char *message) {
	if (winner != "-") {
		string response;
		response = "2 " + boardToMessage() + winner;
		send(ConnectFD, response.c_str(), response.size(), 0);
		return true;
	}
	string playerBoard(message);
	string playerTurn;
	playerBoard.erase(0,1);
	int p = playerBoard.find(" ");
	playerTurn = playerBoard.substr(0, p);
	playerBoard = playerBoard.substr(p + 1);

	string RAB;
	if(!boardControl(playerBoard))
		RAB = boardToMessage();

	if (turn != stoi(playerTurn)) {
		RAB = "0 " + RAB;
		send(ConnectFD, RAB.c_str(), RAB.size(), 0);
	}
	else {
		char buff[BUFFSIZE]; ////////////////////////////////////////////////////////////////////////////////////
  		bzero(buff, BUFFSIZE);
  		RAB = "1 " + RAB;
		send(ConnectFD, RAB.c_str(), RAB.size(), 0);
		recv(ConnectFD, buff, BUFFSIZE, 0);

		string col(buff);
		string row;

	    int p = col.find(" ");
	    row = col.substr(0, p);
	    col = col.substr(p + 1);

	    updateBoard(stoi(row), stoi(col), marks[stoi(playerTurn)]);

	    displayBoard();
		turn = (turn + 1) % NPLAYERS;
	}
	return false;
}

bool actuator(int ConnectFD, char *message) {
	switch(message[0]) {
		case '0':
			sendTurn(ConnectFD, message);
			break;
		case '1':
			return turnControl(ConnectFD, message);
			//break;
	}
}


void *gameController(void *connection) {
	int SocketFD = ((prmt *)connection)->SocketFD;
	socklen_t size = ((prmt *)connection)->size;
	sockaddr_in server_addr = ((prmt *)connection)->server_addr;
	char buff[BUFFSIZE];
	int playerTurn;

	while(true) {
		int ConnectFD = accept(SocketFD, (struct sockaddr *)&server_addr, &size);
  		bzero(buff, BUFFSIZE);
		recv(ConnectFD, buff, BUFFSIZE, 0);
		if (actuator(ConnectFD, buff)) {
			close(ConnectFD);
			break;
		}
		close(ConnectFD);
	}
}

int main(int argc, char const *argv[])
{
	setMarks();
	setBoard();
	int SocketFD, ConnectFD;
	int portNum = 1100;
	bool isExit = false;

	sockaddr_in server_addr;
	socklen_t size;

	SocketFD = socket(AF_INET, SOCK_STREAM, 0);

	if (SocketFD < 0) 
	{
	    cout << "\nError establishing socket..." << endl;
	    exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portNum);


	if ((bind(SocketFD, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
	{
	    cout << "=> Error binding connection, the socket has already been established..." << endl;
	    return -1;
	}

	listen(SocketFD, 5);

	size = sizeof(server_addr);
	
	prmt connection;
	connection.SocketFD = SocketFD;
	connection.size = size;
	connection.server_addr = server_addr;	
	for (int i = 0; i < NPLAYERS; ++i) {
		pthread_create(&handlers[i], NULL, gameController, (void *)&connection);
	}

	for (int i = 0; i < NPLAYERS; ++i) {
		pthread_join(handlers[i], NULL);
	}
	close(SocketFD);

	return 0;
}
