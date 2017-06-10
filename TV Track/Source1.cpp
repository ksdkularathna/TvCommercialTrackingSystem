#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <Windows.h>
#include <fstream>
#include "FeatureMatching.h" 

using namespace cv;
using namespace std;

typedef struct {
	float sd[4];
	float mean[4];
	float mode[4];
	float eudis[4];
	float sdf[4][4];
	int array[4][256];
}wrap;

wrap histogramcalculation(const Mat &Image)
{
	wrap w;

	int rows = Image.rows;
	int cols = Image.cols;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 256; j++)
			w.array[i][j] = 0;
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			w.sdf[i][j] = 0;
	}

	for (int j = 0; j < 4; j++) {

		int x1, x2, y1, y2;

		if (j == 0) { x1 = 0; y1 = 0; x2 = rows / 2; y2 = cols / 2; }
		else if (j == 2) { x1 = rows / 2; y1 = 0; x2 = rows; y2 = cols / 2; }
		else if (j == 3) { x1 = 0; y1 = cols / 2; x2 = rows / 2; y2 = cols; }
		else { x1 = rows / 2; y1 = cols / 2; x2 = rows; y2 = cols; }

		w.mean[j] = 0;
		w.sd[j] = 0;

		for (int i = x1; i < x2; i++) {
			for (int c = y1; c < y2; c++) {
				int m = (int)Image.at<uchar>(i, c);
				w.array[j][m] = w.array[j][m] + 1;
				w.mean[j] += m;
			}
		}

		w.mean[j] = w.mean[j] / ((rows / 2)*(cols / 2));

		for (int i = x1; i < x2; i++) {
			for (int c = y1; c < y2; c++) {
				int m = (int)Image.at<uchar>(i, c);
				w.sd[j] += pow((m - w.mean[j]), 2);
			}
		}

		w.sd[j] = sqrt((w.sd[j] / ((rows / 2)*(cols / 2))));

		int temp = 0;

		for (int i = 0; i < 256; i++) {
			if (temp < w.array[j][i]) {
				w.mode[j] = i;
				temp = w.array[j][i];
			}
		}

		for (int i = 0; i < 4; i++)
			w.eudis[i] = 0;
	}
	return w;
}

wrap currentHist;
wrap prevHist;

int winsize = 10;
int rounds = 2;

float window[2][4][4][10];
/*******************
i - round
j - frame part
k - feature
l - window
********************/

int main()
{
	try {

		int count = 0;
		
		VideoCapture cap("D:\\Projects\\ads\\Output.mp4");
		//VideoCapture cap("D:\\Projects\\ads\\recording\\recording.mp4"); // open video

		if (!cap.isOpened())  // check if we succeeded
			return -1;

		int matchedAdId = 0;
		int status = 1;
		while (1)
		{
			Mat frame;
			cap >> frame; // get a new frame

			int frameNo = cap.get(CV_CAP_PROP_POS_FRAMES);

			imshow("video", frame);
			cvtColor(frame, frame, CV_BGR2GRAY);

			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 4; j++)
					for (int k = 0; k < 4; k++)
						for (int l = 0; l < winsize; l++)
							window[i][j][k][l] = 0;

			currentHist = histogramcalculation(frame);

			if (count > 0) {
				int success[4] = { 0,0,0,0 };
				for (int j = 0; j < 4; j++) {
					for (int b = 0; b < 4; b++) {
						for (int a = 0; a < winsize - 1; a++) {
							window[0][j][b][a] = window[0][j][b][a + 1];
						}
					}

					float eudis = 0;

					for (int arri = 0; arri < 256; arri++) {
						eudis += pow(currentHist.array[j][arri] - prevHist.array[j][arri], 2);
					}

					eudis = sqrt(eudis);
					//eudis = eudis / currentHist.mean[j];

					float mean[4] = { 0,0,0,0 };
					float sd[4] = { 0,0,0,0 };

					window[0][j][0][winsize - 1] = currentHist.mean[j] - prevHist.mean[j];
					window[0][j][1][winsize - 1] = currentHist.sd[j] - prevHist.sd[j];
					window[0][j][2][winsize - 1] = (3 * (currentHist.mean[j] - currentHist.mode[j]) / (currentHist.sd[j])) - (3 * (prevHist.mean[j] - currentHist.mode[j]) / (prevHist.sd[j]));
					window[0][j][3][winsize - 1] = eudis;

					currentHist.eudis[j] = window[0][j][3][winsize - 1];

					for (int k = 0; k < 4; k++) {
						for (int i = 0; i < winsize; i++) {
							mean[k] += window[0][j][k][i];
						}
						mean[k] /= winsize;
					}

					for (int k = 0; k < 4; k++) {
						for (int i = 0; i < winsize; i++) {
							sd[k] = pow(window[0][j][k][i] - mean[k], 2);
						}
						sd[k] /= winsize;
						sd[k] = sqrt(sd[k]);
					}

					int xeudis = 0;
					if (eudis - prevHist.eudis[3] > 0)
						xeudis = eudis - prevHist.eudis[3];
					//for(int rr=0;rr<4;rr++)

					for (int i = 0; i < 4; i++)
						currentHist.sdf[j][i] = sd[i];

					/*******************************************************************************/

					float mean2[10];
					float sd2[10];

					for (int b = 0; b < 4; b++) {
						for (int a = 0; a < winsize - 1; a++) {
							window[1][j][b][a] = window[1][j][b][a + 1];
						}
					}

					for (int i = 0; i < 4; i++)
						window[1][j][i][winsize - 1] = currentHist.sdf[j][0] - prevHist.sdf[j][0];

					for (int k = 0; k < 4; k++) {
						for (int i = 0; i < winsize; i++) {
							mean2[k] += window[1][j][k][i];
						}
						mean2[k] /= winsize;
					}

					for (int k = 0; k < 4; k++) {
						for (int i = 0; i < winsize; i++) {
							sd2[k] = pow(window[1][j][k][i] - mean2[k], 2);
						}
						sd2[k] /= winsize;
						sd2[k] = sqrt(sd2[k]);
						//result << sd2[k] << ",";
					}

					if (j == 3)
						printf("%f\n", window[0][j][2][winsize - 1]);
					/*******************************************************************************/



					//printf("\n");
					if (sd[0] > 0.1)
						success[0]++;
					if (sd[1] > 0.5)
						success[1]++;
					if (sd[2] > 0.1)
						success[2]++;
					if (eudis - prevHist.eudis[3] > 300)
						success[3]++;



				}
				if (success[0] > 1 && success[3] > 2) {
					string s = "f" + std::to_string(frameNo);
					printf("%d  =======================================\n", s);

					if (status == 2) {
						// this means second phase of the search. peform the search in the choosen directory.
						matchedAdId = matchKeypoints(frame, "12", 2, matchedAdId);
						if (matchedAdId == 0) {
							status = 1;
						}
						std::cout << matchedAdId << " from status 2 " << std::endl;
					}

					if (status == 1) {
						// this means initial status of search. perform the search in only fisrt two files of the directories.
						matchedAdId = matchKeypoints(frame, "12", 1, 0);
						if (matchedAdId != 0) {
							status = 2;
						}
						std::cout << matchedAdId <<  " from status 1 " << std::endl;
					}
				}
			}
			else {
				
			}
			//result << "\n";

			prevHist = currentHist;
			count++;
			waitKey(5);
			//if(frameNo>340)
			//Sleep(400);
//			printf("%d",frameNo);
		}
	}
	catch (cv::Exception& e) {
		return 0;
	}
	return 1;
}