#include <cv.h>   // all the OpenCV headers, which links to all the libraries
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace cv;   // a "shortcut" for directly using OpenCV functions

class MotionTracker {
public:  // private members are accessible/modifiable only inside this class
	// only the functions inside MotionTracker can access/modify the values
	// of these variables
	Mat previousPreviousFrame;   // the previous previous frame
	Mat previousFrame;    // the previous frame
	Mat currentFrame;
	Mat referenceFrame;

	int xROI;   // define the top-left corner of our
	int yROI;   //     region-of-interest
	int widthROI;
	int heightROI;
	int sum;
	int mean;
	int threshold;
	int objectCount;
	bool firstTime;
	bool below;

	MotionTracker() {   // our constructor;   an "initialization"
		xROI = 150;   // define the top-left corner of our
		yROI = 250;   //     region-of-interest
		sum = 0;
		mean = 0;
		threshold = 55;
		objectCount = 0;
		widthROI = 100;
		heightROI = 100;
		firstTime = true;
		below = true;
	}

	void feedNewframe(Mat frame) {
		Mat diffPrevious;     // difference between the previous and current frame
		Mat diffReference;
		Mat grayDiffReference;
		Mat grayDiffPrevious;

		int x, y;

		if (firstTime) {
			frame.copyTo(referenceFrame);
			frame.copyTo(currentFrame);
			frame.copyTo(previousFrame);
			frame.copyTo(previousPreviousFrame);
			firstTime = false;
		}

		previousFrame.copyTo(previousPreviousFrame);
		currentFrame.copyTo(previousFrame);
		frame.copyTo(currentFrame);

		absdiff(referenceFrame, frame, diffReference);
		// get the diff between the current frame and the second frame before it
		absdiff(previousPreviousFrame, frame, diffPrevious);
		// convert the color differences into gray differences
		// now, each pixel is in the range 0..255
		cvtColor(diffReference, grayDiffReference, CV_BGR2GRAY);
		cvtColor(diffPrevious, grayDiffPrevious, CV_BGR2GRAY);

		for (y = yROI; y < yROI + heightROI; y++) { // visit pixels row-by-row
			// inside each row, visit pixels from left to right
			for (x = xROI; x < xROI + widthROI; x++) {
				// weight of the pixel  x,y
				sum += grayDiffReference.at<unsigned char>(y,x);
			}
		}

		mean = sum / (widthROI * heightROI);
	}

	void countObject() {
		if (below == true && mean > threshold) {
			objectCount ++;
			below = false;
		}
		if (below == false && mean < threshold) {
			below = true;
		}
	}

	// setROI() is a mutator or "setter" function that is used
	//     to modify the values of private attributes
	// This function is like a "guardian" of private information
	void setROI(int new_xROI, int new_yROI, int new_widthROI, int new_heightROI) {
		xROI = new_xROI;
		yROI = new_yROI;
		widthROI = new_widthROI;
		heightROI = new_heightROI;
	}

	// These functions are called accessor or "getter" functions.
	// They are used to fetch the values of private attributes
	// Again, these functions are like "guardians" that allow
	//   outside code to read private information.
	int get_xCenter() {   return xROI + widthROI / 2;   }
	int get_yCenter() {   return yROI + heightROI / 2;   }
	int get_widthROI() {  return widthROI;  }
	int get_heightROI() {  return heightROI;  }

	void drawROI(Mat frame, Scalar color) {
		rectangle( frame, Rect(xROI,yROI,widthROI,heightROI), color, 2);
	}

};


int main(  int argc, char** argv ) {

	MotionTracker tracker;  // our MotionTracker object is mTrack1
	tracker.setROI(100,200,150,150 );   // set mTrack1's region-of-interest

	Mat drawFrame;   // where we visualize
	Mat frame;   // Mat is a 2-D "matrix" of numbers, containing our image data
	Mat graph = Mat(256, 300, CV_8UC3);

	int frameCount;   // counts the frames that are read from the camera

	VideoCapture cap(0);   // live camera
	if (!cap.isOpened()) {  // check if we succeeded in opening the camera
		return -1;  // quit the program if we did not succeed
	}

	namedWindow("Raw Image",CV_WINDOW_NORMAL);  // create a window
	namedWindow("Draw Frame", CV_WINDOW_NORMAL);
	namedWindow("Graph", CV_WINDOW_NORMAL);

	for (frameCount = 0; frameCount < 100000000; frameCount++) {
		if (frameCount % 100 == 0) {  // every 100 frames, print a message
			printf("frameCount = %d \n", frameCount);
		}

		cap >> frame;  // from the first camera
		flip(frame,frame,1);  // flip the frame horizontally

		tracker.feedNewframe(frame);
		tracker.countObject();
		printf("Object = %d \n", tracker.objectCount);

		frame.copyTo(drawFrame);  // create our "drawing" frame

		tracker.drawROI(drawFrame,Scalar(0,0,255));  // draw mTrack1's ROI

		line(graph, Point(frameCount % 300, 0), Point(frameCount % 300, 255), Scalar(0,0,0), 2);
		line(graph, Point(frameCount % 300, 0), Point(frameCount % 300, tracker.mean), Scalar(0,255,0), 2);

		flip(graph, graph, 0);

		imshow("Graph", graph);
		imshow("Raw Image", frame);  // display the frame in the window
		imshow("Draw Frame", drawFrame);

		flip(graph, graph, 0);


		if (waitKey(20) >= 0) {   // wait 20ms and check if a key was pressed
			break;   // if a key was pressed, exit the for loop
		}
	}

	printf("Final frameCount = %d \n", frameCount);
	return 0;
}
