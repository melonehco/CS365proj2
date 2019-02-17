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

int main(int argc, char *argv[]) {
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