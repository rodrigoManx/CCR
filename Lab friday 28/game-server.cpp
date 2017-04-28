#include "structs.h"

#define NPLAYERS 2
#define DIM 2
int SocketFD;
int SocketFD2;
struct sockaddr_in server_addr1;
struct sockaddr_in server_addr2;

int turn = 0;
bool gameIsFull = false;

vector <playerMove> queque1;
vector <playerMove> queque2;



pthread_t handlers[NPLAYERS];
string board[DIM*DIM];
string winner = "";

string players[NPLAYERS];
string marks[NPLAYERS];


int jugadasIncorrectas = NPLAYERS;
int jugadasHechas = 0;
int jugadasMandadas = 0;
bool jugadaslistas = 0;
//*****************************************************************************
bool rows(int col, string mark) {
    for (int i = 0; i < DIM; ++i) {
        if (board[i * DIM + col] != mark) return false;
    }
    return true;
}

bool cols(int row, string mark) {
    for (int i = 0; i < DIM; ++i) {
        if (board[row * DIM + i] != mark) return false;
    }
    return true;
}

bool nDiagonal(int row, int col, string mark) {
    if (row != col) return false;
    for (int i = 0; i < DIM; ++i) {
        if (board[i * DIM + i] != mark) return false;
    }
    return true;
}

bool pDiagonal(int row, int col, string mark) {
    if (row + col != DIM - 1) return false;
    for (int i = 0, j = DIM - 1; i < DIM; ++i, --j) {
        if (board[i * DIM + j] != mark) return false;
    }
    return true;
}

bool gameOver(int row, int col, string mark) {
    if(rows(col, mark) || cols(row, mark) || nDiagonal(row, col, mark) || pDiagonal(row, col, mark)) {
        return true;
    }
    return false;
}
bool gameOverM() {
    for(int i = 0; i < DIM; ++i) {
        for(int j = 0; j < DIM; ++j) {
            string aux = board[i * DIM + j];
            if (aux == "-")
                break;
            if (gameOver(i, j, aux)) {
                return true;
            }
        }   
    }
    return false;
}
bool empate() {
    for (int i = 0; i < DIM * DIM; ++i){
        if(board[i] == "-")
            return false;
    }
    return true;
}
void setBoard() {
    for (int i = 0; i < DIM * DIM; ++i) {
        board[i] = "-";
    }
}
void displayBoard() {
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            cout << board[i * DIM + j] << "\t";
        }
        cout << endl;
    }
}
void updateBoard(int row, int col, string mark) {
    board[row * DIM + col] = mark;
}

void llenarTablero() {
    for (playerMove i : queque2) {
		string aux = marks[stoi(i.ID)];
        updateBoard(i.row, i.col, aux);
    }

}
void setMarks() {
	for (int i = 0, j = 0; i < NPLAYERS; ++i) {
		if(i == 26) {
			j = 6;
		}
		char a = i + j + 65;
		string aux(1,a);
		marks[i] = aux;
	}
}

void validateQueque1() {
	int count = 0;
	for(int i = queque1.size() - 1; i >= 0; --i) {
		queque1[i].condition = 1;
		if(queque1[i].row > DIM - 1 || queque1[i].row < 0 || queque1[i].col > DIM - 1 || queque1[i].col < 0) {
			++count;
			queque1[i].condition = 0;
		}
		else if(board[queque1[i].row * DIM + queque1[i].col] != "-") {
			++count;
			queque1[i].condition = 0;
		}
		else{
			for(int j = 0; j < i; ++j) {
				if(queque1[i].row == queque1[j].row && queque1[i].col == queque1[j].col) {
					++count;
					queque1[i].condition = 0;
					break;
				}
			}
			for(int k = 0; k < queque2.size(); ++k) {
				if(queque1[i].row == queque2[k].row && queque1[i].col == queque2[k].col) {
					++count;
					queque1[i].condition = 0;
					break;
				}
			}
		}

	}
	for (playerMove i : queque1)
		queque2.push_back(i);
	if(count == 0)
		jugadaslistas = 1;
	jugadasIncorrectas = count;
	jugadasHechas = 0;
	queque1.clear();
}

void validateQueque2() {
	for (int i = 0; i < queque2.size(); ++i) {
		if (queque2[i].condition == 0) {
			queque2.erase(queque2.begin() + i);
		}
	}
}

//*****************************************************************************

void fillQueque1(int ConnectFD, string ID, vector <string> parameters) {
	cout << "player id " << ID << endl;
	cout << parameters[0] << " " << parameters[1] << endl;

	cout << queque1.size() << endl;

	playerMove pm(stoi(parameters[0]), stoi(parameters[1]), ID);
    queque1.push_back(pm);
    ++jugadasHechas;
}

void askForMoves(int ConnectFD) {
	string message;
	if (!jugadaslistas) {
		message = "noEstanListos";
		packet response(message, mess); 
		sendPacket(ConnectFD, response);
		return;
	}
	message = "listos";
	packet response(message, mess); 
	sendPacket(ConnectFD, response);
	packet p(to_string(NPLAYERS), moves); 
	sendPacket(ConnectFD, p);

	for (playerMove i : queque2) {
		packet row(to_string(i.row), mess);
		packet col(to_string(i.col), mess);
		packet mark(marks[stoi(i.ID)], mess);
		sendPacket(ConnectFD, row);
		sendPacket(ConnectFD, col);
		sendPacket(ConnectFD, mark);
	}
	++jugadasMandadas;
}

void checkMyMove(int ConnectFD, string ID, vector <string> parameters) {
	for (playerMove i : queque2) {
		if(i.ID == ID) {
			string response = to_string(i.condition);
			packet p(response, mess);
			sendPacket(ConnectFD, p);
			return;
		}
	}
	string response = "0";
	packet p(response, mess);
	sendPacket(ConnectFD, p);
}

bool methodsPool(int ConnectFD, string &ID, string &methodName, vector <string> &parameters) {
    recvMethod(ConnectFD, ID, methodName, parameters);
    if (methodName == "move") {
        fillQueque1(ConnectFD, ID, parameters);
        return true;
    }
    else if(methodName == "askForMoves") {
    	askForMoves(ConnectFD);
        return true;
    }
    else if(methodName == "checkMyMove") {
    	checkMyMove(ConnectFD, ID, parameters);
        return true;
    }
    return false;
}
bool getPlayerMethod(int ConnectFD) {
	string playerID, methodName;
	vector <string> parameters;
	return 	methodsPool(ConnectFD, playerID, methodName, parameters);
}

bool logIn(int ConnectFD, string clientName) {
	int t = turn++;
	if(t <= NPLAYERS) {
		players[t] = clientName;
		string IDmessage = to_string(t);
		packet IDPacket(IDmessage, login);
		sendPacket(ConnectFD, IDPacket);

		packet boardDim(to_string(DIM), mess);
		sendPacket(ConnectFD, boardDim);

		int j = 0;
		if(t == 26) {
			j = 6;
		}
		char a = t + j + 65;
		string aux(1,a);
		
		packet playerMark(aux, mess);
		sendPacket(ConnectFD, playerMark);
		return true;
	}
	
	string response;
	response = "Game full!";
	packet responsePacket(response, mess);
	sendPacket(ConnectFD, responsePacket);
	return false;
}

bool logOut(int ConnectFD, string ID) {
	string response;
	if(stoi(ID) <= NPLAYERS - 1) {
	    players[stoi(ID)] = "";
		response = "Closed session";
		packet responsePacket(response, logout);
		sendPacket(ConnectFD, responsePacket);
		return true;
	}
	response = "You are not in this game";
	packet responsePacket(response, mess);
	sendPacket(ConnectFD, responsePacket);
	return false;
}

bool controller(int ConnectFD) {
	string message;
    int messageType;
    if (!recvPacket(ConnectFD, messageType, message)) 
        return false;

	if(messageType == login) {
		if(logIn(ConnectFD, message))
			cout << "Player " << message << " successfully registered" << endl;
		else cout << "Unsuccessful registration" << endl;
		return true;
	}
	else if(messageType == logout) {
		string clientName = players[stoi(message)];
		if (logOut(ConnectFD, message))
			cout << "Session closed successfully for the client " << clientName << endl;
		else cout << "Logout failed" << endl;
		return true;

	}
	else if(messageType == method){
		if (getPlayerMethod(ConnectFD))
			cout << "Processed method" << endl;
		else 
			cout << "Unknown method" << endl;
		return true;
	}
	else
		cout << "Unknown message type" << endl;
	return false;
}


void creatingSocket(int &socketFD, int portNum, sockaddr_in &server_addr) {
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portNum);

    if ((bind(socketFD, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) {
	    cout << "=> Error binding connection, the socket has already been established..." << endl;
	    exit(1);
	}
	
	listen(socketFD, 5);
}


void *gameController2(void *connection) {
	int SocketFD = ((prmt *)connection)->SocketFD;
	socklen_t size = ((prmt *)connection)->size;
	sockaddr_in server_addr = ((prmt *)connection)->server_addr;

	int ConnectFD = accept(SocketFD, (struct sockaddr *)&server_addr, &size);
	controller(ConnectFD);
	close(ConnectFD);
}

int main(int argc, char const *argv[]) {
	setBoard();
	creatingSocket(SocketFD, 1100, server_addr1);
	creatingSocket(SocketFD2, 1101, server_addr2);

	prmt connection;
	connection.SocketFD = SocketFD;
	connection.size = sizeof(server_addr1);
	connection.server_addr = server_addr1;

	for (int i = 0; i < NPLAYERS; ++i)
		pthread_create(&handlers[i], NULL, gameController2, (void *)&connection);

	for (int i = 0; i < NPLAYERS; ++i)
		pthread_join(handlers[i], NULL);

	if (turn == NPLAYERS) {
		setMarks();
	}

	do {
		for (int i = 0; i < NPLAYERS; ++i)
			pthread_create(&handlers[i], NULL, gameController2, (void *)&connection);

		for (int i = 0; i < NPLAYERS; ++i)
			pthread_join(handlers[i], NULL);

		if (jugadasHechas == jugadasIncorrectas && jugadaslistas == 0) {
			validateQueque1();
			validateQueque2();
		}
		if (jugadasMandadas == NPLAYERS)
		{
			llenarTablero();
			displayBoard();
		    if (gameOverM() || empate()) {
		        break;
		    }
			queque2.clear();
			queque1.clear();
			jugadasIncorrectas = NPLAYERS;
			jugadasHechas = 0;
			jugadasMandadas = 0;
			jugadaslistas = 0;
			cout << " jugadasMandadas " << jugadasMandadas << endl;
		}
	}
	while(true);

	for (int i = 0; i < NPLAYERS; ++i)
		pthread_create(&handlers[i], NULL, gameController2, (void *)&connection);

	for (int i = 0; i < NPLAYERS; ++i)
		pthread_join(handlers[i], NULL);
	
	close(SocketFD);
	return 0;
}
