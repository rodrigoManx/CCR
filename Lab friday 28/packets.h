#include "lib.h"

enum packetType { 
    login = 0,
    logout = 1,
    method = 2,
    mess = 3,
    moves = 4
};

enum packetProtocol {
    headerSize = 8,
    headerSize2 = 12
};
struct packet {
    string messageLength;
    string message;
    string messageType;
    packet(string m, int mt);
};
packet::packet(string m, int mt) {
    message = m;
    int r = 4 - to_string(m.size()).size();
    messageLength = string(r, '0') + to_string(m.size());
    r = 4 - to_string(mt).size();
    messageType = string(r, '0') + to_string(mt);
}

bool processMessageHeader(string header, int &messageLength, int &messageType) {
    if (header.size() > 8) {
        cout << "wrong header!" << endl;
        return false;
    }
    messageLength = stoi(header.substr(0,4));
    messageType = stoi(header.substr(4,4));
    return true;
}

void sendPacket(int FD, packet request) {
    string r = request.messageLength + request.messageType;
    send(FD, r.c_str(), r.size(), 0);
    send(FD, request.message.c_str(), request.message.size(), 0);
}

bool recvPacket(int FD, int &messageType, string &message) {
    char buff[BUFFSIZE];
    bzero(buff, BUFFSIZE);
    int messageLength;
    recv(FD, buff, headerSize, 0);

    if (!processMessageHeader(string(buff), messageLength, messageType)) 
        return false;
    
    bzero(buff, BUFFSIZE);
    recv(FD, buff, messageLength, 0);
    message = string(buff);
    return true;
}

struct methodPacket {
    string IDlength;
    string ID;
    string nameLength;
    string name;
    string parametersNumber;
    vector<string> parametersLength;
    vector<string> parameters;
    methodPacket();
    methodPacket(string id, string n, vector <string> p);
};
methodPacket::methodPacket() {} 
methodPacket::methodPacket(string id, string n, vector <string> p) {
    ID = id;
    name = n;
    parameters = p;
    int r = 4 - to_string(ID.size()).size();
    IDlength = string(r, '0') + to_string(ID.size());
    r = 4 - to_string(name.size()).size();
    nameLength = string(r, '0') + to_string(name.size());
    r = 4 - to_string(p.size()).size();
    parametersNumber = string(r, '0') + to_string(p.size());
    for (int i = 0; i < stoi(parametersNumber); ++i) {
        r = 4 - to_string(parameters[i].size()).size();
        parametersLength.push_back(string(r, '0') + to_string(parameters[i].size()));
    }
}

bool processMethodHeader(string header, int &IDLength, int &nameLength, int &parametersNumber) {
    if (header.size() != 12) {
        cout << "wrong header!" << endl;
        return false;
    }
    IDLength = stoi(header.substr(0,4));
    nameLength = stoi(header.substr(4,4));
    parametersNumber = stoi(header.substr(8,4));
    return true;
}

void sendPacket(int FD, methodPacket m) {
    string header = m.IDlength + m.nameLength + m.parametersNumber;
    send(FD, header.c_str(), headerSize2, 0);
    send(FD, m.ID.c_str(), m.ID.size(), 0);
    send(FD, m.name.c_str(), m.name.size(), 0);
    for (int i = 0; i < stoi(m.parametersNumber); ++i) {
        send(FD, m.parametersLength[i].c_str(), headerSize/2, 0);
    }
    for (int i = 0; i < stoi(m.parametersNumber); ++i) {
        send(FD, m.parameters[i].c_str(), m.parameters[i].size(), 0);
    }
}
bool recvMethod(int FD, string &ID, string &method, vector<string> &parameters) {
    char buff[BUFFSIZE];
    int IDLength, nameLength, parametersNumber;
    bzero(buff, BUFFSIZE);
    recv(FD, buff, headerSize2, 0);

    if (!processMethodHeader(string(buff), IDLength, nameLength, parametersNumber)) 
        return false;
    
    bzero(buff, BUFFSIZE);
    recv(FD, buff, IDLength, 0);
    ID = string(buff);

    bzero(buff, BUFFSIZE);
    recv(FD, buff, nameLength, 0);
    method = string(buff);

    vector <int> parametersLength;
    for (int i = 0; i < parametersNumber; ++i) {
        bzero(buff, BUFFSIZE);
        recv(FD, buff, headerSize/2, 0);
        parametersLength.push_back(stoi(string(buff)));
    }

    for (int i = 0; i < parametersNumber; ++i) {
        bzero(buff, BUFFSIZE);
        recv(FD, buff, parametersLength[i], 0);
        parameters.push_back(string(buff));
    }
    return true;
}