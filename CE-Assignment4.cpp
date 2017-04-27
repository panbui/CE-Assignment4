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

	int xROI;   // define the top-left corner of our
	int yROI;   //     region-of-interest
	int widthROI;
	int heightROI;
	bool firstTime;

public:
	MotionTracker() {
		xROI = 150;   // define the top-left corner of our
		yROI = 150;   //     region-of-interest
		widthROI = 100;
		heightROI = 100;
		firstTime = true;
	}

	void feedNewframe(Mat frame) {
		Mat diffPrevious;     // difference between the previous and current frame
		Mat grayDiffPrevious;

		int x, y;
		int weight, sumWeight, sumWeightX, sumWeightY;
		int xCenter, yCenter;

		if (firstTime) {
			frame.copyTo(previousFrame);
			frame.copyTo(previousPreviousFrame);
			firstTime = false;
		}
		previousFrame.copyTo( previousPreviousFrame );
		frame.copyTo(previousFrame);
		// get the diff between the current frame and the second frame before it
		absdiff( previousPreviousFrame, frame, diffPrevious);
		// convert the color differences into gray differences
		// now, each pixel is in the range 0..255
		cvtColor( diffPrevious, grayDiffPrevious, CV_BGR2GRAY);

		sumWeightX = sumWeightY = sumWeight = 0;  // set them all to zero
		for (y = yROI; y < yROI + heightROI; y++) { // visit pixels row-by-row
			// inside each row, visit pixels from left to right
			for (x = xROI; x < xROI + widthROI; x++) {
				// weight of the pixel  x,y
				weight = grayDiffPrevious.at<unsigned char>( y,x );
				sumWeight = sumWeight + weight;
				sumWeightX = sumWeightX + x * weight;
				sumWeightY += y * weight;
			}
		}
		if (sumWeight != 0) {
			xCenter = sumWeightX / sumWeight;
			yCenter = sumWeightY / sumWeight;
		}
		xROI = xCenter - widthROI / 2;   // make the ROI "follow" the center-of-mass
		yROI = yCenter - heightROI / 2;
		// let's keep the ROI inside the frame at all times
		if (xROI < 0) {   // the ROI is starting to go out on the left
			xROI = 0;  // keep it inside
		}
		if (yROI < 0) {  // the ROI is starting to go up out of the frame
			yROI = 0;   // keep it inside
		}
		if (xROI + widthROI >= frame.cols) {  // ROI starts to go out on the right
			xROI = frame.cols - widthROI; // keep it inside
		}
		if (yROI + heightROI >= frame.rows) { // ROI starts to go out at bottom
			yROI = frame.rows - heightROI; // keep it inside
		}

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
		rectangle( frame, Rect(xROI,yROI,widthROI,heightROI), color, 2);
	}
};


int main(  int argc, char** argv ) {

	MotionTracker mTrack1;
	mTrack1.setROI(10,20,200,200);

	MotionTracker mTrack2;
	mTrack2.setROI(300,200,150,150);

	Mat drawFrame;

	int frameCount;   // counts the frames that are read from the camera
	Mat frame;   // Mat is a 2-D "matrix" of numbers, containing our image data
	VideoCapture cap(0);   // live camera
	if (!cap.isOpened()) {  // check if we succeeded in opening the camera
		return -1;  // quit the program if we did not succeed
	}

	namedWindow("Raw Image",CV_WINDOW_NORMAL);  // create a window
	namedWindow("Draw Frame", CV_WINDOW_NORMAL);

	for (frameCount = 0; frameCount < 100000000; frameCount++) {
		if (frameCount % 100 == 0) {
			printf("frameCount = %d \n", frameCount);
		}

		cap >> frame;

		flip(frame, frame, 1);

		mTrack1.feedNewframe(frame);
		mTrack2.feedNewframe(frame);

		imshow("Raw Image", frame);  // display the frame in the window
		frame.copyTo( drawFrame );

		mTrack1.drawROI(drawFrame,Scalar(0,0,255));
		mTrack2.drawROI(drawFrame,Scalar(0,255,0));

		imshow("Draw Frame", drawFrame);

		if (waitKey(20) >= 0) {   // wait 20ms and check if a key was pressed
			break;   // if a key was pressed, exit the for loop
		}
	}

	printf("Final frameCount = %d \n", frameCount);
	return 0;
}
