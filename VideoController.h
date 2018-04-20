#ifndef _VIDEOCONTROLLER_H_
#define  _VIDEOCONTROLLER_H_
#include "VideoController.h"
using namespace std;
using namespace cv;
/*********************************
 Interface class "Model" with a virtual funct. This func gets called for processing individual 
 frames. User required to sub-class from this interface class that defines Virtual func 
 "process(Mat &input, Mat &output)". Users can apply different algorithm on individual video frames.
**********************/
class Model {
public:
virtual void process(Mat &input, Mat &output)= 0;
};

class VideoController {
	VideoCapture vidreader; //define opencv video capture object, vidreader.
	VideoWriter writer; //define video writer object
	Model * modelPtr; //Should have arguments list signature, i.e.(const Mat& src, Mat& dest)
	int fps; //define a property to hold the input video rate in frame per sec.
	string outWindowName, outVidName; //saved only if outVidName specified. Same fps/size as input
	bool stop = false, isVidOpen, isWriterInitialized;
	vector<double> frameProcessingTime; // used for storing frameprocessing times(in ms

	bool isOutputVideoSaveReqd() { return (!outVidName.empty()); }
	void initWriter() {
		if (isOutputVideoSaveReqd()) {
			writer.release(); // release any previous instance of writer object
			int codec = static_cast<int>(vidreader.get(CAP_PROP_FOURCC));
			Size sz = Size(vidreader.get(CAP_PROP_FRAME_WIDTH),
				vidreader.get(CAP_PROP_FRAME_HEIGHT));
			writer.open(outVidName.c_str(), codec, fps, sz, true);
			if (!writer.isOpened()) {
				cout << " Error while calling the cv::VideoWriter.open(" << outVidName << ")" << endl;
				writer.release();
				CV_Assert(false);
			}
			isWriterInitialized = true;
		}
	}
public:
	VideoController() : modelPtr(nullptr), fps(0), outWindowName("Output"),
		isVidOpen(false), isWriterInitialized(false) {}
	~VideoController() { writer.release(); vidreader.release(); }
	
	void setOutVidName(string name) { outVidName = name; }
	void setOutWindowName(string name) { outWindowName = name; }
	void setAlgorithmModel(Model* m) { modelPtr = m; }
	void setInputVideo(string ipVideoName) {
		vidreader.release();
		vidreader.open(ipVideoName.c_str());
		if (!vidreader.isOpened()) {
			cout << " Invalid video file read " << endl;
			vidreader.release();
			CV_Assert(false);
		}
		isVidOpen = true;
		fps = vidreader.get(CAP_PROP_FPS);
		cout << "fps: " << fps << "\n";
	}
	
	void run() {
		CV_Assert(isVidOpen); // assert if the video is not opened.
		Mat currentFrame, outputFrame; // define Mat for currentFrame and outFrame
		namedWindow(outWindowName);
		initWriter(); //initialize videowriter object; this will set/unset 'isWriterInitialized'
		while (!stop) { // read each frame in video	
			if (!vidreader.read(currentFrame)) // read next frame
				break;
			double initialTime = (double)getTickCount();
			modelPtr->process(currentFrame, outputFrame); // call the function pointer
			double frameProcessTime = ((double(getTickCount()) - initialTime) / getTickFrequency()) * 1000;
			frameProcessingTime.push_back(frameProcessTime);
			imshow(outWindowName, outputFrame);
			if (isWriterInitialized) //write only if the writer is initialized
			writer.write(outputFrame);
			//get elasped time in ms since the video frame read
			double elaspedTime = ((double(getTickCount()) - initialTime) / getTickFrequency()) * 1000;
			// find the remaining delay as the framew processing+imshow+write took elasped time
			double remainingTime = (1000/fps) - (elaspedTime);
			if (remainingTime > 1) // if positive wait for the remaining time
				waitKey(remainingTime);
				//waitKey();
			else
				waitKey(1); // delay for 1 ms if elaspedtime>delay
				//waitKey();
		}
		Scalar m = mean(Mat(frameProcessingTime));
		cout << endl << " mean frame processing time " << sum(sum(m)) << endl;
		writer.release();
		vidreader.release();
	}
};
#endif