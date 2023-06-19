#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "command_parser.h"
#include "tracex.h"

using namespace std;

cmdp_desp parser_desp[] =
{
	{PUBKEY_SND					, public_key_send		},
	{PUBKEY_RES					, public_key_response	},
	{VIDEO_DATA_SND				, video_data_send		},
	{VIDEO_DATA_RES				, video_data_response	},
	{VER_REQ					, verify_request		},
	
	{STILL_ALIVE				, still_alive			},
	{HI_I_M						, hi_i_am				},
	{NICE_2_MEET_U				, nice_to_meet_you		},


	// Testing command
	{TEST_CMD					, test					},
};

//-----------------------------------------------------------------------------
//	cmd_parser
//-----------------------------------------------------------------------------
#ifndef _CMD_PARSER_
#define _CMD_PARSER_
int cmd_parser(IO_PORT port, HEADERPACKET *pmsg)
{
	int ack, i=0, len;
	len = sizeof(parser_desp) / sizeof(parser_desp[0]);

	// cout << "---------------------------------" << endl;
	// cout << "port : " << port.s << endl;
	// cout << hex << (int)pmsg->startID << endl;
	// cout << (int)pmsg->destID << endl;
	// cout << (int)pmsg->command << endl;
	// cout << (int)pmsg->dataType << endl;
	// cout << dec << (int)pmsg->dataSize << endl;
	// cout << "---------------------------------" << endl;

	if(pmsg->destID != ThisID){
		cout << port.s << " port has problem, Wrong destination!! " << endl;
		return -1; //"HEADERPACKET' destID != ThisID";
	}

	for (i = 0, ack = -3; i < len; i++){
		if(parser_desp[i].code == pmsg->command){
			ack = parser_desp[i].callback(pmsg, &port);
		}
	}
	if(ack == -3) {
		cout << "Somethings Wrong... callback function doesn't work X(";
		return -1;
	}
	else if(ack == -1){
		cout << " doesn't work;" << endl;
		return 0;
	}
	return 0;
}
#endif



