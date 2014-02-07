// Example3

#include "mywinsock.h"

int main(){
	myWinSockTcpClient a("127.0.0.1",12345);
	a.start();
	a.mySendString("from tcp client.");
	printf(a.myRecvString());    // Output: from tcp server.
	a.close();
	return 0;
}
