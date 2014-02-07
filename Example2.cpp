#include "mywinsock.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	try{
		myWinSockHttpClient a("127.0.0.1","/showGet.php",MY_GET,12345); 
		a.start();
		printf(a.myHttpReqOnce(0,NULL,"user=a&pw=b"));
		a.close();
	}
	catch(int err){
		printf("%d",err);
	}
	return 0;
}

// showGet.php

/*
<?php
  var_dump($_GET);
?>
*/


// Output

/*

HTTP/1.1 200 OK
Date: Fri, 07 Feb 2014 14:20:18 GMT
Server: Apache/2.2.21 (Win32) PHP/5.3.10
X-Powered-By: PHP/5.3.10
Content-Length: 82
Content-Type: text/html

array(2) {
  ["user"]=>
  string(1) "a"
  ["pw"]=>
  string(1) "b"
}

*/
