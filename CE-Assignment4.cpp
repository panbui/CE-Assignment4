#include <cv.h>   // all the OpenCV headers, which links to all the libraries
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace cv;   // a "shortcut" for directly using OpenCV functions

class MotionTracker {
private:
	Mat previousPreviousFrame;
	Mat previousFrame;    // the previous frame
	Mat referenceFrame;
	Mat passingObject;

	int xROI;   // define the top-left corner of our
	int yROI;   //     region-of-interest
	int widthROI;
	int heightROI;

	bool firstTime;

	int mean;
	int sum;
	int weight;
	float threshold;
	bool below;

public:
	MotionTracker() {
		xROI = 150;   // define the top-left corner of our
		yROI = 150;   //     region-of-interest
		widthROI = 100;
		heightROI = 100;
		firstTime = true;
		mean = sum = 0;
		threshold = 1.25;
		below = true;
		weight = 0;
	}

	bool objectPass(Mat frame) {
		Mat diffPrevious;     // difference between the previous and current frame
		Mat grayDiffPrevious;

		if (firstTime) {
			frame.copyTo(referenceFrame);
			frame.copyTo(previousFrame);
			frame.copyTo(previousPreviousFrame);
			firstTime = false;
		}

		previousFrame.copyTo(previousPreviousFrame);
		frame.copyTo(previousFrame);

		// get the diff between the current frame and the second frame before it
		absdiff(previousPreviousFrame, frame, diffPrevious);

		// convert the color differences into gray differences
		// now, each pixel is in the range 0..255
		cvtColor(diffPrevious, grayDiffPrevious, CV_BGR2GRAY);

		int x,y;

		for (y = yROI; y < yROI + heightROI; y++) { // visit pixels row-by-row
			// inside each row, visit pixels from left to right
			for (x = xROI; x < xROI + widthROI; x++) {
				// weight of the pixel  x,y
				weight = grayDiffPrevious.at<unsigned char>( y,x );
			}
		}

		mean = sum / (widthROI * heightROI);

		if (below == true && mean > threshold) {
			frame.copyTo(passingObject);
			below = false;
		}

		if (below == false && mean < threshold) {
			below = true;
		}

		return below;
	}

	void setROI(int new_xROI, int new_yROI, int new_widthROI, int new_heightROI) {
		xROI = new_xROI;
		yROI = new_yROI;
		widthROI = new_widthROI;
		heightROI = new_heightROI;
	}

	int get_xCenter() {   return xROI + widthROI / 2;   }
	int get_yCenter() {   return yROI + heightROI / 2;   }
	int get_widthROI() {  return widthROI;  }
	int get_heightROI() {  return heightROI;  }

	void drawROI(Mat frame, Scalar color) {
		rectangle(frame, Rect(xROI,yROI,widthROI,heightROI), color, 2);
	}
};


int main(  int argc, char** argv ) {
	Mat passingObject;
	Mat drawFrame;
	Mat frame;   // Mat is a 2-D "matrix" of numbers, containing our image data

	MotionTracker mTrack1;
	mTrack1.setROI(10,20,200,200);

	int frameCount;   // counts the frames that are read from the camera

	VideoCapture cap(0);   // live camera

	if (!cap.isOpened()) {  // check if we succeeded in opening the camera
		return -1;  // quit the program if we did not succeed
	}

	namedWindow("Raw Image",CV_WINDOW_NORMAL);  // create a window
	namedWindow("Draw Frame", CV_WINDOW_NORMAL);
	namedWindow("Passing Objects", CV_WINDOW_NORMAL);

	for (frameCount = 0; frameCount < 100000000; frameCount++) {
		if (frameCount % 100 == 0) {
			printf("frameCount = %d \n", frameCount);
		}

		cap >> frame;

		flip(frame, frame, 1);

		if (mTrack1.objectPass(frame)) {
			frame.copyTo(passingObject);
		}

		imshow("Raw Image", frame);  // display the frame in the window
		frame.copyTo(drawFrame);

		mTrack1.drawROI(drawFrame,Scalar(0,0,255));

		imshow("Draw Frame", drawFrame);
		imshow("Passing Objects", passingObject);

		if (waitKey(20) >= 0) {   // wait 20ms and check if a key was pressed
			break;   // if a key was pressed, exit the for loop
		}
	}

	printf("Final frameCount = %d \n", frameCount);
	return 0;
}
