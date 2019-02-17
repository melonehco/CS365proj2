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
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

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
    int img1startX = img1.width / 2 - 2;
    int img1startY = img1.height / 2 - 2;
    int img2startX = img2.width / 2 - 2;
    int img2startY = img2.height / 2 - 2;

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

/* returns a copy of the input image database, sorted by the smallest values
 * for the given distance metric calculated from the given query image
 */
vector<Mat> sortImageDB( Mat &queryImg, vector<Mat> &db, int distanceMetric )
{
    /*TODO: pass in function for distance metric*/

    //calculate distance metric for each db image
    vector<float> distances;
    

}

int main( int argc, char *argv[] ) {
    char dirname[256];

	// by default, look at the current directory
	strcpy(dirname, ".");

	// if the user provided a directory path, use it
	if(argc > 1) {
		strcpy(dirname, argv[1]);
	}

    vector<Mat> images = readInImageDir( dirname );
		
	printf("\nTerminating\n");

	return(0);

}