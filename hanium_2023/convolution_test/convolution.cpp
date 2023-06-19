#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

/*--- functions ---*/
void CreateZeroPadding(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::Size& k, const cv::Size& stride);	//zero Padding function
void Convolution(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& outputSize, const cv::InputArray& k, const cv::Size& stride);	//Convolution function
void MaxPooling(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& poolSize, const cv::Size& stride);								//Pooling function. Not USING
/*--- functions ---*/

int main(int argc, char** argv ){
	cv::Mat src, dst;
	const char* window_name = "filter2D Demo";
	const char* imageName = argc >= 2 ? argv[1] : "lena.jpg";				

	// Loads an image
	src = imread(samples::findFile(imageName), IMREAD_COLOR); // Load an image

	if (src.empty())
	{
		printf(" Error opening image\n");
		printf(" Program Arguments: [image_name -- default lena.jpg] \n");
		return EXIT_FAILURE;
	}

	//resize(src, src, Size(640, 480));		 // DONT NEED
	cvtColor(src, src, COLOR_BGR2YUV_I420);	 // CHANGE BGR -> YUV420
	cvtColor(src, src, COLOR_YUV2GRAY_I420); // CHANGE YUV420 -> Y


	cv::Mat input = src.clone();
	input.convertTo(input, CV_32F);		// NEED to CONVERT CV_8U to CV32F for operation

	//cv::Mat kernel = cv::Mat_<float>({ 3, 3 }, { -1, -1, -1, -1, -8, -1, -1, -1,-1 }); // CHANGE KERNEL INPUT 
	cv::Mat kernel = cv::Mat_<float>({ 3, 3 }, { 0, -1, 0, -1, 5, -1, 0, -1, 0 }); // CHANGE KERNEL INPUT 
	cout << "--------------------------------------" << endl;
	cout << "KERNEL:\n " << kernel << endl;
	cout << "--------------------------------------" << endl;
	cv::Mat out;
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
	out = zeroPadding.clone();


	// Convolution
	Convolution(zeroPadding, out, input.size(), kernel, cv::Size(1, 1));
	cout << "Convolution result: " << out.size() << endl;
	CreateZeroPadding(input, zeroPadding, input.size(), kernel.size(), cv::Size(1, 1));
	cout << " CONVOLUTION RESULT" << endl;
	cout << "SIZE: " << out.size() << endl;
	cout << "TYPE: " << out.type() << endl;
	cout << "CHANNELS: " << out.channels() << endl;
	cout << "--------------------------------------" << endl;

	/*
	//Max Pooling
	MaxPooling(out, out, cv::Size(2, 2), cv::Size(2, 2));
	cout << "풀링 연산 후 :" << out.size() << std::endl;
	*/

	out.convertTo(out, CV_8U);			//Need to Change out's type to 8U to SHOW out as Y frame

	while (true) {
		char c = (char)waitKey(500);
		imshow("CONVOLUTIONED", out);
		// Press 'ESC' to exit the program
		if (c == 27)
		{
			break;
		}

	}

	return 0;
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

void MaxPooling(cv::InputArray _Input, cv::OutputArray _Output, const cv::Size& poolSize, const cv::Size& stride)
{
	cv::Mat zeroPaddingMat = _Input.getMat();

	//제로 패딩 행렬이 유효하지 않은지 검사(풀링 성립 유무)
	float outputHeight = (float)(zeroPaddingMat.rows - poolSize.height) / stride.height + 1;
	float outputWidth = (float)(zeroPaddingMat.cols - poolSize.width) / stride.width + 1;
	std::cout << "풀링 함수 안에서 input size :\n" << outputWidth << std::endl << outputHeight << std::endl;

	if (outputHeight != (int)outputHeight
		|| outputWidth != (int)outputWidth) {
		std::cout << "풀링 연산 불가하므로 제로 패딩 추가" << std::endl;
		outputHeight = cvRound(outputHeight);
		outputWidth = cvRound(outputWidth);
		CreateZeroPadding(zeroPaddingMat, zeroPaddingMat, cv::Size(outputWidth, outputHeight), poolSize, stride);
	}
	_Output.create(cv::Size(outputWidth, outputHeight), zeroPaddingMat.type());
	_Output.setTo(0);
	cv::Mat output = _Output.getMat();

	//std::cout << "풀링 연산 전 input :\n" << zeroPaddingMat << std::endl;
	//std::cout << "풀링 연산 전 output :\n" << output << std::endl;
	//std::cout << output.at<float>(0, 0) << std::endl;
	//제로 패딩 행렬과 커널로 교차 상관 연산 후, 연산 결과를 output 행렬에 저장
	for (int y = 0; y < output.rows; y++) {
		for (int x = 0; x < output.cols; x++) {
			float maxValue = 0;
			for (int ky = 0; ky < poolSize.height; ky++) {
				for (int kx = 0; kx < poolSize.width; kx++) {
					if (maxValue < zeroPaddingMat.at<float>(y * stride.height + ky, x * stride.width + kx))
						maxValue = zeroPaddingMat.at<float>(y * stride.height + ky, x * stride.width + kx);
				}
			}
			//std::cout << maxValue << std::endl;
			output.at<float>(y, x) = maxValue;
		}
	}
}
