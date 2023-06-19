#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <resolv.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <thread>

#define THIS_IS_CLIENT

#include "client.h"
#include "tracex.h"
#include "command_define_list.h"

#include <iostream>

#define CMD_BACKGROUND 1


using namespace std;

string getCID() {
    struct timeb tb;   // <sys/timeb.h>                       
    struct tm tstruct;                      
    std::ostringstream oss;   
    
    string s_CID;                             
    char buf[128];                                            
                                                              
    ftime(&tb);
    // For Thread safe, use localtime_r
    if (nullptr != localtime_r(&tb.time, &tstruct)) {         
        strftime(buf, sizeof(buf), "%Y-%m-%d_%T.", &tstruct);  
        oss << buf; // YEAR-MM-DD HH-mm_SS            
        oss << tb.millitm; // millisecond               
    }              

    s_CID = oss.str();
    
    s_CID = s_CID.substr(0,23);
    if(s_CID.length() == 22) {
        s_CID = s_CID.append("0");
    }
    if(s_CID.length() == 21) {
        s_CID = s_CID.append("00");
    }
    
    return s_CID;
}

void insert_port(int ID, int port){
	static mutex m;
	while(true){
		if(m.try_lock()){
			client_port_map.insert({port, ID});
			m.unlock();
			break;
		}
		else{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	while(true){
		if(m.try_lock()){
			map<int, int>::iterator iter;
			for(iter = client_port_map.begin(); iter != client_port_map.end(); ++iter){
				cout << "key : " << (*iter).first << ", value : " << (*iter).second << endl;
			}
			m.unlock();
			break;
		}
		else{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

void pop_port(int port){
	static mutex m;
	while(true){
		if(m.try_lock()){
			map<int, int>::iterator iter;
			for(iter = client_port_map.begin(); iter != client_port_map.end(); ++iter){
				if((*iter).second == port)
					client_port_map.erase((*iter).first);
			}
			m.unlock();
			break;
		}
		else{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

void closesocket(SOCKET sock_fd);

void makePacket(uint8_t destID, uint8_t cmd, uint8_t dataType, uint32_t dataSize)
{
	memset(&sendDataPacket, 0, sizeof(HEADERPACKET));
	sendDataPacket.startID = Logger; 
	sendDataPacket.destID = destID;
	sendDataPacket.command = cmd;
	sendDataPacket.dataType = dataType;
	sendDataPacket.dataSize = dataSize;
}

int __send( IO_PORT *p, void *pdata, int len )
{
	int res = 0;
	int i = 0;
	struct timeval tv;
	fd_set fdset, temps;

	if( p == NULL ) {
		return 0;
	}

	FD_ZERO( &fdset );
	FD_SET( p->s, &fdset );
	do {
		temps = fdset;

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		res = select( p->s+1, NULL, &temps, NULL, &tv );
		if( res > 0 ) {
			break;
		}
		TRACEF( "retry send(%d)\n", i );
	} while( ++i < 4 );

	if( res > 0 ) {
		res = send( p->s, pdata, len, 0 );
		if( res <= 0 ) {
			TRACE( "len(%d), timeout(%d) res(%d)", len, p->timeout, res );
		}
	}
	return res;
}

static int __recv( IO_PORT *p, HANDLE pdata, int len )
{
	int     res;
	struct  timeval tv;

	if( p->timeout > 0 ) {
		fd_set  reads;

		FD_ZERO( &reads );
		FD_SET( p->s, &reads );

		tv.tv_sec = p->timeout;
		tv.tv_usec = 0;

		res = select( p->s+1, &reads, NULL, NULL, &tv );
		if( res == -1 ) {
			TRACE_ERR( "Select error.\n" );
			return -1;
		}
		else if( res == 0 ) {
			//TRACEF( ">>>> Select Time Out...\n" );
			return -9;
		}
	}
	return recv( p->s, pdata, len, 0 );
}

int send_binary( IO_PORT *p, long nSize, void *pdata )
{
	int nSendBytes;

	if( p == NULL ) {
		TRACE_ERR("p == NULL\n");
		return FALSE;
	}

	//TRACEF("nSize = %d\n", nSize);
	do {
		nSendBytes = MIN( ASYNC_BUFSIZE, nSize );
		nSendBytes = __send( p, pdata, nSendBytes );
		if( nSendBytes <= 0 ) {
			TRACE_ERR( "return FALSE(%d)\n", nSendBytes );
			return FALSE;
		}

		pdata += nSendBytes;
		nSize -= nSendBytes;
		sleep(0.5);
	} while( nSize > 0 );

	return TRUE;
}

int recv_binary( IO_PORT *p, long size, void *pdata )
{
	int remainbytes, recvbytes;

	remainbytes = size;
	while(remainbytes > 0) {
		recvbytes = __recv( p, pdata, MIN( ASYNC_BUFSIZE, remainbytes ) );
		if( recvbytes <= 0 ) {
			TRACE_ERR( "ERR recv_binary :%d\n", recvbytes );
			return FALSE;
		}
		pdata += recvbytes;
		remainbytes -= recvbytes;
		//TRACEC(D_RED, "remainbytes = %d, recvBytes = %d\n", remainbytes, recvbytes);
	}
	return TRUE;
}

int ClientServiceThread(void *arg)
{
	IO_PORT *clientThd = (IO_PORT*) arg;
	uint8_t res;
	uint32_t fd_max;
	uint32_t fd_socket;
	uint32_t retry_cnt = 5;
	uint32_t send_retry_cnt = 5;
	struct  timeval tv;
	fd_set  reads;
	uint8_t buf[CMD_HDR_SIZE];
	uint8_t cmd[100]={0,};

	if(arg == NULL){
		TRACE_ERR( "arg == NULL\n" );
		return NULL;
	}

	TRACEF( "################## Start Connection Proecess ##################\n" );

	clientThd->timeout  = 30;
	fd_socket = clientThd->s;
	fd_max = fd_socket+1;

	while( clientThd->timeout > 0 ) {
		memset(buf, 0, CMD_HDR_SIZE);
		FD_ZERO( &reads );
		FD_SET( fd_socket, &reads );
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		//make HEADERPACKET

		// res = select( fd_max, &reads, NULL, NULL, &tv );
		// if( res == -1 ) {
		// 	TRACE_ERR( "connect socket(%d) Select error.\n", fd_socket);
		// 	usleep(10000);
		// 	continue;
		// }
		// else if( res == 0 ) {
		// 	TRACEF( "socket(%d) >>>> Select Time Out...\n", fd_socket);
		// 	goto SERVICE_DONE;
		// }
		while(retry_cnt >= 0) {
			res = recv( fd_socket, buf, CMD_HDR_SIZE, 0 );
			if(res <= 0 ) {
				if(retry_cnt <= 0){
					TRACE_ERR("connect socket(%d) Command receive error\n", fd_socket);
					goto SERVICE_DONE;
				}
				retry_cnt--;
			}
			else break;
		}
		retry_cnt = 5;
		
		if(cmd_parser(*clientThd, (HEADERPACKET *)buf) == -1) {
			cout << "cmd_parser return -1" << endl;
			TRACE_ERR("Data is sent with wrong destination : connected socket (%s)\n", fd_socket);
			return -1;
		}
		else{
			send_retry_cnt--;
			if(send_retry_cnt == 0) {
				TRACE_ERR("connect socket(%d) Command send error\n", fd_socket);
				goto SERVICE_DONE;
			}
			// if (buf[2] == CMD_BACKGROUND) {
			// 	send_retry_cnt = 5;
			// 	usleep(10000);
			// 	continue;
			// }
			return 1;
		}
		send_retry_cnt = 5;

		usleep(10000);
	}
SERVICE_DONE:
	termClient();
	return 1;
}

SOCKET create_socket()
{
	SOCKET s;
	int opt;

	if( ( s = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET ) {
		TRACE_ERR( "\tsocket error.\n" );
		return INVALID_SOCKET;
	}

	opt = 32*1024;
	setsockopt( s, SOL_SOCKET, SO_RCVBUF, &opt, sizeof( opt ) );
	opt = 32*1024;
	setsockopt( s, SOL_SOCKET, SO_SNDBUF, &opt, sizeof( opt ) );
	opt = 1;
	setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof( opt ) );

	return s;
}

int initClient()
{
	cout << "----Client Initializing----" << endl;


	g_pNetwork = (NETWORK_CONTEXT*) malloc(sizeof(NETWORK_CONTEXT));
	g_pNetwork->m_socket = create_socket();

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
	addr.sin_port = htons(SERVER_PORT);

	g_pNetwork->port.addr = addr;
	g_pNetwork->port.s = g_pNetwork->m_socket;

	if(connect(g_pNetwork->m_socket, (struct sockaddr *)&g_pNetwork->port.addr, sizeof(g_pNetwork->port.addr))){
		perror("client : connect");
		return -1;
	}

	cout << "----Initializing END----" << endl << endl;

	return TRUE;
}

void termClient()
{
	close(g_pNetwork->m_socket);
	free(g_pNetwork);
	cout << "terminate Client end" << endl;
}



