#ifndef __SERVER_CFG__
#define __SERVER_CFG__
//CHANGE SERVER_IP_ADDR to RPI's IP that working as SERVER
#define SERVER_PORT        8700
#define storage_dir        "/home/pi/images/"

// Deafault values for datasize
#define Hash_size          64
#define Signed_Hash_size   350
#define CID_size           23

// Communication protocol
#define CMD_HDR_SIZE        8
#define ASYNC_BUFSIZE       4096
#define MAX_USER_CNT        5

#define ThisID Server

#endif
