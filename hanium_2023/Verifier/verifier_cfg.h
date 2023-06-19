#ifndef __VERIFIER_CFG__
#define __VERIFIER_CFG__
// Default DB, storage Settings
// #define DB_IP "127.0.0.1"
// #define DB_user "hanium" // user = webui -> Can extern access
// #define DB_password "1234"
// #define DB_database "hanium"
// #define storage_dir "/home/pi/images/"
//----------------------------------
#define Hash_size 64
#define Signed_Hash_size 350
//----------------------------------
// Default values for Camera Settings
#define VGA_SIZE 460800
#define CIF_SIZE 152064
#define HD_SIZE  1382400

#define YUV420_VGA_WIDTH 640
#define YUV420_VGA_HEIGHT 480 * 3/2
#define YUV420_CIF_WIDTH 352
#define YUV420_CIF_HEIGHT 288 * 3/2
#define YUV420_HD_WIDTH  1280
#define YUV420_HD_HEIGHT 720 * 3/2

#define VGA_WIDTH 640
#define VGA_HEIGHT 480
#define CIF_WIDTH 352
#define CIF_HEIGHT 288
#define HD_WIDTH 1280
#define HD_HEIGHT 720
//----------------------------------
// Deafault values PATH for Verifier
#define pubKey_path "/home/pi/hanium_2022/Server/PUBKEY.txt"
#define video_data_path "/home/pi/images/"
//----------------------------------
#define ThisID Verifier

#endif
