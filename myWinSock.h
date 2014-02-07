#include <WinSock2.h>
#pragma comment(lib,"ws2_32")
#include <memory.h>
#include <string>
#include <sstream>
#include <map>

	bool isWSARunning=false;

	// method
	static const int MY_UNKNOWN=0,
					 MY_POST=1,
					 MY_GET=2;
	// buffsize
	static const int MY_BUFF_SIZE=10000;
	// errors
	static const int MY_SOCKET_ERROR=11,
					 MY_BIND_ERROR=12,
					 MY_LISTEN_ERROR=13,
					 MY_CONNNECT_ERROR=14,
					 MY_STAT_ERROR=15,
					 MY_CLIENT_ERROR=16,
					 MY_UNKNOWN_METHOD_ERROR=17;

using namespace std;

class myWinSock{
public:
	sockaddr * getAddr(const char * IP,int port){
		sockaddr_in * res=new sockaddr_in;
		res->sin_addr.S_un.S_addr=(IP[0]=='*')?INADDR_ANY:inet_addr(IP);
		res->sin_port=htons(port);
		res->sin_family=AF_INET;
		return (sockaddr*)res;
	}
	SOCKET createSocket(int protocol){
		if(!isWSARunning) throw MY_STAT_ERROR;
		SOCKET s;
		if(protocol==IPPROTO_TCP)
			s=socket(AF_INET,SOCK_STREAM,protocol);
		else
			s=socket(AF_INET,SOCK_DGRAM,protocol);
		if(s==SOCKET_ERROR) throw MY_SOCKET_ERROR;
		return s; 
	}
	char * httpGetPacket(char * host,char * url,char *data="",char *cookie=""){
		char *res=(char*)malloc(MY_BUFF_SIZE);
		wsprintf(res,"GET %s?%s HTTP/1.1\r\n"
			"Accept: */*\r\n"
			"Accept-Language: zh-cn\r\n"
			"Host: %s\r\n"
			"Cookie: %s\r\n\r\n",
			url,data,host,cookie);
		return res;
	}
	char * httpPostPacket(char * host,char * url,char *data="",char *cookie=""){
		char *res=(char*)malloc(MY_BUFF_SIZE);
		wsprintf(res,"POST %s HTTP/1.1\r\n"
			"Accept: */*\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Host: %s\r\n"
			"Content-Length: %d\r\n"
			"Connection: Keep-Alive\r\n"
			"Cookie: %s\r\n\r\n%s",
			url,host,strlen(data),cookie,data);
		return res;
	}
	char * httpResPacket(const char *data,int resCode=200,char * contentType="text/html"){
		char *res=(char*)malloc(MY_BUFF_SIZE);
		wsprintf(res,"HTTP/1.1 %d OK\r\n"
				"Content-Length: %d\r\n"
				"Content-Type: %s\r\n\r\n%s",
				resCode,strlen(data),contentType,data);
		return res;
	}

// --------------------------- EXPORTS -------------------------------------

	int start(){
		if(isWSARunning) return MY_STAT_ERROR;
		isWSARunning=true;
		WSADATA wsaData;
		return WSAStartup(MAKEWORD(2,2),&wsaData);
	}
	int close(){
		if(!isWSARunning) return MY_STAT_ERROR;
		isWSARunning=false;
		return WSACleanup();
	}
	int myBind(SOCKET &s,int port,int protocol){
		if(!isWSARunning) throw MY_STAT_ERROR;
		int tmp;
		s=this->createSocket(protocol);
		if(s==SOCKET_ERROR) return SOCKET_ERROR;
		sockaddr * addr=getAddr("*",port);
		if((tmp=bind(s,addr,sizeof(*addr)))==SOCKET_ERROR){
			throw MY_BIND_ERROR;
			closesocket(s);
			s=SOCKET_ERROR;
		}
		delete addr;
		return tmp;
	}
	int myListen(SOCKET &res,int port){
		if(!isWSARunning) throw MY_STAT_ERROR;
		this->myBind(res,port,IPPROTO_TCP);
		if(res==SOCKET_ERROR) return SOCKET_ERROR;
		int tmp=listen(res,5);
		if(tmp==SOCKET_ERROR){ 
			throw MY_LISTEN_ERROR;
			closesocket(res);
		}
		return tmp;
	}
	int myConnect(SOCKET &s,const char * IP,int port){
		if(!isWSARunning) throw MY_STAT_ERROR;
		s=createSocket(IPPROTO_TCP);
		if(s==SOCKET_ERROR) return SOCKET_ERROR;
		sockaddr * addr=getAddr(IP,port);
		int tmp2=connect(s,addr,sizeof(*addr));
		delete addr;
		if(tmp2==SOCKET_ERROR){
			throw MY_CONNNECT_ERROR;
			closesocket(s);
		}
		return tmp2;
	}
	int mySendTo(SOCKET &s,const char * IP,int port,const char * strdata){
		if(!isWSARunning) throw MY_STAT_ERROR;
		sockaddr * addr=this->getAddr(IP,port);
		int res=sendto(s,strdata,strlen(strdata),0,addr,sizeof(*addr));
		delete addr;
		return res;
	} 
};

class myWinSockStruct: protected myWinSock{
public:
	int port; SOCKET s; bool isSocketRunning;
	int closeThis(){
		if(this->isSocketRunning){
			this->isSocketRunning=false;
			return closesocket(this->s);
		}
		return MY_STAT_ERROR;
	}
	int close(){
		this->closeThis();
		if(!isWSARunning) return MY_STAT_ERROR;
		isWSARunning=false;
		return WSACleanup();
	}
	~myWinSockStruct(){
		this->closeThis();
	}
};

class myWinSockTcpServer: public myWinSockStruct{
public:
	SOCKET sClient;
	sockaddr_in addr;
	int addrLen;
	myWinSockTcpServer(int iPort=80){
		this->addrLen=sizeof(this->addr);
		myWinSock a;
		a.start();
		this->port=iPort;
		this->isSocketRunning=false;
		this->s=this->createSocket(IPPROTO_TCP);
		this->sClient=INVALID_SOCKET;
	}
	int start(int iPort=-1){
		if(iPort!=-1) this->port=iPort;
		if(this->isSocketRunning) return MY_STAT_ERROR;
		this->isSocketRunning=true;
		return this->myListen(this->s,this->port);
	}
	void myAccept(){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		if(this->sClient!=INVALID_SOCKET) closesocket(this->sClient);
		this->sClient=accept(this->s,(sockaddr*)&(this->addr),&(this->addrLen));
		if(this->sClient==INVALID_SOCKET) this->myAccept();
	}
	char * myRecv(int &recvLen){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		if(this->sClient==INVALID_SOCKET) throw MY_CLIENT_ERROR;
		char *res=new char[MY_BUFF_SIZE];
		recvLen=recv(this->sClient,res,MY_BUFF_SIZE,0);
		return res;
	}
	char * myRecvString(int *recvLen=NULL){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		if(this->sClient==INVALID_SOCKET) throw MY_CLIENT_ERROR;
		char *res=new char[MY_BUFF_SIZE];
		int tmp=recv(this->sClient,res,MY_BUFF_SIZE,0);
		res[tmp]='\0';
		if(recvLen!=NULL) *recvLen=tmp;
		return res;

	}
	int mySend(const char * data){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		if(this->sClient==INVALID_SOCKET) throw MY_CLIENT_ERROR;
		return send(this->sClient,data,sizeof(data),0);
	}
	int mySendString(const char * str){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		if(this->sClient==INVALID_SOCKET) throw MY_CLIENT_ERROR;
		return send(this->sClient,str,strlen(str),0);
	}
	int closeClient(){
		if(this->sClient==INVALID_SOCKET) throw MY_CLIENT_ERROR;
		int res=closesocket(this->sClient);
		this->sClient=INVALID_SOCKET;
		return res;
	}
	int closeThis(){
		if(this->sClient!=INVALID_SOCKET){
			closesocket(this->sClient);
			this->sClient=INVALID_SOCKET;
		}
		if(this->isSocketRunning){
			this->isSocketRunning=false;
			return closesocket(this->s);
		}
		return MY_STAT_ERROR;
	}
	int close(){
		this->closeThis();
		if(!isWSARunning) return MY_STAT_ERROR;
		isWSARunning=false;
		return WSACleanup();
	}
	~myWinSockTcpServer(){
		this->closeThis();
	}
};

class myWinSockTcpClient: public myWinSockStruct{
public:
	char *IP;
	myWinSockTcpClient(){}
	myWinSockTcpClient(char *host,int iPort=80){
		myWinSock a;
		a.start();
		this->port=iPort;
		this->IP=host;
		this->isSocketRunning=false;
		this->s=this->createSocket(IPPROTO_TCP);
	}
	int start(int iPort=-1){
		if(iPort!=-1) this->port=iPort;
		if(this->isSocketRunning) return MY_STAT_ERROR;
		this->isSocketRunning=true;
		return this->myConnect(this->s,this->IP,this->port);
	}
	char * myRecv(int &recvLen){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		char *res=new char[MY_BUFF_SIZE];
		recvLen=recv(this->s,res,MY_BUFF_SIZE,0);
		return res;
	}
	char * myRecvString(int *recvLen=NULL){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		char *res=new char[MY_BUFF_SIZE];
		int tmp=recv(this->s,res,MY_BUFF_SIZE,0);
		res[tmp]='\0';
		if(recvLen!=NULL) *recvLen=tmp;
		return res;
	}
	int mySend(const char * data){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		return send(this->s,data,sizeof(data),0);
	}
	int mySendString(const char * str){
		if(!this->isSocketRunning) throw MY_STAT_ERROR;
		return send(this->s,str,strlen(str),0);
	}
};

class myWinSockHttpClient: public myWinSockTcpClient{
public:
	char *url; int defMethod;
	myWinSockHttpClient(char *host,char *cUrl=NULL,int method=0,int iPort=80){
		myWinSock a;
		a.start();
		this->url=cUrl;
		this->port=iPort;
		this->defMethod=method;
		this->IP=host;
		this->isSocketRunning=false;
		this->s=this->createSocket(IPPROTO_TCP);
	}
	int mySendHttp(int method=0,char *cUrl=NULL,char *data="",char *cookie=""){
		if(cUrl!=NULL)
			if(cUrl!=url){
				delete url;
				url=cUrl;
			};
		if(method!=0)
			this->defMethod=method;
		else
			method=this->defMethod;
		if(method==MY_UNKNOWN) throw MY_UNKNOWN_METHOD_ERROR;
		if(method==MY_GET)
			return this->mySendString(this->httpGetPacket(this->IP,this->url,data,cookie));
		else
			return this->mySendString(this->httpPostPacket(this->IP,this->url,data,cookie));
	}
	char * myRecvHttp(int *len=NULL){
		char *buff=new char[MY_BUFF_SIZE];
		int recvLen=recv(this->s,buff,MY_BUFF_SIZE,0);
		int recvedLen=0;
		while(recvLen>0){
			recvedLen+=recvLen;
			recvLen=recv(this->s,&buff[recvedLen],MY_BUFF_SIZE-recvedLen,0);
		}
		if(len!=NULL) *len=recvedLen;
		if(recvLen==-1){ delete buff; buff=NULL; }
		else buff[recvedLen]='\0';
		return buff;
	}
	char * myHttpReqOnce(int method=0,char *cUrl=NULL,char *data="",char *cookie=""){
		bool flag=!this->isSocketRunning; bool wsaStat=!isWSARunning;
		if(flag) this->start();
		this->mySendHttp(method,cUrl,data,cookie);
		char *res=this->myRecvHttp();
		if(flag) this->closeThis();
		if(wsaStat) this->close();
		return res;
	}
};

class myWinSockHttpServer: public myWinSockTcpServer{
public:
	myWinSockHttpServer(int iPort=80): myWinSockTcpServer(iPort){}
	int mySendHttp(const char *data,int resCode=200,char * contentType="text/html"){
		return this->mySendString(this->httpResPacket(data,resCode,contentType));
	}
	void urlStringReplacer(char *&p){
		char *s=p;
		while(*s!='\0'){
			if(*s=='/') *s='\\';
			s++;
		}
		if(*p=='\\') p++;
	}
	void httpAnalyser(char *recv,char*fmethod,char*url,char*data,map<string,string> *list=NULL){
		char *ver=new char[10]; char *tmp=new char[MY_BUFF_SIZE],tmp3[100],tmp4[100];
		stringstream s; stringstream ss;
		s<<recv;
		s>>fmethod>>url>>ver;
		s.getline(tmp,MY_BUFF_SIZE);
		while(tmp[0]!='\r'){
			sscanf_s(tmp,"%s: %s\r",tmp3,tmp4);
			if(list!=NULL)
				(*list)[tmp3]=tmp4;
			s.getline(tmp,MY_BUFF_SIZE);
		}
		bool isGet=false;
		char *p=url;
		while(*p){
			if(*p=='?'){
				*p='\n';
				isGet=true;
				break;
			}
			p++;
		}
		if(isGet){ 
			s.str("");
			s<<url;
			s>>url>>data;
		}
		delete [] ver;
		delete [] tmp;
		if(fmethod[0]=='P'){
			string sss("");
			if(isGet) sss.append(data).append("&"); 
			sss.append(this->myRecvString());
			strcpy(data,sss.c_str());
		}
	}
};
