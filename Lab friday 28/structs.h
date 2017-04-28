#include "packets.h"

struct prmt{
	int SocketFD;
	socklen_t size;
	sockaddr_in server_addr;
};

struct playerMove {
	int row;
	int col;
	string ID;
	int condition;
	playerMove(int r, int c, string id);
	void validateMove(int v);
};

playerMove::playerMove(int r, int c, string id) {
	row = r;
	col = c;
	ID = id;
	condition = -1;
}
void playerMove::validateMove(int v) {
	condition = v;
}