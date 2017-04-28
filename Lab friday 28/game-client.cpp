#include "packets.h"
#define DIM 3
int SocketFD;
int SocketFD2;
struct sockaddr_in server_addr;
int ID;
string name;
char buff[BUFFSIZE];
string board[DIM * DIM];
int state = 0;
struct playerMove {
    int row;
    int col;
    string mark;
    playerMove(int r, int c, string m);
};

playerMove::playerMove(int r, int c, string m) {
    row = r;
    col = c;
    mark = m;;
}
vector <playerMove> queque;
//***************************************************
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
    for (playerMove i : queque) {
        updateBoard(i.row, i.col, i.mark);
    }

}
//***************************************************
void creatingSocket() {
    int portNum = 1100; 
    bool isExit = false;

    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0) {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    int Res = inet_pton(AF_INET, "192.168.0.20", &server_addr.sin_addr);
    if (0 > Res) {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(1);
    }

    else if (0 == Res) {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(1);
    }

    if (connect(SocketFD,(struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(SocketFD);
        exit(1);
    }
}

void splitString(string &a, string &b) {
    int p = a.find(" ");
    b = a.substr(0, p);
    a = a.substr(p +1);
}


methodPacket *methodsPool(string methodName) {
    vector <string> parameters;
    methodPacket *m; 
    if (methodName == "move") {
        cout << "Enter the row then the column where you want to play" << endl;
        string row, col;
        cout << "row: ";
        cin >> row;
        cout << "column: ";
        cin >> col;
        parameters.push_back(row);
        parameters.push_back(col);
        m = new methodPacket(to_string(ID), "move", parameters);
        return m;
    }
    else if(methodName == "revisarMiJugada") {
        m = new methodPacket(to_string(ID), "revisarMiJugada", parameters);
        return m;
    }
    else if(methodName == "askForMoves") {
        m = new methodPacket(to_string(ID), "askForMoves", parameters);
        return m;
    }
    return NULL;
}

bool moveCondition() {
    methodPacket *m;
    m = methodsPool("moveCondition");
    if(m == NULL) {
        sendPacket(SocketFD, *m);
    }
    cout << "Unregistered method" << endl;
    return false;
}
bool move() {
    packet methodRequest(name, method);
    sendPacket(SocketFD, methodRequest);

    methodPacket *m;
    m = methodsPool("move");
    sendPacket(SocketFD, *m);
    state = 1;

    return true;
}
bool revisarMiJugada() {
    packet methodRequest(name, method);
    sendPacket(SocketFD, methodRequest);

    methodPacket *m;
    m = methodsPool("revisarMiJugada");
    sendPacket(SocketFD, *m);
    string message;
    int messageType;
    recvPacket(SocketFD, messageType, message);
    if (message == "1")
        state = 2;
    else if(message == "0")
        state = 0;
    cout << "mi jugada " << message << endl;
    return true;
}

bool askForMoves() {
    packet methodRequest(name, method);
    sendPacket(SocketFD, methodRequest);
    methodPacket *m;
    m = methodsPool("askForMoves");
    cout << "check" << endl;
    sendPacket(SocketFD, *m);
    string message;
    int messageType;
    recvPacket(SocketFD, messageType, message);
    if (message != "listos" || messageType != mess)
        return false;
    else {
        recvPacket(SocketFD, messageType, message);
        if (messageType != moves)
            return false;
        else {
            int count = stoi(message);
            for (int i = 0; i < count; ++i) {
                recvPacket(SocketFD, messageType, message);
                int row = stoi(message);
                recvPacket(SocketFD, messageType, message);
                int col = stoi(message);
                recvPacket(SocketFD, messageType, message);
                string mark = message;
                playerMove j(row, col, mark);
                queque.push_back(j);
            }
        }
    }
    state = 0;
    llenarTablero();
    displayBoard();
    if (gameOverM()) {
        exit(1);
    }
    queque.clear();
    return true;
}

bool logOut() {
    cout << "Closing session..." << endl;
    string IDmessage = to_string(ID);

    packet logoutRequest(IDmessage, logout);
    sendPacket(SocketFD, logoutRequest);
    
    string message;
    int messageType;
    if (!recvPacket(SocketFD, messageType, message)) 
        return false;

    cout << message << endl;
    if (messageType != 1) {
        return false;
    }
    return true;
}

bool logIn() {
    cout << "Enter your name: ";
    getline(cin, name);

    packet loginRequest(name, login);
    sendPacket(SocketFD, loginRequest);
    
    string message;
    int messageType;
    if (!recvPacket(SocketFD, messageType, message)) 
        return false;

    if (messageType != 0) {
        cout << message << endl;
        return false;
    }
    ID = stoi(message);
    cout << "registered!" << endl;
    cout << "your ID is: " << ID << endl;
    return true;
}

int main() {
    setBoard();
    creatingSocket();
    logIn();
    close(SocketFD);

    while(true) {
        creatingSocket();
        if (state == 0)
            move();
        else if(state == 1){
            revisarMiJugada();
        }
        else if(state == 2) {
            askForMoves();
            for (playerMove i : queque) {
                cout << i.row << endl;
                cout << i.col << endl;
                cout << i.mark << endl;
            }
        }

        close(SocketFD);
    }

    creatingSocket();
    logOut();
    close(SocketFD);

    return 0;
}
