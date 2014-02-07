// A Simple Http Server

#include "mywinsock.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	myWinSockHttpServer a(12345); FILE *f;
	char url[100],data[100],method[100],*newurl=url,*res=NULL;
	a.start();
	while(true){
		memset(data,0,100);
		a.myAccept();
		a.httpAnalyser(a.myRecvString(),method,url,data);
		a.urlStringReplacer(newurl);
		// ----
		printf(newurl);
		printf("\n");
		printf(data);
		printf("\n");
		// ----
		f=fopen(newurl,"rb");
		if(f==NULL){
			a.mySendHttp("404 NOT FOUND",404);
			//a.closeClient();
			continue;
		}
		fseek(f,0,SEEK_END);
		int fSize=ftell(f);
		rewind(f);
		if(res!=NULL) delete [] res;
		res=(char*)malloc(fSize);
		fread(res,1,fSize,f);
		fclose(f);
		a.mySendHttp(res);
		//a.closeClient();
	}
	a.close();
	return 0;
}
