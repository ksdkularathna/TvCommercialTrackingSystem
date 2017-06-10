#define FeatureMatching_H
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <string>
using namespace std;
using namespace cv;

typedef struct {
	int frameId;
	int adId;
	string addName;
	float duration;
}adDetails;

int getLastAddId();
int addANewAdvertisement(int addId, std::string addName);
int extractFeaturesAndCreateFeatureFile(int addId, Mat image2, string frameName);

//int templateMatching();
int histogramMatching();
int flannMatching();
int bruteForceMatching();

adDetails  matchKeypoints(Mat image, string time, int status, int directoryId);

adDetails evaluateAd(int addId, int matchCount, float duration);

int writeOutput(int startFrame, float duration, string addName, int status);

adDetails matchLAd(Mat image);

