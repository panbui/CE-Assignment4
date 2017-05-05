
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

			// Specify the region to crop image
		Rect ROI;
		ROI.x = xROI;
		ROI.y = yROI - 50;	// -50 to ensure the vehicle shape is within the image
		ROI.width = widthROI;
		ROI.height = heightROI;

			// For displaying counting
		string laneNo = "Lane " + intToString(lane);

			// Condition to count and display objects
		if (below == true && mean > threshold) {
			objectCount ++;
			below = false;

				// Save frame as a jpg file
		    sprintf(fileNameImage, "%d-%03d.jpg", lane, objectCount);
		    imwrite(fileNameImage, frame);

		    	// Crop image within the region above
		    image = imread(fileNameImage, 1);
		    crop = image(ROI);

		    	// Replace the frame image by the cropped image
		    sprintf(fileNameImage, "%d-%03d.jpg", lane, objectCount);
		    imwrite(fileNameImage, crop);

		    	// Show image in separate window
			namedWindow(laneNo, CV_WINDOW_NORMAL);
			imshow(laneNo, crop);

			file = fopen("objects.html", "a");
			fprintf(file, "<img scr = %s>", fileNameImage);
			fclose(file);
		}
		if (below == false && mean < threshold) {
			below = true;
		}
	}

		// Function allows users to customise the ROI
	void setROI(int new_xROI, int new_yROI, int new_widthROI, int new_heightROI) {
		xROI = new_xROI;
		yROI = new_yROI;
		widthROI = new_widthROI;
		heightROI = new_heightROI;
	}

		// Function to draw the ROI on the window
	void drawROI(Mat frame, Scalar color) {
		rectangle( frame, Rect(xROI,yROI,widthROI,heightROI), color, 2);
	}

		// Graph the signal
	void graph(Mat graph, int frameCount, int bottom, int top, int thresline) {
			// Clear the graph with color black
		line(graph, Point(frameCount % 300, bottom), Point(frameCount % 300, 768), Scalar(0,0,0), 2);
			// Plot the signal graph with position input from the user
		line(graph, Point(frameCount % 300, bottom), Point(frameCount % 300, top), Scalar(0,255,0), 2);
			// Red threshold line
		circle(graph, Point(frameCount % 300, thresline), 1, Scalar(0,0,255),3);
	}

		// Display counting numbers as text on screen
	void count(Mat frame, int lane, int x, int y) {
			// Example: "Lane 1: 10"
		string status = "Lane " + intToString(lane) + ": " + intToString(objectCount);
		putText(frame, status, Point(x,y), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,255), 2, 6);
	}

};


int main(  int argc, char** argv ) {
		// Declare 3 ROIs for 3 lanes from the video
	MotionTracker lane1;
	lane1.setROI(110,350,100,70);
	MotionTracker lane2;
	lane2.setROI(240,350,80,70);
	MotionTracker lane3;
	lane3.setROI(360,350,90,70);

		// Declare necessary Mat objects
	Mat drawFrame;   					// The main frame with video and customised objects
	Mat frame;
	Mat graph = Mat(768, 300, CV_8UC3);	// 768 x 300 Mat object for graph
										// The signal value for each ROI is within [0..255], and we need to graph for 3 ROIs,
										// so 256 x 3 = 768, hence the height of the Mat

	int frameCount;   // counts the frames that are read from the camera

	char video[50] = "road.mp4";

	VideoCapture cap(video);   	// Open video
	if (!cap.isOpened()) {  	// Check if we succeeded in opening the video
		return -1;  			// quit the program if we did not succeed
	}

		// Open and initialise the HTML file
	FILE *fileImg;
	fileImg = fopen("objects.html", "w");
	fprintf(fileImg, "<html>\n <body> \n");
	fclose(fileImg);

		// Create 2 windows for main video and graph
	namedWindow("Graph", CV_WINDOW_NORMAL);
	namedWindow("Main Video", CV_WINDOW_NORMAL);

	for (frameCount = 0; frameCount < 100000000; frameCount++) {
			// Inform the user every 100 frames
		if (frameCount % 100 == 0) {
			printf("frameCount = %d \n", frameCount);
		}

			// Capture frame to Mat object
		cap >> frame;

			// Count objects passing through 3 ROIsin each frame, and print on console the counting number
		lane1.feedNewframe(frame, frameCount);
		lane1.countObject(frame, 1, fileImg);
		printf("Lane 1 = %d \n", lane1.objectCount);

		lane2.feedNewframe(frame, frameCount);
		lane2.countObject(frame, 2, fileImg);
		printf("Lane 2 = %d \n", lane2.objectCount);

		lane3.feedNewframe(frame, frameCount);
		lane3.countObject(frame, 3, fileImg);
		printf("Lane 3 = %d \n", lane3.objectCount);

			// Create a "drawing" frame
		frame.copyTo(drawFrame);

			// Draw ROI for each lane
		lane1.drawROI(drawFrame,Scalar(0,0,255));
		lane2.drawROI(drawFrame,Scalar(0,0,255));
		lane3.drawROI(drawFrame,Scalar(0,0,255));

			// Graph signal for each ROI
		lane1.graph(graph, frameCount, 0, lane1.mean, lane1.threshold);
		lane2.graph(graph, frameCount, 256, lane2.mean + 256, lane2.threshold + 256);
		lane3.graph(graph, frameCount, 512, lane3.mean + 512, lane3.threshold + 512);

			// Display counting number on screen for each ROI
		lane1.count(drawFrame, 1, 10, 60);
		lane2.count(drawFrame, 2, 230, 60);
		lane3.count(drawFrame, 3, 460, 60);

			// Flip the graph upside down
		flip(graph, graph, 0);

			// Show in windows
		imshow("Main Video", drawFrame);
		imshow("Graph", graph);

			// Flip the graph upside down again
		flip(graph, graph, 0);

		if (waitKey(20) >= 0) {   // wait 20ms and check if a key was pressed
			break;   // if a key was pressed, exit the for loop
		}
	}

	printf("Final frameCount = %d \n", frameCount);
	return 0;
}
