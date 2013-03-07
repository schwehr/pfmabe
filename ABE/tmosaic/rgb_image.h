#ifndef RGB_IMAGE_T
#define RGB_IMAGE_T

#include "image.t"
#include "algebra3.h"
#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <strstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

class rgb_image
{
    public:
        string pathname;
        double east;
        double north;
        double alt;
        double omega;       //  roll
        double phi;         //  pitch
        double kappa;       //  yaw
        mat4   M, MI;
        vec3   zrot;
        vec3   cam;
        double unit_depth;  //  depth (from camera) where 1 pixel = 1 meter
        int zone;
        int rows;
        int cols;
        int alpha;
        image<unsigned char> img[4];
        bool read_params ( FILE *fp );
        bool create_image ( int nrows, int ncols, int alpha=1 );
        void delete_image ();
        bool read_image();
        rgb_image() {
            rows = 0;
            cols = 0;
            alpha = 0;
        }
        rgb_image(int nrows, int ncols, int alpha=1 ) {
            create_image ( nrows, ncols, alpha );
        }
        ~rgb_image() {
            delete_image();
        }
};

#endif
