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
#include <numeric>
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

	int histSize = 32; //number of bins
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

	int normRange = 1;
	//          src          dst     min    max      norm type  same data type
	normalize(img1Hist_b, img1Hist_b, 0, normRange, NORM_MINMAX, -1 );
	normalize(img1Hist_g, img1Hist_g, 0, normRange, NORM_MINMAX, -1 );
	normalize(img1Hist_r, img1Hist_r, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_b, img2Hist_b, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_g, img2Hist_g, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_r, img2Hist_r, 0, normRange, NORM_MINMAX, -1 );

	int comp_method = CV_COMP_CORREL;
	double comp_b = compareHist(img1Hist_b, img2Hist_b, comp_method);
	double comp_g = compareHist(img1Hist_g, img2Hist_g, comp_method);
	double comp_r = compareHist(img1Hist_r, img2Hist_r, comp_method);
	float comparison = (float)( (comp_b + comp_g + comp_r) / 3 );
	
	return comparison;
}

/* returns a distance metric for the given images based on
 * a comparison of multiple histograms on each image
 */
float distanceMultHist( Mat &img1, Mat &img2 )
{
	int numXDivisions = 2;
	int numYDivisions = 2;
	int subsectionWidth1  = img1.cols/numXDivisions;
	int subsectionHeight1 = img1.rows/numYDivisions;
	int subsectionWidth2  = img2.cols/numXDivisions;
	int subsectionHeight2 = img2.rows/numYDivisions;
	vector<float> compResults;

	for (int i = 0; i < numYDivisions; i++ )
	{
		for (int j = 0; j < numXDivisions; j++ )
		{
			Mat croppedImg1 (img1, Rect(i*subsectionWidth1, j*subsectionHeight1, 
										  subsectionWidth1, subsectionHeight1));
			Mat croppedImg2 (img2, Rect(i*subsectionWidth2, j*subsectionHeight2, 
								subsectionWidth2, subsectionHeight2));
			compResults.push_back(distanceBaselineHist(croppedImg1, croppedImg2));						  
		}
	}
	
	// make sure to use 0.0 instead of 0; otherwise they will be summed as ints
	float sum = std::accumulate(compResults.begin(), compResults.end(), 0.0);
	float average = sum/(numXDivisions * numYDivisions);
	return average;
}

/* helper function that returns a comparison metric for the Sobel filter
 * results of the two given images
 */
double compareSobelHist( Mat &img1, Mat &img2 )
{
	Mat imgOne, imgTwo; //local copies of images for modification

	//blur to reduce noise (src, dst, kernel size, sigmaX & sigmaY from given size)
	GaussianBlur( img1, imgOne, Size(3,3), 0, 0);
	GaussianBlur( img2, imgTwo, Size(3,3), 0, 0);

	//convert to grayscale
	cvtColor( imgOne, imgOne, CV_BGR2GRAY );
	cvtColor( imgTwo, imgTwo, CV_BGR2GRAY );

	//apply Sobel
	Mat grad_x_1, grad_y_1; //gradient output in x and y directions
	Mat grad_x_2, grad_y_2;
	int ddepth = CV_16S; //output image depth

	/* Gradient X (src, dst, output depth,
					1st-order derivative for x, none for y, kernel size (3 is default)) 
	 */
	Sobel( imgOne, grad_x_1, ddepth, 1, 0, 3);
	// Gradient Y (swapped which direction gets the 1)
	Sobel( imgOne, grad_y_1, ddepth, 0, 1, 3);
	Sobel( imgTwo, grad_x_2, ddepth, 1, 0, 3);
	Sobel( imgTwo, grad_y_2, ddepth, 0, 1, 3);

	//scale, get absolute value, convert to unsigned 8-bit
	Mat xGrad1, yGrad1, xGrad2, yGrad2;
	convertScaleAbs( grad_x_1, xGrad1 );
	convertScaleAbs( grad_y_1, yGrad1 );
	convertScaleAbs( grad_x_2, xGrad2 );
	convertScaleAbs( grad_y_2, yGrad2 );

	//calculate histograms of x and y gradient magnitude
	int histSize = 8; //number of bins
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	Mat img1Hist_x, img1Hist_y, img2Hist_x, img2Hist_y;
	//        Mat array, # imgs, channels, mask, output Mat, dims, # bins,    ranges
	calcHist( &xGrad1,   1,      0,       Mat(), img1Hist_x, 1,   &histSize, &histRange);
	calcHist( &yGrad1,   1,      0,       Mat(), img1Hist_y, 1,   &histSize, &histRange);
	calcHist( &xGrad2,   1,      0,       Mat(), img2Hist_x, 1,   &histSize, &histRange);
	calcHist( &yGrad2,   1,      0,       Mat(), img2Hist_y, 1,   &histSize, &histRange);

	//compare histograms
	int comp_method = CV_COMP_CORREL;
	double comp_x = compareHist(img1Hist_x, img2Hist_x, comp_method);
	double comp_y = compareHist(img1Hist_y, img2Hist_y, comp_method);
	double comparison = (comp_x + comp_y) / 2;

	return comparison;
}

/* returns a distance metric for the given images based on
 * a comparison of the texture and color of each image,
 * using histograms and Sobel derivatives
 */
float distanceTextureColor( Mat &img1, Mat &img2 )
{
	//get color component from baseline histogram metric
	float colorDist = distanceBaselineHist( img1, img2 );
	
	float textureDist = (float) compareSobelHist( img1, img2 );

	return colorDist + textureDist;
}

/* returns a distance metric for the given images based on
 * a comparison of the texture (by Sobel filter) and color
 * (in HSV) of each image
 */
float distanceCustom( Mat &img1, Mat &img2 )
{
	//convert to HSV
	Mat imgOne, imgTwo; //local copies of images for modification
	cvtColor( img1, imgOne, COLOR_BGR2HSV );
	cvtColor( img2, imgTwo, COLOR_BGR2HSV );
	
	/// Separate the images by channel
	vector<Mat> hsv_planes1;
	split( imgOne, hsv_planes1 );
	vector<Mat> hsv_planes2;
	split( imgTwo, hsv_planes2 );

	int histSize = 256; //number of bins
	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	Mat img1Hist_h, img1Hist_s, img1Hist_v, img2Hist_h, img2Hist_s, img2Hist_v;
	//       	Mat array, # imgs, channels, mask, output Mat, dims, # bins,    ranges
	calcHist( &hsv_planes1[0], 1,     0,    Mat(), img1Hist_h,  1,   &histSize, &histRange);
	calcHist( &hsv_planes1[1], 1,     0,    Mat(), img1Hist_s,  1,   &histSize, &histRange);
	calcHist( &hsv_planes1[2], 1,     0,    Mat(), img1Hist_v,  1,   &histSize, &histRange);
	calcHist( &hsv_planes2[0], 1,     0,    Mat(), img2Hist_h,  1,   &histSize, &histRange);
	calcHist( &hsv_planes2[1], 1,     0,    Mat(), img2Hist_s,  1,   &histSize, &histRange);
	calcHist( &hsv_planes2[2], 1,     0,    Mat(), img2Hist_v,  1,   &histSize, &histRange);

	int normRange = 1;
	//          src          dst     min    max      norm type  use same data type as src
	normalize(img1Hist_h, img1Hist_h, 0, normRange, NORM_MINMAX, -1 );
	normalize(img1Hist_s, img1Hist_s, 0, normRange, NORM_MINMAX, -1 );
	normalize(img1Hist_v, img1Hist_v, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_h, img2Hist_h, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_s, img2Hist_s, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_v, img2Hist_v, 0, normRange, NORM_MINMAX, -1 );

	int comp_method = CV_COMP_CORREL;
	double comp_h = compareHist(img1Hist_h, img2Hist_h, comp_method);
	double comp_s = compareHist(img1Hist_s, img2Hist_s, comp_method);
	double comp_v = compareHist(img1Hist_v, img2Hist_v, comp_method);
	float comparison = (float)( (comp_h + comp_s + comp_v) / 3 );

	//get texture component
	float textureComp = (float) compareSobelHist( img1, img2 );
	
	return comparison + textureComp;
}

/* returns a distance metric for the given images based on
 * comparing their texture (by gradient orientation) and
 * their color (as HSV)
 */
float distanceGradOrient( Mat &img1, Mat &img2 )
{
	Mat imgOne, imgTwo; //local copies of images for modification

	//blur to reduce noise (src, dst, kernel size, sigmaX & sigmaY from given size)
	GaussianBlur( img1, imgOne, Size(3,3), 0, 0);
	GaussianBlur( img2, imgTwo, Size(3,3), 0, 0);

	//convert to grayscale
	cvtColor( imgOne, imgOne, CV_BGR2GRAY );
	cvtColor( imgTwo, imgTwo, CV_BGR2GRAY );

	//apply Sobel
	Mat grad_x_1, grad_y_1; //gradient output in x and y directions
	Mat grad_x_2, grad_y_2;
	int ddepth = CV_16S; //output image depth

	/* Gradient X (src, dst, output depth,
					1st-order derivative for x, none for y, kernel size (3 is default)) 
	 */
	Sobel( imgOne, grad_x_1, ddepth, 1, 0, 3);
	// Gradient Y (swapped which direction gets the 1)
	Sobel( imgOne, grad_y_1, ddepth, 0, 1, 3);
	Sobel( imgTwo, grad_x_2, ddepth, 1, 0, 3);
	Sobel( imgTwo, grad_y_2, ddepth, 0, 1, 3);

	grad_x_1.convertTo( grad_x_1, CV_32F );
	grad_x_2.convertTo( grad_x_2, CV_32F );
	grad_y_1.convertTo( grad_y_1, CV_32F );
	grad_y_2.convertTo( grad_y_2, CV_32F );

	//use Sobel output to find gradient orientation
	Mat angles1, angles2;
	angles1.create( img1.size(), img1.type() );
	angles2.create( img2.size(), img2.type() );
	phase(grad_x_1, grad_y_1, angles1, true);
	phase(grad_x_2, grad_y_2, angles2, true);

	int histSize = 24; //number of bins
	/// Set the ranges
	float range[] = { 0, 360 } ;
	const float* histRange = { range };

	Mat img1Hist_grad, img2Hist_grad;
	//       	Mat array, # imgs, channels, mask, output Mat, dims, # bins,    ranges
	calcHist( &angles1, 1,     0,    Mat(), img1Hist_grad,  1,   &histSize, &histRange);
	calcHist( &angles2, 1,     0,    Mat(), img2Hist_grad,  1,   &histSize, &histRange);

	int normRange = 1;
	//          src                dst     min    max      norm type  same data type
	normalize(img1Hist_grad, img1Hist_grad, 0, normRange, NORM_MINMAX, -1 );
	normalize(img2Hist_grad, img2Hist_grad, 0, normRange, NORM_MINMAX, -1 );

	int comp_method = CV_COMP_CORREL;
	float comp = (float) compareHist(img1Hist_grad, img2Hist_grad, comp_method);

	//get color component from baseline histogram metric
	float colorDist = distanceBaselineHist( img1, img2 );

	return comp + colorDist;
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

	//TODO: add command-line argument for # output images
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
	stringToFuncMap["HIST"] = &distanceBaselineHist;
	stringToFuncMap["MULTHIST"] = &distanceMultHist;
	stringToFuncMap["TEXCOL"] = &distanceTextureColor;
	stringToFuncMap["CUSTOM"] = &distanceCustom;
	stringToFuncMap["GRADORIENT"] = &distanceGradOrient;

	// Reference to which distance metric function to use
	distFuncPtr funcToUse; 

	// Check if the user's input matches a function in the map
	if (stringToFuncMap.find(funcNameString) == stringToFuncMap.end())
	{
    	//there's no such function to use
		cout << "There is no such function\n";
		exit(-1);
	}
	else
	{
		funcToUse = stringToFuncMap[funcNameString];
	}

    vector<Mat> images = readInImageDir( dirName );

	cout << "Sorting image db...\n\n";
	vector<Mat> sortedImages = sortImageDB( searchImg, images, funcToUse);

	float scaledWidth = 500;
	float scale, scaledHeight;
	for (int i = 0; i < 10; i++)
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