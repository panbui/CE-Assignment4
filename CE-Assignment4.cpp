
//============================================================================
// Name        : EEET2448 COMPUTING ENGINEERING 2017A - ASSIGNMENT 3
// Author      : - Bui Khac Phuong Uyen _ s3618720 : Image cropping and HTML
//				 - Tran Thi Hong Phuong _ s3623386 : Object counting
// Copyright   : Your copyright notice
// Description : This program counts the number of vehicles from a video. Each time a vehicle passes by, its image is displayed on a separate window.
//				Then all images are stored in an HTML file.
//
// During the process, we encountered some difficulties such as:
// 		- Cropping the frame under the condition so that the vehicle shape is fully (or mostly) within the image.
//		- Some difficulties with writing to HTML file in a function under a class.
//============================================================================

#include <cv.h>   // all the OpenCV headers, which links to all the libraries
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <string>
#include <sstream>

using namespace cv;   // a "shortcut" for directly using OpenCV functions
using namespace std;

	// A function to convert from interger to string data type
string intToString(int number) {
	ostringstream temp;
	temp << number;
	return temp.str();
}

	// Declaration of our main class
class MotionTracker {
public:
	Mat previousPreviousFrame;  // the previous previous frame
	Mat previousFrame;    		// the previous frame
	Mat currentFrame;			// the current frame
	Mat referenceFrame;			// the first frame of the video
	Mat diffReference;
	Mat crop;					// the image cropped from the frame

		// Region of Interest
	int xROI;
	int yROI;
	int widthROI;
	int heightROI;
		// Parameters for counting objects
	int mean;
	int sumMean;
	int averageMean;
	int threshold;
	int objectCount;
	bool firstTime;
	bool below;

		// Constructor - initialise all attributes of the class
	MotionTracker() {
		xROI = 0;
		yROI = 0;
		mean = 0;
		sumMean = 0;
		averageMean = 0;
		threshold = 0;
		objectCount = 0;
		widthROI = 100;
		heightROI = 100;
		firstTime = true;
		below = true;
	}

		// Process every frame
	void feedNewframe(Mat frame, int frameCount) {
		Mat diffPrevious;     // difference between the previous and current frame
		Mat grayDiffReference;
		Mat grayDiffPrevious;

		int x, y;
		int sum = 0;
		float thresholdFactor = 1.25;

			// The first capture is slightly different from the following capture, so we have a specific condition for it
		if (firstTime) {
			frame.copyTo(referenceFrame);
			frame.copyTo(currentFrame);
			frame.copyTo(previousFrame);
			frame.copyTo(previousPreviousFrame);
			firstTime = false;
		}

			// After the first frame, all the following frames are processed as usual
		previousFrame.copyTo(previousPreviousFrame);
		currentFrame.copyTo(previousFrame);
		frame.copyTo(currentFrame);

			// Get the diff between the current frame and the first frame
		absdiff(referenceFrame, frame, diffReference);

			// Convert the color differences into gray differences
		cvtColor(diffReference, grayDiffReference, CV_BGR2GRAY);

			// Visit pixels row-by-row, column-by-column to calculate the weight of each pixel
		for (y = yROI; y < yROI + heightROI; y++) {
			for (x = xROI; x < xROI + widthROI; x++) {
					// Weight of the pixel  x,y
				sum += grayDiffReference.at<unsigned char>(y,x);
			}
		}

			// Calculate the average weight of the ROI
		mean = sum / (widthROI * heightROI);
			// Modify the threshold so that it changes accordingly to the change of the video
		sumMean += mean;
		averageMean = sumMean / (frameCount + 1);
		threshold = (int) 5 + (averageMean * thresholdFactor);
	}

		// Function to count objects
	void countObject(Mat frame, int lane, FILE * file) {
		char fileNameImage[100];

		Mat image;

		Rect ROI;
		ROI.x = xROI;
		ROI.y = yROI - 50;
		ROI.width = widthROI;
		ROI.height = heightROI;

		string laneNo = "Lane " + intToString(lane);

		if (below == true && mean > threshold) {
			objectCount ++;

		    sprintf(fileNameImage, "%d-%03d.jpg", lane, objectCount);
		    imwrite(fileNameImage, frame);

		    image = imread(fileNameImage, 1);
		    crop = image(ROI);

		    sprintf(fileNameImage, "%d-%03d.jpg", lane, objectCount);
		    imwrite(fileNameImage, crop);

			namedWindow(laneNo, CV_WINDOW_NORMAL);
			imshow(laneNo, crop);

			file = fopen("objects.html", "a");
			fprintf(file, "<img scr = %s>", fileNameImage);
			fclose(file);

			below = false;
		}

		if (below == false && mean < threshold) {
			below = true;
		}
	}

	void setROI(int new_xROI, int new_yROI, int new_widthROI, int new_heightROI) {
		xROI = new_xROI;
		yROI = new_yROI;
		widthROI = new_widthROI;
		heightROI = new_heightROI;
	}

	void drawROI(Mat frame, Scalar color) {
		rectangle( frame, Rect(xROI,yROI,widthROI,heightROI), color, 2);
	}

	void graph(Mat graph, int frameCount, int bottom, int top, int thresline) {
		line(graph, Point(frameCount % 300, bottom), Point(frameCount % 300, 768), Scalar(0,0,0), 2);
		line(graph, Point(frameCount % 300, bottom), Point(frameCount % 300, top), Scalar(0,255,0), 2);
		circle(graph, Point(frameCount % 300, thresline), 1, Scalar(0,0,255),3);
	}

	void count(Mat frame, int lane, int x, int y) {
		string status = "Lane " + intToString(lane) + ": " + intToString(objectCount);
		putText(frame, status, Point(x,y), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,255), 2, 6);
	}

};


int main(  int argc, char** argv ) {

	MotionTracker lane1;
	lane1.setROI(110,350,100,70);

	MotionTracker lane2;  // our MotionTracker object is mTrack1
	lane2.setROI(240,350,80,70);   // set mTrack1's region-of-interest

	MotionTracker lane3;  // our MotionTracker object is mTrack1
	lane3.setROI(360,350,90,70);   // set mTrack1's region-of-interest

	Mat drawFrame;   // where we visualize
	Mat frame;   // Mat is a 2-D "matrix" of numbers, containing our image data
	Mat graph = Mat(768, 300, CV_8UC3);

	int frameCount;   // counts the frames that are read from the camera

	char video[50] = "road.mp4";

	VideoCapture cap(video);   // live camera
	if (!cap.isOpened()) {  // check if we succeeded in opening the camera
		return -1;  // quit the program if we did not succeed
	}

	FILE *fileImg;
	fileImg = fopen("objects.html", "w");
	fprintf(fileImg, "<html>\n <body> \n");
	fclose(fileImg);

	namedWindow("Graph", CV_WINDOW_NORMAL);
	namedWindow("Main Video", CV_WINDOW_NORMAL);

	for (frameCount = 0; frameCount < 100000000; frameCount++) {
		if (frameCount % 100 == 0) {  // every 100 frames, print a message
			printf("frameCount = %d \n", frameCount);
		}

		cap >> frame;  // from the first camera

		lane1.feedNewframe(frame, frameCount);
		lane1.countObject(frame, 1, fileImg);
		printf("Lane 1 = %d \n", lane1.objectCount);

		lane2.feedNewframe(frame, frameCount);
		lane2.countObject(frame, 2, fileImg);
		printf("Lane 2 = %d \n", lane2.objectCount);

		lane3.feedNewframe(frame, frameCount);
		lane3.countObject(frame, 3, fileImg);
		printf("Lane 3 = %d \n", lane3.objectCount);

		frame.copyTo(drawFrame);  // create our "drawing" frame

		lane1.drawROI(drawFrame,Scalar(0,0,255));  // draw mTrack1's ROI
		lane2.drawROI(drawFrame,Scalar(0,0,255));  // draw mTrack1's ROI
		lane3.drawROI(drawFrame,Scalar(0,0,255));  // draw mTrack1's ROI

		lane1.graph(graph, frameCount, 0, lane1.mean, lane1.threshold);
		lane2.graph(graph, frameCount, 256, lane2.mean + 256, lane2.threshold + 256);
		lane3.graph(graph, frameCount, 512, lane3.mean + 512, lane3.threshold + 512);

		lane1.count(drawFrame, 1, 10, 60);
		lane2.count(drawFrame, 2, 230, 60);
		lane3.count(drawFrame, 3, 460, 60);

		flip(graph, graph, 0);

		imshow("Main Video", drawFrame);
		imshow("Graph", graph);

		flip(graph, graph, 0);

		if (waitKey(20) >= 0) {   // wait 20ms and check if a key was pressed
			break;   // if a key was pressed, exit the for loop
		}
	}

	printf("Final frameCount = %d \n", frameCount);
	return 0;
}
