#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include "FeatureMatching.h" 

using namespace std;
using namespace cv;


int histogramMatching() {

	Mat src_base, hsv_base;
	Mat src_test1, hsv_test1;
	Mat src_test2, hsv_test2;
	Mat hsv_half_down;

	src_base = imread("source.jpg", IMREAD_COLOR);
	src_test1 = imread("image3.jpg", IMREAD_COLOR);
	src_test2 = imread("image4.jpg", IMREAD_COLOR);

	if (src_base.empty() || src_test1.empty() || src_test2.empty())
	{
		cout << "Can't read one of the images" << endl;
		return -1;
	}

	// Convert to HSV
	cvtColor(src_base, hsv_base, COLOR_BGR2HSV);
	cvtColor(src_test1, hsv_test1, COLOR_BGR2HSV);
	cvtColor(src_test2, hsv_test2, COLOR_BGR2HSV);

	// Using 50 bins for hue and 60 for saturation
	int h_bins = 50; int s_bins = 60;
	int histSize[] = { h_bins, s_bins };

	// hue varies from 0 to 179, saturation from 0 to 255
	float h_ranges[] = { 0, 180 };
	float s_ranges[] = { 0, 256 };

	const float* ranges[] = { h_ranges, s_ranges };

	// Use the o-th and 1-st channels
	int channels[] = { 0, 1 };

	/// Histograms
	MatND hist_base;
	MatND hist_test1;
	MatND hist_test2;

	/// Calculate the histograms for the HSV images
	calcHist(&hsv_base, 1, channels, Mat(), hist_base, 2, histSize, ranges, true, false);
	normalize(hist_base, hist_base, 0, 1, NORM_MINMAX, -1, Mat());

	// save calulated histogram to a file
	/*cv::FileStorage fs("2.yml", cv::FileStorage::WRITE);
	write(fs, "histogram_data1", hist_base);
	write(fs, "histogram_data2", hist_base);
	fs.release();
	*/

	calcHist(&hsv_test1, 1, channels, Mat(), hist_test1, 2, histSize, ranges, true, false);
	normalize(hist_test1, hist_test1, 0, 1, NORM_MINMAX, -1, Mat());

	calcHist(&hsv_test2, 1, channels, Mat(), hist_test2, 2, histSize, ranges, true, false);
	normalize(hist_test2, hist_test2, 0, 1, NORM_MINMAX, -1, Mat());

	/*FileStorage fs2("2.yml", FileStorage::READ);
	FileNode kptFileNode = fs2["frame 4"];
	read(kptFileNode, hist_test1);
	FileNode kptFileNode2 = fs2["frame 5"];
	read(kptFileNode2, hist_test2);
	fs2.release();

	FileStorage fs2("my_histogram_file.yml", FileStorage::READ);
	FileNode kptFileNode = fs2["keypoints_1"];
	read(kptFileNode, hist_base);
	fs2.release();
	*/

	/// Apply the histogram comparison methods
	for (int i = 0; i < 4; i++)
	{
		int compare_method = i;
		double base_base = compareHist(hist_base, hist_base, compare_method);
		double base_test1 = compareHist(hist_base, hist_test1, compare_method);
		double base_test2 = compareHist(hist_base, hist_test2, compare_method);

		printf(" Method [%d] Perfect, Base-Test(1), Base-Test(2) : %f, %f, %f \n", i, base_base, base_test1, base_test2);
	}

	printf("Done \n");
	return 0;
}