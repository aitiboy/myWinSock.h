// Example3

#include "mywinsock.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	myWinSockTcpServer a(12345);
	a.start();
	a.myAccept();
	printf(a.myRecvString());      // Output: from tcp client.
	a.mySendString("from tcp server.");   
	a.close();
	return 0;
}
