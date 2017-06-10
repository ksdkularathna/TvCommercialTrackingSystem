#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include "FeatureMatching.h"
#include <math.h>

using namespace std;
using namespace cv;

// added for flann matching
#include "opencv2/opencv_modules.hpp"
#include <stdio.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <fstream>

// added for directory reading
#include <string>
#include <filesystem>
namespace fs = std::experimental::filesystem;

adDetails matchKeypoints(Mat image, string time, int status, int directoryId) {

	if (status == 1) {
		// This means perform the search only for first two directories

		Mat hsv_base;
		if (image.empty())
		{
			cout << "Can't read the images" << endl;
			adDetails d;
			d.frameId = 0;
			d.adId = 0;
			return d;
		}

		//-- Step 1: Detect the keypoints using SURF Detector
		int minHessian = 400;
		SurfFeatureDetector detector(minHessian);
		vector<KeyPoint> keypoints_1;
		keypoints_1.reserve(100000);
		detector.detect(image, keypoints_1);;

		SurfDescriptorExtractor extractor;
		Mat descriptors_1, descriptors_2;
		extractor.compute(image, keypoints_1, descriptors_1);

		std::string path = "db/";
		int addId = 0;
		int frameId = 0;

		bool isNestedLoopsBreaked = false;
		for (auto & p : fs::directory_iterator(path)) {

			//std::cout << p.path().filename() << std::endl;
			bool outerBreak = false;
			int i = 0;
			int c = 0;

			vector<int> paths;
			for (auto & ip : fs::directory_iterator(p)) {
				string temp = ip.path().relative_path().string();
				paths.push_back(stoi(temp.substr(temp.find("f") + 1, temp.find("."))));
				c++;
			}
			std::sort(paths.begin(), paths.end());

			for (int i = 0; i < c; ++i) {
				//std::cout << paths[i] << std::endl;

				//std::cout << ip << std::endl;
				if (i == 4) {
					break; // break the for loop after two iterations
				}
				//std::cout << ip << std::endl;
				string filePath = p.path().relative_path().string() + "\\f" + std::to_string(paths[i]) + ".yml";
				//std::cout << filePath << std::endl;

				FileStorage fs2(filePath, FileStorage::READ);
				FileNode kptFileNode = fs2["f" + std::to_string(paths[i])];
				read(kptFileNode, descriptors_2);
				fs2.release();

				//-- Step 3: Matching descriptor vectors using FLANN matcher
				FlannBasedMatcher matcher;
				std::vector< DMatch > matches;
				matches.reserve(100000);
				matcher.match(descriptors_1, descriptors_2, matches);

				// Evaluate the results
				double max_dist = 0; double min_dist = 100;
				//-- Quick calculation of max and min distances between keypoints
				for (int j = 0; j < descriptors_1.rows; j++)
				{
					double dist = matches[j].distance;
					if (dist < min_dist) min_dist = dist;
					if (dist > max_dist) max_dist = dist;
				}

				if (min_dist == 0 && max_dist == 0) {

					// this means a perfect match is found. No need to search further. loops are breaked.
					// adding the entry to the log

					addId = std::stoi(p.path().filename().string());
					frameId = paths[i];
					outerBreak = true;
					std::cout << "Match found" << std::endl;
					break;
				}
				//std::cout << good_matches.size() << std::endl;
				//if a exact match is not found selecting the most closer one
				std::vector< DMatch > good_matches;
				good_matches.reserve(100000);

				for (int k = 0; k < descriptors_1.rows; k++)
				{
					if (matches[k].distance <= max(2 * min_dist, 0.02))
					{
						good_matches.push_back(matches[k]);
					}
				}
				//std::cout << "Match vector size " << matches.size() << std::endl;
				//std::cout << "Good match vector size " << good_matches.size() << std::endl;
				if (((float)good_matches.size() / (float)matches.size()) * 100 >= 15) {
					// Declaring the matching frame to a given threshold value

					addId = std::stoi(p.path().filename().string());
					frameId = paths[i];
					outerBreak = true;
					std::cout << "Threshold match found" << std::endl;
					break;

				}
			}
			if (outerBreak == true) {

				isNestedLoopsBreaked = true;
				break;
			}
		}
		if (isNestedLoopsBreaked == true) {

			// this means there is a exact matches
			/*std::ofstream outfile;
			outfile.open("matched_adds.csv", std::ios_base::app);
			outfile << mostMatchedAddId << "," << time << endl;
			outfile.close();
			*/

			adDetails ad;
			ad.adId = addId;
			ad.frameId = frameId;

			ifstream ip("add_index.csv");
			if (!ip.is_open()) std::cout << "ERROR: add_index.csv File Open" << '\n';
			string adIdd, adNamed, durationStringd;
			printf("Match Status 2\n");
			while (1) {
				getline(ip, adIdd, ',');
				getline(ip, adNamed, ',');
				getline(ip, durationStringd, '\n');
				if (adIdd.find(std::to_string(addId)) == 0 && adIdd.size() == std::to_string(addId).size()) {
					ad.duration = stof(durationStringd);
					printf("%f\n", ad.duration);
					break;
				}
			}
			ip.close();

			std::ofstream outfile;
			outfile.open("currentAd.txt");
			outfile << adNamed;
			outfile.close();

			return ad;
		}
		//std::cout << "Most feature matched count" << goodMatchesCount << std::endl;
		//std::cout << "Matched add ID " << mostMatchedAddId << std::endl;
		//std::cout << "Match finished" << std::endl;
		adDetails ad;
		ad.adId = 0;
		ad.frameId = 0;
		return ad; // means there is no any matches found
	}

	if (status == 2) {

		// This means second phase of the search. Perform the search in choosen directory

		Mat hsv_base;
		if (image.empty())
		{
			cout << "Can't read the images" << endl;
			adDetails ad;
			ad.adId = 0;
			ad.frameId = 0;
			return ad;
		}

		//-- Step 1: Detect the keypoints using SURF Detector
		int minHessian = 400;
		SurfFeatureDetector detector(minHessian);
		vector<KeyPoint> keypoints_1;
		keypoints_1.reserve(100000);
		detector.detect(image, keypoints_1);;

		SurfDescriptorExtractor extractor;
		Mat descriptors_1, descriptors_2;
		extractor.compute(image, keypoints_1, descriptors_1);

		std::string path = "db/" + std::to_string(directoryId) + "/";

		int addId = 0;
		bool isLoopBreaked = false;
		int frameId = 0;
		for (auto & ip : fs::directory_iterator(path)) {

			//std::cout << ip << std::endl;

			string filePath = ip.path().relative_path().string();
			string fileName = ip.path().filename().string();
			std::size_t pos = fileName.find(".");
			string descriptorId = fileName.substr(0, 0 + pos);

			FileStorage fs2(filePath, FileStorage::READ);
			FileNode kptFileNode = fs2[descriptorId];
			read(kptFileNode, descriptors_2);
			fs2.release();

			//-- Step 3: Matching descriptor vectors using FLANN matcher
			FlannBasedMatcher matcher;
			std::vector< DMatch > matches;
			matches.reserve(100000);
			matcher.match(descriptors_1, descriptors_2, matches);

			// Evaluate the results
			double max_dist = 0; double min_dist = 100;
			//-- Quick calculation of max and min distances between keypoints
			for (int i = 0; i < descriptors_1.rows; i++)
			{
				double dist = matches[i].distance;
				if (dist < min_dist) min_dist = dist;
				if (dist > max_dist) max_dist = dist;
			}

			if (min_dist == 0 && max_dist == 0) {

				// this means a perfect match is found. No need to search further. loops are breaked.
				// adding the entry to the log
				/*std::ofstream outfile;

				outfile.open("matched_adds.csv", std::ios_base::app);
				outfile << p.path().filename().string() << "," << time << endl;
				outfile.close();
				*/
				addId = directoryId;
				frameId = std::stoi(descriptorId.substr(1));
				std::cout << "Match found" << std::endl;
				isLoopBreaked = true;
				break;
			}
			//if a exact match is not found selecting the most closer one
			std::vector< DMatch > good_matches;
			good_matches.reserve(100000);
			for (int k = 0; k < descriptors_1.rows; k++)
			{
				if (matches[k].distance <= max(2 * min_dist, 0.02))
				{
					good_matches.push_back(matches[k]);
				}
			}
			//std::cout << "Match vector size " << matches.size() << std::endl;
			//std::cout << "Good match vector size " << good_matches.size() << std::endl;
			if (((float)good_matches.size() / (float)matches.size()) * 100 >= 15) {
				// Declaring the matching frame to a given threshold value

				addId = directoryId;
				frameId = std::stoi(descriptorId.substr(1));
				isLoopBreaked = true;
				std::cout << "Threshold match found" << std::endl;
				break;
			}
		}
		if (isLoopBreaked == true) {

			// this means there is an exact matches.
			// adding the best match add id to the log
			/*std::ofstream outfile;
			outfile.open("matched_adds.csv", std::ios_base::app);
			outfile << mostMatchedAddId << "," << time << endl;
			outfile.close();
			*/
			adDetails ad;
			ad.adId = addId;
			ad.frameId = frameId;

			ifstream ip("add_index.csv");
			if (!ip.is_open()) std::cout << "ERROR: add_index.csv File Open" << '\n';
			string adIdd, adNamed, durationStringd;
			printf("Match Status 2\n");
			while (1) {
				getline(ip, adIdd, ',');
				getline(ip, adNamed, ',');
				getline(ip, durationStringd, '\n');
				if (adIdd.find(std::to_string(addId)) == 0 && adIdd.size() == std::to_string(addId).size()) {
					ad.duration = stof(durationStringd);
					printf("%f\n", ad.duration);
					break;
				}
			}
			ip.close();

			std::ofstream outfile;
			outfile.open("currentAd.txt");
			outfile << adNamed;
			outfile.close();

			return ad;
		}
		//std::cout << "Most feature matched count" << goodMatchesCount << std::endl;
		//std::cout << "Matched add ID " << mostMatchedAddId << std::endl;
		std::cout << "Match finished" << std::endl;
		adDetails ad;
		ad.adId = 0;
		ad.frameId = 0;
		return ad; // means there is no any matches found
	}
}

adDetails evaluateAd(int addId, int matchCount, float duration) {
	printf("Eval Status 2\n");
	ifstream ip("add_index.csv");
	if (!ip.is_open()) std::cout << "ERROR: add_index.csv File Open" << '\n';
	adDetails ad0;
	if (addId == 0)
		return ad0;


	string adId;
	string addName;
	string durationString;
	int frameCount = 0;


	while (1) {
		if (addId == 0)
			break;
		getline(ip, adId, ',');
		getline(ip, addName, ',');
		getline(ip, durationString, '\n');
		if (adId.find(std::to_string(addId)) == 0 && adId.size() == std::to_string(addId).size()) {
			break;
		}
	}
	//std::cout << adId << " " << addName << " " << endl;

	float adOriginalDuration = std::stof(durationString);

	//&& ((float)matchCount / (float)frameCount) * 100 >= 80

	if (((float)duration / (float)adOriginalDuration) * 100 >= 90) {
		// this means the advertisement is fully telecasted
		adDetails d;
		d.addName = addName;
		d.adId = addId;
		return d;
	}
	else {
		adDetails d;
		d.frameId = 0;
		d.adId = 0;
		d.addName = addName;
		return d;
	}
	ip.close();
}
int writeOutput(int startFrame, float duration, string addName, int status) {

	std::ofstream outfile;
	outfile.open("IterativeInputRead.csv", std::ios_base::app);

	float startTime = (float)startFrame / 30;
	float rounded_down = roundf(startTime * 100) / 100;
	string time = std::to_string(rounded_down);
	int pos = time.find(".");

	//std::cout << time.substr(0, pos) << " " << time.substr(pos).substr(1, 2) << endl;

	string a;
	if (status == 1) {
		a = "Fully telecasted";
	}
	else {
		a = "Partially telecasted";
	}

	outfile << addName << "," << time.substr(0, pos) << "-" << time.substr(pos).substr(1, 2) << "," << duration << "," << a << "\n";
	outfile.close();
	return 1;
}

adDetails matchLAd(Mat image) {

	Mat hsv_base;
	if (image.empty())
	{
		cout << "Can't read the images" << endl;
		adDetails d;
		d.frameId = 0;
		d.adId = 0;
		return d;
	}

	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;
	SurfFeatureDetector detector(minHessian);
	vector<KeyPoint> keypoints_1;
	keypoints_1.reserve(100000);
	detector.detect(image, keypoints_1);;

	SurfDescriptorExtractor extractor;
	Mat descriptors_1, descriptors_2;
	extractor.compute(image, keypoints_1, descriptors_1);

	std::string path = "L/";
	string addName;

	bool isNestedLoopsBreaked = false;
	for (auto & p : fs::directory_iterator(path)) {

		//std::cout << p.path().filename() << std::endl;
		//std::cout << ip << std::endl;
		string filePath = p.path().relative_path().string();
		int pos = p.path().filename().string().find('.');
		string fileName = p.path().filename().string().substr(0, pos);

		FileStorage fs2(filePath, FileStorage::READ);
		FileNode kptFileNode = fs2[fileName];
		read(kptFileNode, descriptors_2);
		fs2.release();

		//-- Step 3: Matching descriptor vectors using FLANN matcher
		FlannBasedMatcher matcher;
		std::vector< DMatch > matches;
		matches.reserve(100000);
		matcher.match(descriptors_1, descriptors_2, matches);

		// Evaluate the results
		double max_dist = 0; double min_dist = 100;
		//-- Quick calculation of max and min distances between keypoints
		for (int j = 0; j < descriptors_1.rows; j++)
		{
			double dist = matches[j].distance;
			if (dist < min_dist) min_dist = dist;
			if (dist > max_dist) max_dist = dist;
		}

		if (min_dist == 0 && max_dist == 0) {

			// this means a perfect match is found. No need to search further. loops are breaked.
			// adding the entry to the log

			addName = fileName;
			isNestedLoopsBreaked = true;
			std::cout << "Match found" << std::endl;
			break;
		}
		//std::cout << good_matches.size() << std::endl;
		//if a exact match is not found selecting the most closer one
		std::vector< DMatch > good_matches;
		good_matches.reserve(100000);

		for (int k = 0; k < descriptors_1.rows; k++)
		{
			if (matches[k].distance <= max(2 * min_dist, 0.02))
			{
				good_matches.push_back(matches[k]);
			}
		}
		//std::cout << "Match vector size " << matches.size() << std::endl;
		//std::cout << "Good match vector size " << good_matches.size() << std::endl;
		if (((float)good_matches.size() / (float)matches.size()) * 100 >= 15) {
			// Declaring the matching frame to a given threshold value

			addName = std::stoi(p.path().filename().string());
			isNestedLoopsBreaked = true;
			std::cout << "Threshold match found" << std::endl;
			break;

		}
	}
	if (isNestedLoopsBreaked == true) {

		adDetails ad;
		ad.addName = addName;
		return ad;
	}
	//std::cout << "Most feature matched count" << goodMatchesCount << std::endl;
	//std::cout << "Matched add ID " << mostMatchedAddId << std::endl;
	//std::cout << "Match finished" << std::endl;
	adDetails ad;
	ad.adId = 0;
	return ad; // means there is no any matches found
}
