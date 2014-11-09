#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

int main (int argc, char* argv[])
{
    int listenFd, portNo;
    bool loop = false;
    struct sockaddr_in svrAdd;
    struct hostent *server;
    
    if(argc < 3)
    {
        cerr<<"Syntax : ./client <host name> <port>"<<endl;
        return 0;
    }
    
    portNo = atoi(argv[2]);
    
    if((portNo > 65535) || (portNo < 1024))		//making sure client enters port number in valid range
    {
        cerr<<"Please enter port number between 1024 - 65535"<<endl;
        return 0;
    }       
    
    //create client socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }
    
    server = gethostbyname(argv[1]);
    
    if(server == NULL)
    {
        cerr << "Host does not exist" << endl;
        return 0;
    }
    
    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;
    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);
    svrAdd.sin_port = htons(portNo);
    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
	
	if (checker < 0)
    {
        cerr << "Cannot connect!" << endl;
        return 0;
    }
    
	char group[2] = {0};
	cin.clear();
    cout << "Choose Group No: ";
	cin>>group;
	
    //send stuff to server periodically
    for(;;)
    {
		char message[512]= "Hello, I'm a client from Group: ";
		strcat(message,group);
		write(listenFd, message, strlen(message));
		sleep(5);
    }
}

