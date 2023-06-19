using namespace std;

int key_generation();                                                                                                                     // make privatKey and PublicKey
int send_pubKey_to_server();                                                                                                              // Send PublicKey to Server
int init();                                                                                                                               // Init Camera Setting and OPEN CAP
void init_all_settings();                                                                                                                 // Init all settings at the end
void init_queue();                                                                                                                        // Init all datas in queues
void lamping_time();                                                                                                                      // lamping time
void *UpdateFrame(void *arg);                                                                                                             // Update Frames
void capture();                                                                                                                           // Capture frames;
void convert_frames(queue<cv::Mat> &BGR_QUEUE);                                                                                           // Convert saved frames;
void edge_detection(queue<cv::Mat> &Y_QUEUE);                                                                                             // Edge detact by y frames
void make_hash(queue<cv::Mat> &FV_QUEUE);                                                                                                 // make hash using feature vector
void sign_hash(queue<string> &HASH_QUEUE);                                                                                                // Sign hash using Private Key
string getCID();                                                                                                                          // Make CID for each frames
void send_image_hash_to_UI(queue<cv::Mat> &ORI, queue<cv::Mat> &Y);                                                                       // SEND BGR_Image,png, Y_Image.png PATH and Hash code
void send_data_to_server(queue<string> &CID_QUEUE, queue<string> &HASH_QUEUE, queue<string> &SIGNED_HASH_QUEUE, queue<cv::Mat> &YUV420_QUEUE); // send datas to Server
/*Test*/
void convolution_extraction(queue<cv::Mat> &Y_QUEUE);                                                                                     // Feature Extraction by convolution operation
void CreateZeroPadding(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::Size& k, const cv::Size& stride);  //zero Padding function
void Convolution(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::InputArray& k, const cv::Size& stride);  //Convolution function
//-------------
