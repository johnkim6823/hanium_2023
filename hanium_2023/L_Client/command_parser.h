#include <stdio.h>
#include <stdlib.h>

#ifndef __CMDPARSERHEADER__
#define __CMDPARSERHEADER__

#include "client.h"
#include "client_cfg.h"
#include "command_define_list.h"
#include "command_function_list.cpp"

typedef struct _cmdpdesp
{
	u_int8_t code;
	int(* callback)(HEADERPACKET* msg, IO_PORT *port);
}cmdp_desp;

int cmd_parser(IO_PORT port, HEADERPACKET *pmsg);

/* command fucntions*/
int public_key_send(HEADERPACKET* msg, IO_PORT *port);			//PUBKET_SND
int public_key_response(HEADERPACKET* msg, IO_PORT *port);		//PUBKEY_RES
int video_data_send(HEADERPACKET* msg, IO_PORT *port);			//VIDEO_DATA_SND
int video_data_response(HEADERPACKET* msg, IO_PORT *port);		//VIDEO_DATA_RES
int verify_request(HEADERPACKET* msg, IO_PORT *port);			//VER_REQ

int still_alive(HEADERPACKET* msg, IO_PORT *port);				//STILL_ALIVE
int hi_i_am(HEADERPACKET* msg, IO_PORT *port);					//HI_I_M
int nice_to_meet_you(HEADERPACKET* msg, IO_PORT *port);		//NICE_2_MEET_U

int test(HEADERPACKET* msg, IO_PORT *port);					//TEST_CMD
#endif
