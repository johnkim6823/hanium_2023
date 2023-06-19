#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <fstream>

#include "logger_cfg.h"
#include "Logger_function_list.h"
#include "sign.cpp"
#include "../Merkle_Tree/merkle_tree.h"
#include "../L_Client/client.cpp"
#include "../L_Client/command_define_list.h"
#include "../msg_queue/msg_queue.cpp"

using namespace std;
using namespace cv;

int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;
int fps = DEFAULT_FPS;

cv::VideoCapture cap;
cv::Mat frame(cv::Size(width, height), CV_8UC3);
string getCID();

pthread_mutex_t frameLocker;
pthread_t UpdThread;

queue<cv::Mat> bgr_queue;            // for original frame(BGR)Mat queue
queue<cv::Mat> yuv420_queue;         // for original frame(yuv)Mat queue
queue<cv::Mat> y_queue;              // for y_frame Mat queue
queue<cv::Mat> feature_vector_queue; // for edge detection Canny
queue<string> hash_queue;            // for hash made by feature vector
queue<string> hash_signed_queue;
queue<string> cid_queue; // for CID for images

int key_generation()
{
    cout << "----Key Geneartion----" << endl;
    RSA *privateRSA = genPrivateRSA();
    publicKey = genPubicRSA(privateRSA);

    cout << "PRIKEY and PUBKEY are made" << endl;
    cout << "public Key = " << endl
         << publicKey;
}

int send_pubKey_to_server()
{

    cout << "----SEND PUBKEY to SERVER----" << endl;
    int pubKey_bufsize = publicKey.size();
    std::cout << "pubKey_bufsize: " << pubKey_bufsize << std::endl;

    char *pubKey_buffer = new char[pubKey_bufsize];
    strcpy(pubKey_buffer, publicKey.c_str());

    makePacket(Server, PUBKEY_SND, 0xa0, strlen(pubKey_buffer));
    void *p_packet = &sendDataPacket;

    if (!send_binary(&g_pNetwork->port, CMD_HDR_SIZE, p_packet))
    {
        cout << "PubKey send Error!!" << endl;
    }

    if (!send_binary(&g_pNetwork->port, strlen(pubKey_buffer), (void *)pubKey_buffer))
    {
        cout << "PubKey send Error!!" << endl;
    }
    cout << "----SENDING PUBKEY to SERVER END----" << endl;
}

int init()
{
    cout << "----Initalizing---------" << endl
         << endl;

    // open the default camera using default API
    int deviceID = 0;         // 0 = open default camera
    int apiID = cv::CAP_V4L2; // use V4L2
    // open selected camera using selected API
    cap.open(deviceID, apiID);

    camera_cfg_recv(width, height, fps);

    cap.set(cv::CAP_PROP_FRAME_WIDTH, width );
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS, fps);

    cout << "    Frame Width: " << cvRound(cap.get(CAP_PROP_FRAME_WIDTH)) << endl;
    cout << "    Frame Height: " << cvRound(cap.get(CAP_PROP_FRAME_HEIGHT)) << endl;
    cout << "    FPS : " << cvRound(cap.get(CAP_PROP_FPS)) << endl;

    cv::Mat img(cv::Size(width, height), CV_8UC3, Scalar(0));
    frame = img.clone();
    img.release();

    //--- If Cap is opened
    if (!cap.isOpened())
    {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    else
    {
        cout << "----Initalized----------" << endl;
        return 0;
    }
}

void init_all_settings()
{

    init_queue();

    cout << endl
         << "----Initializing all settings." << endl
         << endl;
    cout << "    bgr queue size: " << bgr_queue.size() << endl;
    cout << "    yuv420 queue size: " << yuv420_queue.size() << endl;
    cout << "    y_frame queue size: " << y_queue.size() << endl;
    cout << "    feature vector queue size: " << feature_vector_queue.size() << endl;
    cout << "    hash queue size: " << hash_queue.size() << endl;
    cout << "    CID queue size: " << cid_queue.size() << endl;
    cout << endl
         << "--------------------------------" << endl
         << endl;
}

void init_queue()
{

    while (!yuv420_queue.empty())
        yuv420_queue.pop();
    while (!bgr_queue.empty())
        bgr_queue.pop();
    while (!y_queue.empty())
        y_queue.pop();
    while (!feature_vector_queue.empty())
        feature_vector_queue.pop();
    while (!hash_queue.empty())
        hash_queue.pop();
    while (!hash_signed_queue.empty())
        hash_signed_queue.pop();
    while (!cid_queue.empty())
        cid_queue.pop();
}

void *UpdateFrame(void *arg)
{
    while (true)
    {

        cv::Mat tempFrame(cv::Size(width, height), CV_8UC3);
        cap >> tempFrame;

        pthread_mutex_lock(&frameLocker);
        frame = tempFrame.clone();
        pthread_mutex_unlock(&frameLocker);
    }
    pthread_exit((void *)0);
}

void lamping_time() {
    Mat temp;
    for(int i = 0; i < 10; i++){
        cap >> temp;
    }
    temp.release();
    cout << "lamping time end." << endl;
}
void capture()
{
    cout << endl
         << "----Starting Capturing" << endl
         << endl;

    pthread_mutex_init(&frameLocker, NULL);
    pthread_create(&UpdThread, NULL, UpdateFrame, NULL);

    while (true)
    {
        cv::Mat currentFrame(cv::Size(height, width), CV_8UC3, Scalar(0));

        pthread_mutex_lock(&frameLocker);
        currentFrame = frame;
        pthread_mutex_unlock(&frameLocker);

        if (currentFrame.empty())
        {
            cout << "Frame is empty" << endl;
        }

        else
        {
            bgr_queue.push(currentFrame);
            // Make CID for FRAMES
            string s_cid = getCID();
            cid_queue.push(s_cid);
        }


        if (bgr_queue.size() == DEFAULT_FRAME_COUNT)
        {

            int ret = pthread_cancel(UpdThread);
            int status;

            if (ret == 0)
            {
                // auto END
                ret = pthread_join(UpdThread, (void **)&status);
                if (ret == 0)
                {
                    // AUTO END = STATUS= -1
                    cout << "Capture End Successfully." << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                }
                else
                {
                    cout << "Thread End Error!" << ret << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                }
                break;
            }
        }

        // if ESC is pressed, then force thread to end
        if (cv::waitKey(20) == 27)
        {

            int ret = pthread_cancel(UpdThread);
            int status;

            if (ret == 0)
            {
                // auto END
                ret = pthread_join(UpdThread, (void **)&status);
                if (ret == 0)
                {
                    // AUTO END = STATUS= -1
                    cout << "Capture End Successfully." << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                }
                else
                {
                    cout << "Thread End Error!" << ret << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                }
                break;
            }
        }
    }
}


void convert_frames(queue<cv::Mat> &BGR_QUEUE)
{

    cout << endl
         << "----Start to convert Frames into YUV420 and Y----" << endl
         << endl;
    queue<cv::Mat> BGR_queue(BGR_QUEUE);

    while (true)
    {

        if (BGR_queue.size() == 0)
        {
            break;
        }

        cv::Mat original = BGR_queue.front();
        cv::Mat yuv_frame(cv::Size((height * 3 / 2), width), CV_8UC1, Scalar(0));
        cv::Mat y_frame(cv::Size(height, width), CV_8UC1, Scalar(0));
        BGR_queue.pop();

        // CONVERT BGR To YUV420 and YUV420 to Y
        cv::cvtColor(original, yuv_frame, cv::COLOR_BGR2YUV_I420);
        cv::cvtColor(yuv_frame, y_frame, cv::COLOR_YUV2GRAY_I420);

        // save frames into queue
        yuv420_queue.push(yuv_frame);
        y_queue.push(y_frame);

        // release Mat
        original.release();
        yuv_frame.release();
        y_frame.release();
    }

    cout << "    YUV420 amd Y frame are saved" << endl;
    cout << "    YUV420 frame: " << yuv420_queue.size() << "    Y frame: " << y_queue.size() << endl
         << endl;
    cout << "----FRAMES CONVERTED---------" << endl
         << endl;
}

void edge_detection(queue<cv::Mat> &Y_QUEUE)
{
    queue<cv::Mat> Y_queue(Y_QUEUE);

    cout << "----Building feature vectors." << endl;
    int cnt = 0;

    while (true)
    {
        if (Y_queue.size() == 0)
        {
            break;
        }
        cv::Mat temp;

        // Canny(img, threshold1, threshold2)
        // threshold1 = Determining whether an edge is in the adjacency with another edge
        // threshold2 = Determine if it is an edge or not
        cv::Canny(Y_queue.front(), temp, 20, 40);

        feature_vector_queue.push(temp);
        Y_queue.pop();
        cnt++;
        temp.release();
    }
    cout << endl
         << "    Edge Detection made: " << feature_vector_queue.size() << endl;
}

void make_hash(queue<cv::Mat> &FV_QUEUE)
{

    queue<cv::Mat> Feature_Vector_queue(FV_QUEUE);
    cout << endl
         << "----Make HASH from feature vectors." << endl
         << endl;

    int mat_width = Feature_Vector_queue.front().cols;
    int mat_height = Feature_Vector_queue.front().rows;
    int mat_channels = Feature_Vector_queue.front().channels();
    int umat_data_bufsize = mat_width * mat_height * mat_channels;

    while (true)
    {
        if (Feature_Vector_queue.size() == 0)
        {
            break;
        }
        cv::Mat temp = Feature_Vector_queue.front();
        Feature_Vector_queue.pop();

        string mat_data = "";
        string sha_result = "";

        // unsigned char *umat_data = new unsigned char[umat_data_bufsize];
        // memcpy(umat_data, temp.data, umat_data_bufsize);

        for (int i = 0; i < temp.rows; i++)
        {
            for (int j = 0; j < temp.cols; j++)
            {
                mat_data += to_string(temp.at<uchar>(i, j));
            }
        }

        sha_result = hash_sha256(mat_data);
        hash_queue.push(sha_result);
        temp.release();
    }
    cout << "    hash made : " << hash_queue.size() << endl;
}

void sign_hash(queue<string> &HASH_QUEUE)
{
    queue<string> sign(HASH_QUEUE);

    cout << "----Signing Hash by private Key" << endl
         << endl;

    while (true)
    {
        if (sign.size() == 0)
        {
            break;
        }
        string signed_hash = signMessage(privateKey, sign.front());

        char *ch = new char[350];
        strcpy(ch, signed_hash.c_str());

        hash_signed_queue.push(signed_hash);
        sign.pop();
    }
    cout << "    Signed Hash made: " << hash_signed_queue.size() << endl;
}

// string getCID()
// {
//     struct timeb tb; // <sys/timeb.h>
//     struct tm tstruct;
//     std::ostringstream oss;

//     string s_CID;
//     char buf[128];

//     ftime(&tb);
//     // For Thread safe, use localtime_r
//     if (nullptr != localtime_r(&tb.time, &tstruct))
//     {
//         strftime(buf, sizeof(buf), "%Y-%m-%d_%T.", &tstruct);
//         oss << buf;        // YEAR-MM-DD HH-mm_SS
//         oss << tb.millitm; // millisecond
//     }

//     s_CID = oss.str();

//     s_CID = s_CID.substr(0, 23);
//     if (s_CID.length() == 22)
//     {
//         s_CID = s_CID.append("0");
//     }
//     if (s_CID.length() == 21)
//     {
//         s_CID = s_CID.append("00");
//     }

//     return s_CID;
// }

void send_image_hash_to_UI(queue<cv::Mat> &ORI, queue<cv::Mat> &Y)
{
    cout << "----SEND BGR, Y frame and hash to WEB----" << endl;
    cv::Mat ori(Size(width, height), CV_8UC3);
    cv::Mat y(Size(width, height), CV_8UC3);

    ORI.front().copyTo(ori);
    Y.front().copyTo(y);

    cv::imwrite(orifile_path, ori);
    cv::imwrite(yfile_path, y);
    string hash = hash_queue.front();

    fstream hash_file("hash.txt", ios::app);
    if(hash_file.is_open()){
        hash_file << hash << endl;
    }

    hash_file.close();

    Image_Hash_request();

    ori.release();
    y.release();
}

void send_data_to_server(queue<string> &CID_QUEUE, queue<string> &HASH_QUEUE, queue<string> &SIGNED_HASH_QUEUE, queue<cv::Mat> &YUV420_QUEUE)
{
    cout << endl
         << "----SEND DATA to SERVER" << endl;

    queue<string> cid_send(CID_QUEUE);
    queue<cv::Mat> yuv_send(YUV420_QUEUE);
    queue<string> hash_send(HASH_QUEUE);
    queue<string> signed_hash_send(SIGNED_HASH_QUEUE);

    int total_data_size = 0;
    int cid_bufsize = cid_send.front().capacity();
    int hash_bufsize = hash_send.front().capacity();
    int signed_hash_bufsize = signed_hash_send.front().capacity();

    int video_rows = yuv_send.front().rows;
    int video_cols = yuv_send.front().cols;
    int video_channels = yuv_send.front().channels();
    int video_bufsize = video_rows * video_cols * video_channels;

    total_data_size += cid_bufsize;
    total_data_size += hash_bufsize;
    total_data_size += signed_hash_bufsize;
    total_data_size += video_bufsize;

    cout << "total data size : " << total_data_size << endl;
    cout << "size of CID data: " << cid_bufsize << endl;
    cout << "size of hash data: " << hash_bufsize << endl;
    cout << "size of signed_hash data: " << signed_hash_bufsize << endl;
    cout << "video size: " << yuv_send.front().size() << endl;
    cout << "size of video data: " << video_bufsize << endl;
    cout << endl
         << "---------------------- " << endl;

    int step = 0;
    while (true)
    {
        if (cid_send.size() == 0 && hash_send.size() == 0 && signed_hash_send.size() == 0 && yuv_send.size() == 0)
        {
            break;
        }
        cout << "step : " << ++step << endl;

        char *cid_buffer = new char[cid_bufsize];
        char *hash_buffer = new char[hash_bufsize];
        char *signed_hash_buffer = new char[signed_hash_bufsize];
        unsigned char *video_buffer = new unsigned char[video_bufsize];

        strcpy(cid_buffer, cid_send.front().c_str());
        strcpy(hash_buffer, hash_send.front().c_str());
        strcpy(signed_hash_buffer, signed_hash_send.front().c_str());
        memcpy(video_buffer, yuv_send.front().data, video_bufsize);

        makePacket(Server, VIDEO_DATA_SND, 0xa1, total_data_size);

        // cout << hex << (int)sendDataPacket.startID << endl;
        // cout << (int)sendDataPacket.destID << endl;
        // cout << (int)sendDataPacket.command << endl;
        // cout << (int)sendDataPacket.dataType << endl;
        // cout << dec << (int)sendDataPacket.dataSize << endl;
        // cout << endl << "----------------------------------------------------------" << endl << endl;
        // cout << "video rows: " << video_rows << endl << "video cols: " << video_cols << endl;
        // cout << "size: " << (strlen((char*)video_buffer)) * sizeof(unsigned char) << endl;
        // cout << "hash: " << hash_buffer << "size: " << strlen(hash_buffer) * sizeof(char) << endl;
        // cout << "CID: " << cid_buffer << endl << "size: " << strlen(cid_buffer) * sizeof(char) << endl;
        // cout << endl << "----------------------------------------------------------" << endl << endl;

        void *p_packet = &sendDataPacket;

        if (!send_binary(&g_pNetwork->port, sizeof(HEADERPACKET), (void **)p_packet))
        {
            cout << "Packet send Error!!" << endl;
            break;
        }

        if (!send_binary(&g_pNetwork->port, cid_bufsize, (void *)cid_buffer))
        {
            cout << "CID send Error!!" << endl;
        }

        if (!send_binary(&g_pNetwork->port, hash_bufsize, (void *)hash_buffer))
        {
            cout << "hash send Error!!" << endl;
        }

        if (!send_binary(&g_pNetwork->port, signed_hash_bufsize, (void *)signed_hash_buffer))
        {
            cout << "signed_hash send Error!!" << endl;
        }

        if (!send_binary(&g_pNetwork->port, video_bufsize, (void *)video_buffer))
        {
            cout << "Image send Error!!" << endl;
        }

        if (ClientServiceThread((void *)&g_pNetwork->port) == -1)
        {
            cout << "ClientServerThread return -1!!" << endl;
            exit(0);
        }

        yuv_send.pop();
        hash_send.pop();
        signed_hash_send.pop();
        cid_send.pop();
        sleep(0.2);
    }

    cout << "----SEND END----------------" << endl;
}

//TEST!!!!---------------------------------------------------------

void convolution_extraction(queue<cv::Mat> &Y_QUEUE) {
    queue<cv::Mat> Y_queue(Y_QUEUE);

    cout << "----Building feature vectors." << endl;
    cv::Mat kernel = cv::Mat_<float>({ 3, 3 }, { 0, -1, 0, -1, 5, -1, 0, -1, 0 }); // CHANGE KERNEL INPUT 
	cout << "--------------------------------------" << endl;
	cout << "KERNEL:\n " << kernel << endl;
	cout << "--------------------------------------" << endl;

    while (true)
    {
        if (Y_queue.size() == 0)
        {
            break;
        }
        cv::Mat input = Y_queue.front().clone();
        input.convertTo(input, CV_32F);		// NEED to CONVERT CV_8U to CV32F for operation
        cv::Mat output;
	    cv::Mat zeroPadding;

	    // ZeroPadding
	    cout << "Input Data size: " << input.size() << endl;
	    cout << "--------------------------------------" << endl;
	    CreateZeroPadding(input, zeroPadding, input.size(), kernel.size(), cv::Size(1, 1));
	    cout << "ZERO PADDING RESULT" << endl;
	    cout << "SIZE: " << zeroPadding.size() << endl;
	    cout << "TYPE: " << zeroPadding.type() << endl;
	    cout << "CHANNELS: " << zeroPadding.channels() << endl;
	    cout << "--------------------------------------" << endl;

	    int Kernel_halfWidth = (kernel.rows - 1) / 2;
	    output = zeroPadding.clone();

	    // Convolution
	    Convolution(zeroPadding, output, input.size(), kernel, cv::Size(1, 1));
        output.convertTo(output, CV_8U);
	    cout << "Convolution result: " << output.size() << endl;
	    cout << " CONVOLUTION RESULT" << endl;
	    cout << "SIZE: " << output.size() << endl;
	    cout << "TYPE: " << output.type() << endl;
	    cout << "CHANNELS: " << output.channels() << endl;
	    cout << "--------------------------------------" << endl;
        
        feature_vector_queue.push(output);
        Y_queue.pop();

        input.release();
        output.release();
        zeroPadding.release();
    }
    cout << endl
         << "    Edge Detection made: " << feature_vector_queue.size() << endl;
}

void CreateZeroPadding(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::Size& k, const cv::Size& stride)
{
	cv::Mat input = _Input.getMat();
	_Input.copyTo(_Output);

	//패딩 맞추기 자동화(합성곱 출력 크기 계산 https://excelsior-cjh.tistory.com/79)
	double p = 0;
	int oH = (int)((input.rows + 2 * p - k.height) / stride.height) + 1;
	int oW = (int)((input.cols + 2 * p - k.width) / stride.width) + 1;
	//제로 패딩 행렬 생성
	//패딩이 0.5 늘어날 때마다 왼쪽 + 위, 오른쪽 + 아래 순으로 행렬 확장
	while (oH != outputSize.height) {
		p += 0.5;
		if (p - (int)p != 0)
			cv::copyMakeBorder(_Output, _Output, 1, 0, 0, 0, cv::BORDER_CONSTANT, 0);
		else
			cv::copyMakeBorder(_Output, _Output, 0, 1, 0, 0, cv::BORDER_CONSTANT, 0);

		oH = (int)((input.rows + 2 * p - k.height) / stride.height) + 1;
	}
	p = 0;
	while (oW != outputSize.width) {
		p += 0.5;
		if (p - (int)p != 0)
			cv::copyMakeBorder(_Output, _Output, 0, 0, 1, 0, cv::BORDER_CONSTANT, 0);
		else
			cv::copyMakeBorder(_Output, _Output, 0, 0, 0, 1, cv::BORDER_CONSTANT, 0);

		oW = (int)((input.cols + 2 * p - k.width) / stride.width) + 1;
	}
}

void Convolution(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::InputArray& k, const cv::Size& stride)
{
	Mat kernel = k.getMat();
	Mat zeroPaddingMat = _Input.getMat();
	//입력 행렬이 제로 패딩 되어있지 않을 경우 제로 패딩 함수를 호출
	if ((zeroPaddingMat.rows - kernel.rows) / stride.height + 1 != outputSize.height
		|| (zeroPaddingMat.cols - kernel.cols) / stride.width + 1 != outputSize.width) {
		CreateZeroPadding(zeroPaddingMat, zeroPaddingMat, zeroPaddingMat.size(), kernel.size(), stride);
	}

	_Output.create(outputSize, _Input.type());
	_Output.setTo(0);
	Mat output = _Output.getMat();

	//제로 패딩 행렬과 커널로 교차 상관 연산 후, 연산 결과를 _Output 행렬에 저장
	for (int y = 0; y < output.rows; y++) {
		for (int x = 0; x < output.cols; x++) {
			for (int ky = 0; ky < kernel.rows; ky++) {
				for (int kx = 0; kx < kernel.cols; kx++) {
					output.at<float>(y, x) += kernel.at<float>(ky, kx) * zeroPaddingMat.at<float>(y * stride.height + ky, x * stride.width + kx);
				}
			}
		}
	}
}


//------------------------------------------------------------------

int main(int, char **)
{
    
    // key GEN
    key_generation();

    // Init Client
    if (!initClient())
    {
        cout << "init client error!!" << endl;
        return -1;
    }

    send_pubKey_to_server();
    
    while (true)
    {
        if (init() == -1)
        {
            break;
        }

        else
        {
            
            lamping_time();
            // capture frames
            capture();
            // show_frames(bgr_queue);

            // convert frames to YUV420 and Y
            convert_frames(bgr_queue);

            // USE Canny Edge Detection with Y_Frames
            //edge_detection(y_queue);
            convolution_extraction(y_queue);

            // make Hash by edge_detected datas
            make_hash(feature_vector_queue);
            sign_hash(hash_queue);

            // Send Data to WEB UI
            send_image_hash_to_UI(bgr_queue, y_queue);

            // send Datas to Server
            send_data_to_server(cid_queue, hash_queue, hash_signed_queue, yuv420_queue);
            // initialize all settings
            init_all_settings();

        }
    }
}
