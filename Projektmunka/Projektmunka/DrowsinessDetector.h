#pragma once
#include <opencv2/core/core.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>

#define EAR_THRESHOLD 0.18
#define EAR_CONSEC_FRAMES 2
#define EYE_CONS_FRAME 60 // ~2sec
#define FRAME_SKIPS 1
#define MIN_BLINKS_PER_MIN 10
#define MAX_BLINKS_PER_MIN 20
#define MAR_THRESHOLD 0.35
#define MAR_CONSEC_FRAME 3
#define SIZE_RATIO 1
#define PERCLOS_THRESHOLD 0.10
#define MAX_FRAMES 1800

using namespace cv;
using namespace dlib;
using namespace std;

class DrowsinessDetector
{
public:
	DrowsinessDetector();
	void start();
	void start(string file);
	void drawPoints(Mat& image, full_object_detection landmarks);
	float calcLeftEAR(full_object_detection landmarks);
	float calcRightEAR(full_object_detection landmarks);
	float calcEAR(full_object_detection landmarks);
	float calcMAR(full_object_detection landmarks);
	void displayStat(Mat& img, int blinks, float ear, float mar);
	void addNewWarning(string msg);
	void reset();
private:
	frontal_face_detector detector;
	shape_predictor predictor;
	unsigned int close_counter;
	unsigned int yawning_counter;
	unsigned int blinks;
	unsigned int total_frames;
	unsigned int closed_frames;
	float perclos;
	unsigned int numberOfWarnings;
	float EAR_Threshold;
};

