#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>

#include "DrowsinessDetector.h"
#include "TextLog.h"

DrowsinessDetector::DrowsinessDetector() : close_counter(0), blinks(0), total_frames(0), closed_frames(0), numberOfWarnings(0), yawning_counter(0), EAR_Threshold(0.22)
{
	detector = get_frontal_face_detector();
	deserialize("shape_predictor_68_face_landmarks.dat") >> predictor;
}

void DrowsinessDetector::start()
{
	VideoCapture video(0);
	
	std::vector<dlib::rectangle> faces;

	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now();

	float EAR = 0.0;
	float MAR = 0.0;

	Mat frame;
	Mat gray;
	Mat resized;

	
	float sumEAR = 0;
	for (size_t i = 0; i < MAX_FRAMES; i++)
	{
		video.read(frame);

		int height = frame.rows * 0.4;
		int width = frame.cols * 0.4;

		resize(frame, resized, Size(width, height));
		cv::cvtColor(resized, gray, COLOR_BGR2GRAY);
		equalizeHist(gray, gray);

		cv_image<uchar> dlib_gray(gray);
		faces = detector(dlib_gray);
		for (int i = 0; i < faces.size(); i++)
		{
			full_object_detection landmarks = predictor(dlib_gray, faces[i]);
			EAR = calcEAR(landmarks);
			sumEAR += EAR;
		}
	}
	EAR_Threshold = (sumEAR / MAX_FRAMES) * 0.75;

	while (video.isOpened())
	{
		video.read(frame);
		if (total_frames++ % FRAME_SKIPS == 0)
		{
			int height = frame.rows * 0.4;
			int width = frame.cols * 0.4;

			resize(frame, resized, Size(width, height));
			cv::cvtColor(resized, gray, COLOR_BGR2GRAY);
			equalizeHist(gray, gray);

			cv_image<uchar> dlib_gray(gray);
			faces = detector(dlib_gray);

			for (int i = 0; i < faces.size(); i++)
			{
				full_object_detection landmarks = predictor(dlib_gray, faces[i]);;
				MAR = calcMAR(landmarks);
				EAR = calcEAR(landmarks);

				if (EAR > EAR_THRESHOLD)
				{
					close_counter = 0;
				}
				else
				{
					closed_frames++;
					if (close_counter == 0)
					{
						blinks++;
						close_counter++;
					}
					if (close_counter >= 130)
					{
						addNewWarning("Critical warning: You closed your eyes for too long.");
					}
				}

				displayStat(frame, blinks, EAR, MAR);
				drawPoints(frame, landmarks);
			}

			imshow("Frame", frame);
			end = chrono::steady_clock::now();

			if (chrono::duration_cast<chrono::seconds>(end - start).count() >= 60 || blinks > MAX_BLINKS_PER_MIN)
			{
				perclos = closed_frames / total_frames;
				if (blinks > MAX_BLINKS_PER_MIN || blinks < MIN_BLINKS_PER_MIN || perclos > PERCLOS_THRESHOLD)
				{
					addNewWarning("Warning: You may be tired.");
				}
				blinks = 0;
				closed_frames = 0;
				total_frames = 0;
				start = end;

			}
			int key = waitKey(1);
			if (key == 27)
				break;
		}
	}
}


void DrowsinessDetector::start(std::string file)
{
	reset();

	VideoCapture video;
	video.open(file);

	std::vector<dlib::rectangle> faces;
	
	float EAR = 0.0;
	float MAR = 0.0;

	Mat frame;
	Mat gray;
	Mat resized;

	float sumEAR = 0;
	for (size_t i = 0; i < MAX_FRAMES; i++)
	{
		video.read(frame);

		int height = frame.rows * 0.3;
		int width = frame.cols * 0.3;
		
		resize(frame, resized, Size(width, height));
		cv::cvtColor(resized, gray, COLOR_BGR2GRAY);
		equalizeHist(gray, gray);

		cv_image<uchar> dlib_gray(gray);
		faces = detector(dlib_gray);
		for (int i = 0; i < faces.size(); i++)
		{
			full_object_detection landmarks = predictor(dlib_gray, faces[i]);
			EAR = calcEAR(landmarks);
			sumEAR += EAR;
		}
		
	}

	EAR_Threshold = (sumEAR / MAX_FRAMES) * 0.70;

	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now();

	auto frame_start = chrono::steady_clock::now();
	auto frame_end = chrono::steady_clock::now();

	while (video.isOpened())
	{
		faces.clear();
		frame_start = chrono::steady_clock::now();
		video.read(frame);
		if (total_frames++ % FRAME_SKIPS == 0 && !frame.empty())
		{
			int height = frame.rows * 0.3;
			int width = frame.cols * 0.3;

			resize(frame, resized, Size(width, height));
			cv::cvtColor(resized, gray, COLOR_BGR2GRAY);
			equalizeHist(gray, gray);

			cv_image<uchar> dlib_gray(gray);
			faces = detector(dlib_gray);

			for (int i = 0; i < faces.size(); i++) 
			{
				full_object_detection landmarks = predictor(dlib_gray, faces[i]);
				MAR = calcMAR(landmarks);
				EAR = calcEAR(landmarks);

				if (EAR > EAR_Threshold)
				{
					if (close_counter >= EAR_CONSEC_FRAMES)
					{
						blinks++;
					}
					if (close_counter > EYE_CONS_FRAME)
					{
						addNewWarning("Critical warning: You closed your eyes for too long.");
					}
					close_counter = 0;
					
				}
				else
				{
					closed_frames++;
					close_counter++;
				}

				if (MAR < MAR_THRESHOLD)
				{
					if (yawning_counter >= MAR_CONSEC_FRAME && MAR < 0.6)
					{
						addNewWarning("Warning: You may be tired(Yawning).");
					}
					yawning_counter = 0;
				}
				else
				{
					yawning_counter++;
				}
				
				/*if (MAR > MAR_THRESHOLD && prev_MAR < MAR_THRESHOLD && MAR < 0.6)
				{
					addNewWarning("Warning: You may be tired(Yawning).");
				}
				prev_MAR = MAR;*/

				displayStat(frame, blinks, EAR, MAR);
				drawPoints(frame, landmarks);
			}
			
			end = chrono::steady_clock::now();
			if (total_frames >= MAX_FRAMES || blinks > MAX_BLINKS_PER_MIN)
			{
				perclos = closed_frames / total_frames;
				if (blinks > MAX_BLINKS_PER_MIN || blinks < MIN_BLINKS_PER_MIN || perclos > PERCLOS_THRESHOLD)
				{
					addNewWarning("Warning: You may be tired.");
				}
				blinks = 0;
				closed_frames = 0;
				total_frames = 0;
				start = end;
			}
			imshow("Frame", frame);
			//imshow("Gray", gray);
			frame_end = chrono::steady_clock::now();
			
			long processTime = chrono::duration_cast<chrono::milliseconds>(frame_end - frame_start).count();
			if (processTime < 30)
			{
				this_thread::sleep_for(std::chrono::milliseconds(30 - processTime));
			}
			
			//cout << chrono::duration_cast<chrono::milliseconds>(frame_end - frame_start).count() << " ms" << endl;
		}
		int key = waitKey(1);
		if (key == 27)
			break;
	}
	
	TextLog log(file.append(".log"));
	log.putInfo(to_string(numberOfWarnings));
	log.close();
}

void DrowsinessDetector::drawPoints(cv::Mat& image, full_object_detection landmarks)
{
	for (unsigned int i = 0; i < landmarks.num_parts(); i++) {
		cv::circle(image, cv::Point(landmarks.part(i).x(), landmarks.part(i).y()), 3, cv::Scalar(0, 255, 255), -1);
	}
}

float DrowsinessDetector::calcLeftEAR(full_object_detection landmarks)
{
	unsigned int x_37 = landmarks.part(36).x();
	unsigned int y_38 = landmarks.part(37).y();
	unsigned int y_39 = landmarks.part(38).y();
	unsigned int x_40 = landmarks.part(39).x();
	unsigned int y_41 = landmarks.part(40).y();
	unsigned int y_42 = landmarks.part(41).y();

	int height_left_eye1 = y_42 - y_38;
	int height_left_eye2 = y_41 - y_39;
	float height_left_eye = (height_left_eye1 + height_left_eye2) / 2;
	int length_left_eye = x_40 - x_37;
	/*if (height_left_eye == 0)
	* {
		height_left_eye = 1;
		}
	*/

	return height_left_eye / length_left_eye;
}

float DrowsinessDetector::calcRightEAR(full_object_detection landmarks)
{
	unsigned int x_43 = landmarks.part(42).x();
	unsigned int y_44 = landmarks.part(43).y();
	unsigned int y_45 = landmarks.part(44).y();
	unsigned int x_46 = landmarks.part(45).x();
	unsigned int y_47 = landmarks.part(46).y();
	unsigned int y_48 = landmarks.part(47).y();

	int length_right_eye = x_46 - x_43;
	int height_right_eye1 = y_48 - y_44;
	int height_right_eye2 = y_47 - y_45;

	float height_right_eye = (height_right_eye1 + height_right_eye2) / 2;
	/*if (height_right_eye == 0)
	{
		height_right_eye = 1;
	}*/

	return height_right_eye / length_right_eye;
}

float DrowsinessDetector::calcEAR(full_object_detection landmarks)
{
	float leftEar = calcLeftEAR(landmarks);
	float rightEar = calcRightEAR(landmarks);
	float EAR = (leftEar + rightEar) / 2;;
	if (abs(leftEar - rightEar) > 0.2)
	{
		if (leftEar > rightEar)
		{
			EAR = leftEar;
		}
		else
		{
			EAR = rightEar;
		}
	}
	
	return EAR;
}

float DrowsinessDetector::calcMAR(full_object_detection landmarks)
{
	unsigned int x_60 = landmarks.part(60).x();
	unsigned int y_61 = landmarks.part(61).y();
	unsigned int y_62 = landmarks.part(62).y();
	unsigned int y_63 = landmarks.part(63).y();
	unsigned int x_64 = landmarks.part(64).x();
	unsigned int y_65 = landmarks.part(65).y();
	unsigned int y_66 = landmarks.part(66).y();
	unsigned int y_67 = landmarks.part(67).y();

	int length_mouth = x_64 - x_60;
	int height_mouth1 = y_67 - y_61;
	int height_mouth2 = y_66 - y_62;
	int height_mouth3 = y_65 - y_63;

	float height_mouth = (height_mouth1 + height_mouth2 + height_mouth3) / 3;
	/*if (height_mouth == 0)
	{
		height_mouth = 1;
	}*/

	return height_mouth / length_mouth;
}

void DrowsinessDetector::displayStat(Mat& img, int blinks, float ear, float mar)
{
	char str[200];
	sprintf(str, "EAR: %f", ear);
	putText(img, str, Point2f(100, 100), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255, 255));

	char str_blinks[200];
	sprintf(str_blinks, "Blinks: %f", (float)blinks);
	putText(img, str_blinks, Point2f(100, 50), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255, 255));

	char str_MAR[200];
	sprintf(str_MAR, "MAR: %f", mar);
	putText(img, str_MAR, Point2f(100, 150), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255, 255));
}

void DrowsinessDetector::addNewWarning(string msg)
{
	numberOfWarnings++;
	cout << msg << endl;
}

void DrowsinessDetector::reset()
{
	close_counter = 0;
	blinks = 0;
	total_frames = 0;
	closed_frames = 0;
	perclos = 0;
	numberOfWarnings = 0;
}
