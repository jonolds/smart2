#include <opencv2/opencv.hpp>
#include<iostream>
#include "VideoController.h"
using namespace std;
using namespace cv;

/*******List of properties for reference ***************
VideoCapture videoreader; VideoWriter writer; Model *modelPtr; int fps; double delay;
string outputWindowName; string outputVideoName; bool stop; bool isVideoOpened;
bool isWriterInitialized; bool testFirstFrameOnly; vector<double> frameProcessingTime;
****************/


void VideoController::setInputVideo(string ipVideoName) {
	// It is important to release the any previous instances of class videoCapture 
	videoreader.release();
	videoreader.open(ipVideoName.c_str());
	// check if video is opened successfully
	if (!videoreader.isOpened()) {
		cout << " Invalid video file read " << endl;
		videoreader.release();
		CV_Assert(false);
	}
	// we are here only if the video is succesfully opened
	isVideoOpened = true;
	fps = videoreader.get(CV_CAP_PROP_FPS);
}

void VideoController::setOutputVideoName(string name) { outputVideoName = name; }
void VideoController::setOutputWindowName(string name) { outputWindowName = name; }
void VideoController::setAlgorithmModel(Model* m) { modelPtr = m; }
bool VideoController::isOutputVideoSaveReqd() { return (!outputVideoName.empty()); }

void VideoController::initWriter() {
	if (isOutputVideoSaveReqd()) {
		// release any previous instance of writer object
		writer.release();
		int codec = static_cast<int>(videoreader.get(CV_CAP_PROP_FOURCC));
		Size sz = Size(videoreader.get(CV_CAP_PROP_FRAME_WIDTH),
		               videoreader.get(CV_CAP_PROP_FRAME_HEIGHT));
		writer.open(outputVideoName.c_str(), codec, fps, sz, true);
		if (!writer.isOpened()) {
			cout << " Error while calling the cv::VideoWriter.open(" << outputVideoName << ")" << endl;
			writer.release();
			CV_Assert(false);
		}
		isWriterInitialized = true;
	}
}

void VideoController::run() {
	CV_Assert(isVideoOpened); // assert if the video is not opened.
	Mat currentFrame, outputFrame; // define Mat for currentFrame and outFrame
	namedWindow(outputWindowName);
	initWriter(); //initialize videowriter object; this will set/unset 'isWriterInitialized'
	int delay = 1000 / fps; // delay in ms
	while (!stop) { // read each frame in video
		// read next frame
		if (!videoreader.read(currentFrame))
			break;
		double initialTime = (double)getTickCount();
		// call the function pointer
		modelPtr->process(currentFrame, outputFrame);
		double frameProcessTime = ((double(getTickCount()) - initialTime) / getTickFrequency()) * 1000;
		frameProcessingTime.push_back(frameProcessTime);
		imshow(outputWindowName, outputFrame);
		if (isWriterInitialized) //write only if the writer is initialized
			writer.write(outputFrame);
		//get elasped time in ms since the video frame read
		double elaspedTime = ((double(getTickCount()) - initialTime) / getTickFrequency()) * 1000;
		// find the remaining delay as the framew processing+imshow+write took elasped time
		double remainingTime = delay - (elaspedTime);
		if (remainingTime) // if positive wait for the remaining time
			waitKey(remainingTime);
		else
			waitKey(1); // delay for 1 ms if elaspedtime>delay
	}
	Scalar m = mean(Mat(frameProcessingTime));
	cout << endl << " mean frame processing time " << sum(sum(m)) << endl;
	writer.release();
	videoreader.release();
}