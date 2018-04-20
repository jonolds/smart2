#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void merge();
void avSlope(deque<Vec4i> v, Point& leftAve, Point& rightAve);

Scalar colorGreen = Scalar(0, 255, 0);
Scalar colorRed = Scalar(0, 0, 255);
Scalar colorBlue = Scalar(255, 0, 0);
Scalar colorYellow = Scalar(0, 255, 255);
Scalar colorOrange = Scalar(0, 165, 255);
deque<Vec4i> slopeOfBridge;


void process(Mat& src, Mat& out) {
	Mat img;
	out = src.clone();
	cvtColor(src, img, COLOR_BGR2GRAY);
	Mat im_edge(img.size(), CV_8U, Scalar::all(0));
	Canny(img, im_edge, 100, 300, 3);

	// Rect(x,y,w,h) w->width=cols;h->rows
	Mat roi = im_edge(Rect(0, im_edge.rows * 0.6, im_edge.cols, im_edge.rows - im_edge.rows * 0.6));

	vector<Vec4i> lines;
	// 1st best {30, 20, 20} > {30,10,20}>{40, 20, 10} 
	HoughLinesP(roi, lines, 1, 1 * CV_PI / 180, 30, 20, 20);
	/* the detected hough lines are for the roi image
	hence need to add the offset y to the detected lines for displaying in the full image */
	int y_offset = im_edge.rows * 0.6;
	double  maxXl = 0, maxYl = img.size().height, maxXr = 0, maxYr = img.size().height;
//	cout << img.size().width << " " << img.size().height << " " << "init max height: " << maxY << "\n";
	for (size_t i = 0; i < lines.size(); i++) {
		bool skipFlagSlope = false;
		lines[i][1] += y_offset;
		lines[i][3] += y_offset;
		// calculated slope is with reference to origin at botton left, aka mathematical orignal
		float x1 = lines[i][0], y1 = lines[i][1], x2 = lines[i][2], y2 = lines[i][3];
		float slope = (y2 - y1) / (x2 - x1);
		slope = tan(slope) * 180 / CV_PI;
		if (slope > 360)
			slope -= 360;
		if (slope < -360)
			slope += 360;
		//lane lines are close to +/- 45 degree; horizontal lane lines have slope~0
		if (abs(abs(slope) - 45) > 15.0)
			skipFlagSlope = true;
		//do not draw lines if any of the flags are set
		if (!skipFlagSlope) {
			if(slope > 0)
				line(out, Point(x1, y1), Point(x2, y2), colorRed, 2, 8);
			else
				line(out, Point(x1, y1), Point(x2, y2), colorBlue, 2, 8);
			if(slope < 0) {
				if(y1 < maxYl || y2 < maxYl) {
					maxYl = (y1 > y2) ? y2 : y1;
					maxXl = (y1 > y2) ? x2 : x1;
				}
			}
			else {
				if(y1 < maxYr || y2 < maxYr) {
					maxYr = (y1 > y2) ? y2 : y1;
					maxXr = (y1 > y2) ? x2 : x1;
				}
			}
		}
	}
	//line(out, Point(0, maxYl), Point(img.size().width, maxYl), colorBlue, 2, LINE_8, 0);
	//line(out, Point(0, maxYr), Point(img.size().width, maxYr), colorRed, 2, LINE_8, 0);
	line(out, Point(maxXl, maxYl), Point(maxXr, maxYr), colorYellow, 2, LINE_8, 0);
	slopeOfBridge.push_front(Vec4i(maxXl, maxYl, maxXr, maxYr));
	if (slopeOfBridge.size() > 8) {
		slopeOfBridge.pop_back();
		Point leftAve, rightAve;
		avSlope(slopeOfBridge, leftAve, rightAve);
		line(out, leftAve, rightAve, colorOrange, 2, LINE_8, 0);
	}
		
}

void avSlope(deque<Vec4i> v, Point& leftAve, Point& rightAve) {
	double xLave = 0, yLave = 0, xRave = 0, yRave = 0;
	int num = 8;
	for(int i = 0; i < 8; i++) {
		xLave += v[i][0];
		yLave += v[i][1];
		xRave += v[i][2];
		yRave += v[i][3];
	}
	leftAve = Point(xLave / num, yLave/num);
	rightAve = Point(xRave / num, yRave / num);
}

int main() {
	Mat src = imread("input.png");
	Mat out(src.size(), src.type());
	process(src, out);
	namedWindow("output", WINDOW_AUTOSIZE);
	imshow("output", out);
	waitKey();
	return 0;
}

void addText(Mat &tmp, double slope, vector<Vec4i> lines, int i) {
	int y_offset2 = -5, x_offset2 = slope > 0 ? -250 : 60, fontFace = CV_FONT_HERSHEY_DUPLEX, thickness = 3;
	double fontScale = 1;
	int baseline = 0;

	string text = "slope = " + to_string(slope);
	Point org(30, 30);
	Size textSz = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	rectangle(tmp, org + Point(0, 5), Point(30 + textSz.width, 25 - textSz.height), CV_RGB(0, 0, 0), CV_FILLED);
	putText(tmp, "slope = " + to_string(slope), Point(30, 30), fontFace, fontScale, colorGreen, thickness, LINE_8, false);

	text = "1(" + to_string(lines[i][0]) + ", " + to_string(lines[i][1]) + ")";
	org = Point(lines[i][0] + x_offset2, lines[i][1] + y_offset2);
	textSz = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	rectangle(tmp, org + Point(0, 5), Point(org.x + textSz.width, org.y - textSz.height), CV_RGB(0, 0, 0), CV_FILLED);
	putText(tmp, text, org, fontFace, fontScale, colorGreen, thickness, LINE_8, false);


	text = "2(" + to_string(lines[i][2]) + ", " + to_string(lines[i][3]) + ")";
	org = Point(lines[i][2] + x_offset2, lines[i][3] + y_offset2);
	textSz = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	rectangle(tmp, org + Point(0, 5), Point(org.x + textSz.width, org.y - textSz.height), CV_RGB(0, 0, 0), CV_FILLED);
	putText(tmp, text, org, fontFace, fontScale, colorGreen, thickness, LINE_8, false);
}

void merge() {
	Mat dst;
	for (int i = 1; i < 13; i += 3) {
		Mat row, tmp, img1, img2, img3;
		tmp = imread(to_string(i) + ".jpg");
		resize(tmp, img1, Size(), .4, .4, INTER_CUBIC);
		tmp = imread(to_string(i + 1) + ".jpg");
		resize(tmp, img2, Size(), .4, .4, INTER_CUBIC);
		tmp = imread(to_string(i + 2) + ".jpg");
		resize(tmp, img3, Size(), .4, .4, INTER_CUBIC);
		hconcat(img1, img2, row);
		hconcat(row, img3, row);
		if (!dst.data)
			dst = row;
		else
			vconcat(dst, row, dst);
	}
	imwrite("finalLanes.jpg", dst);
	namedWindow("lanes", WINDOW_AUTOSIZE);
	imshow("lanes", dst);
	waitKey();
}