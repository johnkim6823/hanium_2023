using namespace std;

int read_pubKey();
void init();
void init_queue();
int get_data_from_DB(string &CID, queue<string> &CID_QUEUE, queue<string> &HASH_DB_QUEUE);	          //Get Datas from SERVER
void read_video_data(string &CID , queue<string> &CID_QUEUE);
void convert_frames(queue<cv::Mat> &YUV420_QUEUE);	  //Convert Datas
void edge_detection(queue<cv::Mat> &Y_QUEUE);         //Edge detact by y frames
void make_hash(queue<cv::Mat> &FV_QUEUE);             //make hash using feature vector
int make_merkle_tree(queue<string> &HASH_DB_QUEUE, queue<string> &HASH_VERIFIER_QUEUE);
//Temporary
void show_hash(queue<string> &DB_HASH, queue<string> &VF_HASH);
void show_frames(queue<cv::Mat> &ORI);