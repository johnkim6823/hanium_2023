using namespace std;
// Logger <--> Web UI                                       //Logger <----> WEbUI
void camera_cfg_res_send();                                 //Logger(SND)  -> Web UI(RECV)      B
void camera_cfg_recv(int &WIDTH, int &HEIGHT, int &FPS);    //Logger(RECV) <- Web UI(SND)       BBI
void Image_Hash_request();                                  //Logger(RECV) <- Web UI(REQ)       B
void Image_Hash_send();                                     //Logger(SND)  -> Web UI(RECV)      ss
int Image_Hash_response();                                  //Logger(RECV) <- Web UI(SND))      B
