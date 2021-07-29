/*

This sample project uses OpenCV with ZBar and ZXing libraries to detect QR codes in order to test
implementations and compare detection performance relative to the included QR detection provided by
OpenCV installations.

To test the different detection algorithms, in main() uncomment:

detectQR() - to see OpenCV detection
zbar()     - to see ZBar detection
zxingQR()  - to see ZXing detection

*/


// OpenCV Library Headers
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

// ZBar Library Headers
#include "zbar.h"  

// ZXing Library Headers
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/MatSource.h>

#include <iostream>

using namespace cv;
using namespace zbar;
using namespace zxing;
using namespace zxing::qrcode;
using namespace std;


/* Display Image */
void showImage() {

	string path = "Resources/QRCode.png";
	Mat img = imread(path);
	imshow("Image", img);
	waitKey(0);

}

/* Play Video */
void showVideo() {
	string path = "Resources/DemoVideo.mp4";
	VideoCapture cap(path);
	Mat img;

	while (true) {
		cap.read(img);

		// Print framerate of capture
		double fps = cap.get(CAP_PROP_FPS);
		cout << "FPS: " + to_string(fps) << endl;

		imshow("Image", img);
		waitKey(1);
	}
	cap.release();

}

/* Display External Camera Feed*/
void showWebcam() {
	VideoCapture cap(0);
	Mat img;

	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
		return;
	}

	while (true) {
		cap.read(img);

		imshow("Image", img);
		waitKey(1);
	}
	cap.release();

}

/* Facial Detection through External Camera */
void detectFace() {
	CascadeClassifier faceCascade;
	faceCascade.load("Resources/haarcascade_frontalface_default.xml");

	if (faceCascade.empty()) {
		cout << "XML file not loaded" << endl;
	}

	VideoCapture cap(0);
	Mat img;

	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
		return;
	}


	while (true) {
		cap.read(img);

		vector<Rect> faces;
		faceCascade.detectMultiScale(img, faces, 1.1, 10);

		for (int i = 0; i < faces.size(); i++) {
			rectangle(img, faces[i].tl(), faces[i].br(), Scalar(255, 0, 255), 1);
		}

		imshow("Image", img);
		waitKey(1);
	}
	cap.release();

}

/* OpenCV native Qr code Detection through External Camera */
void detectQr() {
	// Open Webcam
	VideoCapture cap(0);

	// Check for webcam success
	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
		return;
	}

	// Create image
	Mat inputImage;

	// Qr code detector init
	cv::QRCodeDetector qrDecoder = cv::QRCodeDetector::QRCodeDetector();

	vector<Point> qrPoints;

	namedWindow("QRCode", WINDOW_NORMAL);

	// Set camera frame rate
	//cap.set(CAP_PROP_FPS, 60);

	while (true) {
		// Load Image from webcam
		cap.read(inputImage);

		string data = qrDecoder.detectAndDecode(inputImage, qrPoints);

		if (data.length() > 0)
		{
			cout << "Decoded Data : " << data << endl;

			// Create rectangle from upper left and bottom right output array points
			rectangle(inputImage, qrPoints[0], qrPoints[2], Scalar(255, 0, 255), 1);

			// Image conversion to only have QR show in feed
			//rectifiedImage.convertTo(rectifiedImage, CV_8UC3);

		}
		else {
			cout << "QR Code not detected" << endl;
		}

		imshow("QRCode", inputImage);
		if (waitKey(30) >= 0) {
			break;
		}
	}
	cap.release();

}


typedef struct
{
	string type;
	string data;
	vector <Point> location;

} decodedObject;

/* Find and decode barcodes and QR codes */
void decode(Mat& im, vector<decodedObject>& decodedObjects)
{
	// Create zbar scanner
	ImageScanner scanner;

	// Configure scanner to detect image types
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	// Convert image to grayscale
	Mat imGray;
	cvtColor(im, imGray, COLOR_BGR2GRAY);

	// Wrap opencv Mat image data in a zbar image
	Image image(im.cols, im.rows, "Y800", (uchar*)imGray.data, im.cols * im.rows);

	// Scan the image for barcodes and QRCodes
	int n = scanner.scan(image);

	if (n <= 0) {
		cout << "No QR detected" << endl;
		return;
	}

	// Print results
	for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
	{
		decodedObject obj;

		obj.type = symbol->get_type_name();
		obj.data = symbol->get_data();

		cout << to_string(symbol->get_location_size()) << endl;

		// Print type and data
		cout << "Type : " << obj.type << endl;
		cout << "Data : " << obj.data << endl << endl;

		// Obtain location
		for (int i = 0; i < symbol->get_location_size(); i++)
		{
			obj.location.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));

			cout << "X: " + to_string(symbol->get_location_x(i)) + " Y: " + to_string(symbol->get_location_y(i)) << endl;
		}

		decodedObjects.push_back(obj);
	}
}



/* Display barcode and QR code location */
void display(Mat& im, vector<decodedObject>& decodedObjects)
{
	// Loop over all decoded objects
	for (int i = 0; i < decodedObjects.size(); i++)
	{
		vector<Point> points = decodedObjects[i].location;
		vector<Point> hull;

		// If the points do not form a quad, find convex hull
		if (points.size() > 4) {
			convexHull(points, hull);
		}
		else {
			hull = points;
		}

		// Number of points in the convex hull
		int n = hull.size();

		for (int j = 0; j < n; j++)
		{
			line(im, hull[j], hull[(j + 1) % n], Scalar(255, 0, 0), 3);
		}
	}
}

/* ZBar QR Code Detection through External Camera */
void zbarQR() {

	// Initialize stream
	VideoCapture cap = VideoCapture();

	// Check for webcam success
	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
	}

	// Open Camera
	cap.open(0);

	// Check for webcam success
	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
		return;
	}

	// Set camera resolution
	//cap.set(CAP_PROP_FRAME_WIDTH, 1952);
	//cap.set(CAP_PROP_FRAME_HEIGHT, 1110);

	// Create image
	Mat inputImage;

	// Variable for decoded objects
	vector<decodedObject> decodedObjects;

	while (true) {
		// Load Image from webcam
		cap.read(inputImage);

		// Find and decode barcodes and QR codes
		decode(inputImage, decodedObjects);

		// Display location
		display(inputImage, decodedObjects);

		// Display results
		imshow("QRCode-Zbar", inputImage);

		waitKey(1);

		// Clear vector for drawing bb on next video frame
		decodedObjects.clear();
	}

	// Deallocate videocapturing
	cap.release();

}


Point toCvPoint(Ref<ResultPoint> resultPoint) {
	return Point(resultPoint->getX(), resultPoint->getY());
}

/* ZXing library QR Code Detection through External Camera */
void zxingQR() {

	// Initialize stream
	VideoCapture cap = VideoCapture();

	// Check for webcam success
	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
	}

	// Open Camera
	cap.open(0);

	// Check for webcam success
	if (!cap.isOpened()) {
		cout << "Webcam is not opened" << endl;
		return;
	}

	cap.set(CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);

	// Create image
	Mat inputImage, grey;

	// Stopped flag will be set to -1 from subsequent wayKey() if no key was pressed
	int stopped = -1;

	while (stopped == -1) {

		// Capture image
		bool result = cap.read(inputImage);

		if (result) {

			// Convert to grayscale
			cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);

			try {

				// Create luminance  source
				Ref<LuminanceSource> source = MatSource::create(grey);

				// Search for QR code
				Ref<Reader> reader;

				reader.reset(new QRCodeReader);

				Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));
				Ref<BinaryBitmap> bitmap(new BinaryBitmap(binarizer));
				Ref<Result> result(reader->decode(bitmap, DecodeHints(DecodeHints::TRYHARDER_HINT)));

				// Get result point count
				int resultPointCount = result->getResultPoints()->size();

				for (int j = 0; j < resultPointCount; j++) {

					// Draw circle
					circle(inputImage, toCvPoint(result->getResultPoints()[j]), 10, Scalar(110, 220, 0), 2);

				}

				// Draw boundary on image
				if (resultPointCount > 1) {

					for (int j = 0; j < resultPointCount; j++) {

						// Get start result point
						Ref<ResultPoint> previousResultPoint = (j > 0) ? result->getResultPoints()[j - 1] : result->getResultPoints()[resultPointCount - 1];

						// Draw line
						line(inputImage, toCvPoint(previousResultPoint), toCvPoint(result->getResultPoints()[j]), Scalar(110, 220, 0), 2, 8);

						// Update previous point
						previousResultPoint = result->getResultPoints()[j];

					}

				}

				if (resultPointCount > 0) {

					// Draw text
					putText(inputImage, result->getText()->getText(), toCvPoint(result->getResultPoints()[0]), LINE_4, 1, Scalar(110, 220, 0));

				}

			}
			catch (const ReaderException& e) {
				cerr << e.what() << " (ignoring)" << endl;
			}
			catch (const zxing::IllegalArgumentException& e) {
				cerr << e.what() << " (ignoring)" << endl;
			}
			catch (const zxing::Exception& e) {
				cerr << e.what() << " (ignoring)" << endl;
			}
			catch (const std::exception& e) {
				cerr << e.what() << " (ignoring)" << endl;
			}

			// Show captured image
			imshow("ZXing", inputImage);

			// Wait a key for 1 millis
			stopped = waitKey(1);

		}
		else {

			// Log
			cerr << "video capture failed" << endl;

		}
	}

	// Deallocate videocapturing
	cap.release();
}


int main() {
	//showImage();
	//showVideo();
	//showWebcam();
	//detectFace();
	//detectQr();
	//zbarQR();
	zxingQR();
	return EXIT_SUCCESS;
}