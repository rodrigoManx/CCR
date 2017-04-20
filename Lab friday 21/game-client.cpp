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
#include <netdb.h>
#include <sstream>
#include <vector>
#define BUFFSIZE 250

using namespace std;

string myTurn;
string myBoard;
char buffer[BUFFSIZE];

int SocketFD;
struct sockaddr_in server_addr;


void recvTurn(int SocketFD, char *buff) {
    recv(SocketFD, buff, BUFFSIZE, 0);
    string board(buff);
    string turn;
    int p = board.find(" ");
    turn = board.substr(0, p);
    board = board.substr(p + 1);
    myTurn = turn;
    myBoard = board;
    send(SocketFD, "read", 4, 0); 
}


void creatingSocket() {
    int portNum = 1100; 
    bool isExit = false;

    SocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if (SocketFD < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }   

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    int Res = inet_pton(AF_INET, "192.168.0.20", &server_addr.sin_addr);

    if (0 > Res)
    {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(1);
    }
    else if (0 == Res)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(1);
    }


    if (connect(SocketFD,(struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
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

int main()
{
    creatingSocket();

    char buff[BUFFSIZE];
    bzero(buff, BUFFSIZE);
    send(SocketFD, "0", 1, 0);
    recvTurn(SocketFD, buff);
    cout << myTurn << endl;
    cout << myBoard << endl;
    cout << "__________________________________________________________" << endl;
    
    close(SocketFD);

    while(true) {
        creatingSocket();
        string request;
        request = request + "1" + myTurn + " " + myBoard;
        strcpy(buff, request.c_str());
        send(SocketFD, buff, strlen(buff), 0);
        bzero(buff, BUFFSIZE);
        recv(SocketFD, buff, BUFFSIZE, 0);
        
        string updatedBoard(buff);
        string response;
        splitString(updatedBoard, response); 


        if (response == "2") {
            char winner = updatedBoard[updatedBoard.size() - 1];
            updatedBoard.erase(updatedBoard.size() - 1, 1);
            cout << updatedBoard;
            cout << "the player " << winner << " has won." << endl;
            break;
        }
        else if(response == "1") {
            /*printf("It's your turn:\n");
            printf("Enter the row, a space and then the column where you want to put your mark.\n");
            printf("e.g. 1 2\nMean you will mark the position in the 1st row and the 2nd column.\n");
            printf("_\tx\t_\n_\t_\t_\n_\t_\t_\n");*/
            myBoard = updatedBoard;
            cout << endl << myBoard << endl << "==> "; 
            string move;
            getline(cin, move);
            send(SocketFD, move.c_str(), move.size(), 0);
            cout << "__________________________________________________________" << endl;
        }
        else {
            if (updatedBoard != "") {
                myBoard = updatedBoard;
                cout << endl << myBoard << endl;
                cout << "__________________________________________________________" << endl;
            }
        }
        close(SocketFD);

    }

    return 0;
}
