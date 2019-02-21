/* task1.cpp
 * Implements a content-based image retrieval (CBIR) program.
 * Takes in a query image and an image database, matches the query image
 * to each database image using a distance metric, then sorts the database
 * images according to their similarity to the query images
 * 
 * to run:
 * <path-to-bin>/bin/task1 <# results to show> <optional db directory>
 * 
 * Zena Abulhab and Melody Mao
 * CS365 Spring 2019
 * Project 2
 */
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

// Define a function pointer type for using different distance metrics
typedef float (*distFuncPtr)(Mat&, Mat&);

/* reads in images from the given directory and returns them in a Mat vector */
vector<Mat> readInImageDir( const char *dirname )
{
	DIR *dirp;
	struct dirent *dp;
	printf("Accessing directory %s\n\n", dirname);

	// open the directory
	dirp = opendir( dirname );
	if( dirp == NULL ) {
		printf("Cannot open directory %s\n", dirname);
		exit(-1);
	}

	// loop over the contents of the directory, looking for images
    vector<Mat> images;
	while( (dp = readdir(dirp)) != NULL ) {
		if( strstr(dp->d_name, ".jpg") ||
				strstr(dp->d_name, ".png") ||
				strstr(dp->d_name, ".ppm") ||
				strstr(dp->d_name, ".tif") ) {

			//printf("reading in image file: %s\n", dp->d_name);

            // read the image
            string filename = string( dirname ) + "/" + string( dp->d_name );
            Mat src;
            src = imread( filename );

            // test if the read was successful
            if(src.data == NULL) {
                cout << "Unable to read image" << filename << "\n";
                exit(-1);
            }
            images.push_back( src );
		}
	}

	// close the directory
	closedir(dirp);

    return images;
}

/* returns the sum-squared-distance of a 5x5 block of pixels
 * in the centers of the two given images
 */
float distanceSSD( Mat &img1, Mat &img2 )
{
    //coordinates of 5x5 block corners
    int img1startX = img1.cols / 2 - 2;
    int img1startY = img1.rows / 2 - 2;
    int img2startX = img2.cols / 2 - 2;
    int img2startY = img2.rows / 2 - 2;

    float sum = 0;
    float blueDiff, greenDiff, redDiff;
    for ( int i = 0; i < 5; i++ )
    {
        for ( int j = 0; j < 5; j++ )
        {
            Vec3b color1 = img1.at<Vec3b>( img1startX + i, img1startY + j );
            Vec3b color2 = img2.at<Vec3b>( img2startX + i, img2startY + j ); 

            blueDiff = color1[0] - color2[0];
            greenDiff = color1[1] - color2[1];
            redDiff = color1[2] - color2[2]; 
            sum += (blueDiff * blueDiff) + (greenDiff * greenDiff) + (redDiff * redDiff);           
        }
    }

    return sum;
}

/* returns the sum-squared-distance of a 5x5 block of pixels
 * in the centers of the two given images
 */
float otherDistance( Mat &img1, Mat &img2 )
{
	cout << "YOU THOUGHT IT WAS THE SSD FUNCTION, BUT IT WAS I, DIO!\n";

    //coordinates of 5x5 block corners
    int img1startX = img1.cols / 2 - 2;
    int img1startY = img1.rows / 2 - 2;
    int img2startX = img2.cols / 2 - 2;
    int img2startY = img2.rows / 2 - 2;

    float sum = 0;
    float blueDiff, greenDiff, redDiff;
    for ( int i = 0; i < 5; i++ )
    {
        for ( int j = 0; j < 5; j++ )
        {
            Vec3b color1 = img1.at<Vec3b>( img1startX + i, img1startY + j );
            Vec3b color2 = img2.at<Vec3b>( img2startX + i, img2startY + j ); 

            blueDiff = color1[0] - color2[0];
            greenDiff = color1[1] - color2[1];
            redDiff = color1[2] - color2[2]; 
            sum += (blueDiff * blueDiff) + (greenDiff * greenDiff) + (redDiff * redDiff);           
        }
    }
	

    return sum;
}

/* returns a distance metric for the given images based on
 * a comparison of histograms for the full images
 */
float distanceBaselineHist( Mat &img1, Mat &img2 )
{

	/// Separate the images by channel ( B, G and R )
	vector<Mat> bgr_planes1;
	split( img1, bgr_planes1 );
	vector<Mat> bgr_planes2;
	split( img2, bgr_planes2 );

	int histSize = 256; //number of bins
	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	Mat img1Hist_b, img1Hist_g, img1Hist_r, img2Hist_b, img2Hist_g, img2Hist_r;
	//       	Mat array, # imgs, channels, mask, output Mat, dims, # bins,    ranges
	calcHist( &bgr_planes1[0], 1,     0,    Mat(), img1Hist_b,  1,   &histSize, &histRange);
	calcHist( &bgr_planes1[1], 1,     0,    Mat(), img1Hist_g,  1,   &histSize, &histRange);
	calcHist( &bgr_planes1[2], 1,     0,    Mat(), img1Hist_r,  1,   &histSize, &histRange);
	calcHist( &bgr_planes2[0], 1,     0,    Mat(), img2Hist_b,  1,   &histSize, &histRange);
	calcHist( &bgr_planes2[1], 1,     0,    Mat(), img2Hist_g,  1,   &histSize, &histRange);
	calcHist( &bgr_planes2[2], 1,     0,    Mat(), img2Hist_r,  1,   &histSize, &histRange);

	int comp_method = CV_COMP_CORREL;
	double comp_b = compareHist(img1Hist_b, img2Hist_b, comp_method);
	double comp_g = compareHist(img1Hist_g, img2Hist_g, comp_method);
	double comp_r = compareHist(img1Hist_r, img2Hist_r, comp_method);
	float comparison = (float)( (comp_b + comp_g + comp_r) / 3 );
	
	return comparison;
}

/**
 * Returns true if the second value in the first pair is less than 
 * the second value in the second pair. Used to sort distances for sortImageDB.
 */
bool sortBySecondVal(const pair<Mat, float> &pair1, const pair<Mat, float> &pair2)
{
	return (pair1.second < pair2.second);
}

/* returns a copy of the input image database, sorted by the smallest values
 * for the given distance metric calculated from the given query image
 */
vector<Mat> sortImageDB( Mat &queryImg, vector<Mat> &db, distFuncPtr func)
{
    //calculate distance metric for each db imagedfafdsfdsa

	vector<pair <Mat, float> > imgToDistPairs;

	for (int i = 0; i < db.size(); i++)
	{
		float distance = func(queryImg, db[i]);
		imgToDistPairs.push_back(make_pair(db[i], distance));
	}

	sort(imgToDistPairs.begin(), imgToDistPairs.end(), sortBySecondVal);
	vector<Mat> sortedDb;

	for (int i = 0; i < db.size(); i++)
	{
		sortedDb.push_back(imgToDistPairs[i].first);
	}

	return sortedDb;
}

int main( int argc, char *argv[] ) {
    char dirName[256];
	char searchImgName[256];
	char funcNameString[256];
	Mat searchImg;

	// TODO: Take these defaults out??
	// by default, look at the current directory
	strcpy(dirName, ".");
	strcpy(searchImgName, ".");

	// If user didn't give directory name and query image name
	if(argc != 4) 
	{
		cout << "please provide a directory name and query image name!\n";
		exit(-1);
	}
	strcpy(dirName, argv[1]);
	strcpy(searchImgName, argv[2]);
	strcpy(funcNameString, argv[3]);

	searchImg = imread(searchImgName);

	// Map the user input strings to the functions
	map<string, distFuncPtr> stringToFuncMap;
	stringToFuncMap["SSD"] = &distanceSSD;
	stringToFuncMap["OTHER"] = &otherDistance;

	// Reference to which distance metric function to use
	distFuncPtr funcToUse; 

	// Check if the user's input matches a function in the map
	if (stringToFuncMap.find(funcNameString) == stringToFuncMap.end())
	{
    	//there's no such function to use
		cout << "fghjk\n";
		exit(-1);
	}
	else
	{
		funcToUse = stringToFuncMap[funcNameString];
	}

	// Choose which kind of function to execute

    vector<Mat> images = readInImageDir( dirName );
	vector<Mat> sortedImages = sortImageDB( searchImg, images, funcToUse);


	float scaledWidth = 500;
	float scale, scaledHeight;
	for (int i = 0; i < sortedImages.size(); i++)
	{
		scale = scaledWidth / sortedImages[i].cols;
		scaledHeight = sortedImages[i].rows * scale;
		resize(sortedImages[i], sortedImages[i], Size(scaledWidth, scaledHeight));

		string window_name = "match " + to_string(i);
		namedWindow(window_name, CV_WINDOW_AUTOSIZE);
		imshow(window_name, sortedImages[i]);
	}

	waitKey(0);
		
	printf("\nTerminating\n");

	return(0);

}