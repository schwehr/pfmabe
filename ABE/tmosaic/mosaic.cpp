#include "tile.h"
#include "image.t"
#include "params.h"
#include "rgb_image.h"
#include "algebra3.h"
#include "process_mgr.h"
#include "version.h"
#include <vector>

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <proj_api.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <ostream>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <leastsquares.h>

using namespace std;

process_env pe;                 // Object for managing process requests/messages

image<double> inverse_scale;
image<double> east_trans;
image<double> north_trans;

int readers;                    // Number of reader processes to fork
int mappers;                    // Number of mapper processes to fork
int writers;                    // Number of writer processes (1 is probably the limit)

//
//  The tiled mosaic program will divide the output tif file into a collection
//  of rectangular tiles.  The width and height of each block are obtained from 
//  the parameter file and adjusted (up) to be powers of 2.
//
int block_x;                    // width of data blocks
int block_y;                    // height of data blocks
int x_blocks;                   // number of blocks across the output
int y_blocks;                   // number of blocks down the output
int n_blocks;

//
//  Blocks will be numbered 0, 1, ,,, x_blocks*y_blocks-1
//  Given block b, we need to know its first row and column.
//  This depends on whether we are going first across or down.
//  We will operate using rows vs cols
//  If rows <= cols we will go across first:
//     row(b) = (b / y_blocks) * block_y;
//     col(b) = (b % x_blocks) * block_x;
//  otherwise
//     row(b) = (b % y_blocks) * block_y;
//     col(b) = (b / x_blocks) * block_x;
//

//
//  The input camera images are stored in the imgs vector.  Each object in the
//  vector has information about the image and pointers to red, green and blue
//  image arrays allocated with mmap to be sharable with all the mosaic
//  processes.
vector<rgb_image> imgs;
int num_imgs;

//
//  These variables define the output region.
//
double east1;               // Minimum easting value (west)
double east2;               // Maximum easting value (east)
double north1;              // Minimum northing value (south)
double north2;              // Maximum northing value (north)
double wid;                 // Width (and also height) of output pixels in meters
double longitude_left;
double longitude_right;
double latitude_bottom;
double latitude_top;

//
//  The distance values stored in the distance image are actually distance
//  squared.  Initially the distance image is filled with a value greater than
//  the squared distance from the center of an RGB image to its corners (+44%).
//
double max_dist;            // The maximum distance in meters
double max_dist_2;          // The maximum distance squared

//
//  The DEM file needs to be related to the output file by geographic
//  coordinates in order to project properly from the output point to input
//  image plane.
//
double dem_east;            // The easting value of column 0 of the DEM file
double dem_north;           // The northing value of row 0 of the DEM file
double dem_wid;             // The width (and height) of DEM file pixels

//
//  The mapping for water is done with the surface (more or less) rather than
//  the bottom.  This means that if the elevation value is less than the value
//  for sea level, the program uses the sea level value instead.  This has no
//  effect for pixels where the elevation is above sea level.
//
double sea_level;

int rows, cols;
int alpha;
int input_rows, input_cols;
int inv_rows, inv_cols;
double lens[5];
double row_offset, column_offset;



double (*xy)[2];

int *valid_img;             // Whether an rgb image is ready to use by a mapper
int *first_needed;          // First tile needing an rgb image
int *last_needed;           // Last tile needing an rgb image (time to re-use!)
int *img_order;             // Order for reading images
int *img_free_order;        // Order for freeing images
int *img_replacement;
int max_needed;             // The number of images needed to keep mappers busy
int slack;                  // Number of extra rgb images to keep readers busy
int max_rgb_images;         // max_needed + slack to smooth the flow
int active_images;          // Number of images in use

int output_utm = 1;         // Default to utm, if 0 use geographic coords

double cell_width;          // With geographic coordinates, width and height
double cell_height;         // of output cells are in degrees

char *pj_args[3] = { "proj=utm", "ellps=WGS84", NULL };

projUV p1, p2;              // Holds points in UTM or geographic
projPJ pj;                  // Holds projection information

params p;

void compute_utm_trans()
{
    int r, c, rowsx, colsx;

    rowsx = (rows + 9)/10+1;
    colsx = (cols + 9)/10+1;
    east_trans.create(rowsx,colsx);
    north_trans.create(rowsx,colsx);

    for ( r = 0; r < rowsx; r++ ) {
        p1.v = latitude_top - r * cell_height * 10;
        p1.v *= DEG_TO_RAD;
        for ( c = 0; c < colsx; c++ ) {
            p1.u = longitude_left + c * cell_width * 10;
            p1.u *= DEG_TO_RAD;
            p2 = pj_fwd(p1, pj);
            east_trans[r][c] = p2.u;
            north_trans[r][c] = p2.v;
        }
    }
}

void compute_geographic_cellsize()
{
    char t[32];
    double center_lon, center_lat, s;

    pj_args[2] = t;
    sprintf(pj_args[2],"zone=%s", p.value("utm_zone").c_str() );
    if (!(pj = pj_init(3, pj_args))) exit(1);

    center_lon = (longitude_left+longitude_right)/2.0;
    center_lat = (latitude_bottom+latitude_top)/2.0;
    p1.u = center_lon;
    p1.v = center_lat;
    p1.u *= DEG_TO_RAD;
    p1.v *= DEG_TO_RAD;
    p1 = pj_fwd(p1, pj);
    p2.u = p1.u+wid;
    p2.v = p1.v;
    p1 = pj_inv(p1, pj);
    p2 = pj_inv(p2, pj);
    p1.u *= RAD_TO_DEG;
    p1.v *= RAD_TO_DEG;
    p2.u *= RAD_TO_DEG;
    p2.v *= RAD_TO_DEG;
    printf("%.9f %.9f\n", p1.u, p1.v);
    printf("%.9f %.9f\n", p2.u, p2.v);
    cell_width = sqrt((p2.u-p1.u)*(p2.u-p1.u)+(p2.v-p1.v)*(p2.v-p1.v));
    s = cell_width * 3600;
    printf("width %.12f\t%.9f\n", cell_width, s);
    p1.u = center_lon;
    p1.v = center_lat;
    p1.u *= DEG_TO_RAD;
    p1.v *= DEG_TO_RAD;
    p1 = pj_fwd(p1, pj);
    p2.u = p1.u;
    p2.v = p1.v+wid;
    p1 = pj_inv(p1, pj);
    p2 = pj_inv(p2, pj);
    p1.u *= RAD_TO_DEG;
    p1.v *= RAD_TO_DEG;
    p2.u *= RAD_TO_DEG;
    p2.v *= RAD_TO_DEG;
    printf("%.9f\t%.9f\n", p1.u, p1.v);
    printf("%.9f\t%.9f\n", p2.u, p2.v);
    cell_height = sqrt((p2.u-p1.u)*(p2.u-p1.u)+(p2.v-p1.v)*(p2.v-p1.v));
    s = cell_height * 3600;
    printf("height %.12f\t%.9f\n", cell_height, s);
}

image<float> dem;
rgb_image *out;
image<float> *dist;
image<short> *img;
image<float> distx;
image<short> imgx;
tile *tiles;
double camera_roll, camera_pitch, camera_yaw;
mat4 cam;

//
//  Function to compute the first row number for a tile (block)
//
int first_row ( int b )
{
    if ( rows >= cols ) return (b / x_blocks) * block_y;
    else return (b % y_blocks) * block_y;
}

//
//  Function to compute the first column number for a tile (block)
//
int first_col ( int b )
{
    if ( rows >= cols ) return (b % x_blocks) * block_x;
    else return (b / y_blocks) * block_x;
}

//
//  Function to find the x value where the perpindicular bisector of
//  points (r1,c1) and (r2,c2) intersects row r.
//
//  This could be useful if the Voronoi code is rewritten to define
//  intervals for each output row rather than preparing an image.
//
double xcross ( double r1, double c1, double r2, double c2, double r )
{
    double rm, cm, dxy, diff;

    rm = (r1 + r2) / 2.0;
    cm = (c1 + c2) / 2.0;
    dxy = -(r2 - r1) / (c2 - c1);
    diff = r - rm;
    return cm + dxy * diff;
}

//
//  This functions sets defaults for some of the parameters used in the
//  mosaic program.
//
void set_defaults()
{
    p.set("images_file",      "centers" );
    p.set("easting_left",     "379000.0" );
    p.set("easting_right",    "382500.0" );
    p.set("northing_top",     "2938000.0" );
    p.set("northing_bottom",  "2937000.0" );
    p.set("utm_zone",         "17" );
    p.set("output_projection", "utm" );
    p.set("output_cell_size", "1.0" );
    p.set("input_cell_size",  "0.2" );
    p.set("input_rows",       "1200" );
    p.set("input_columns",    "1600" );
    p.set("output_file",      "rgb.tif");
    p.set("dist_file",        "");
    p.set("blob_file",        "");
    p.set("overviews",        "3" );
    p.set("focal_length",     "16.065" );
    p.set("pixel_size",       "7.4" );
    p.set("readers",          "2" );
    p.set("writers",          "1" );
    p.set("mappers",          "2" );
    p.set("block_x",          "512" );
    p.set("block_y",          "512" );
    p.set("slack",            "20" );
    p.set("camera_roll",      "0.0" );
    p.set("camera_pitch",     "10.0" );
    p.set("camera_yaw",       "0.0" );
    p.set("k0",               "1.0" );
    p.set("k1",               "0.0" );
    p.set("k2",               "0.0" );
    p.set("k3",               "0.0" );
    p.set("k4",               "0.0" );
    p.set("alpha",            "0" );

}

double elev_at ( vec2 p )
{
    int dr, dc;
    dr = int((dem_north - p[1]) / dem_wid + 0.5);
    dc = int((p[0] - dem_east) / dem_wid + 0.5);
    if ( dr < 0 ) dr = 0;
    if ( dr >= dem.rows ) dr = dem.rows - 1;
    if ( dc < 0 ) dc = 0;
    if ( dc >= dem.cols ) dc = dem.cols - 1;
    return dem[dr][dc];
}

vec2 find_surface_point ( rgb_image &i )
{
    double mid, low, high, mid_elev, low_elev, high_elev;
    vec3 center3(0,0,0);
    vec2 dir, pos, center;


    center3[0] = 0;
    center3[1] = 0;
    center3[2] = -i.alt;
    center3 = i.M * center3;
    center[0] = i.east;
    center[1] = i.north;
    dir[0] = center3[0] - i.east;
    dir[1] = center3[1] - i.north;

    mid = 0.5;
    low = 0.0;
    high = 1.0;
    high_elev = i.alt;
    low_elev = 0.0;
    while ( high - low > 0.0005 ) {
        mid = (high + low) / 2.0;
        pos = center + dir*mid;
        mid_elev = elev_at ( pos );
        //printf("mid %g %g %g, pos %9.2f %10.2f\n",
                //mid, mid_elev, (1.0-mid)*i.alt, pos[0], pos[1]);
        if ( mid_elev <  (1.0-mid) * i.alt ) {
            low = mid;
        } else {
            high = mid;
        }
    }
    return pos;
}
//
//  This function places an RGB image index into img and the distance from
//  the image center at pixel (r,c).  The distance will be 0 if the center
//  of the image is at (r,c), but many images will be centered outside img.
//
void add ( image<short> & img, image<float> & dist, int i, int r, int c )
{
    double d;

    d = (r-xy[i][0])*(r-xy[i][0]) + (c-xy[i][1])*(c-xy[i][1]);
    if ( d < dist[r+1][c+1] ) {
        img[r+1][c+1] = i;
        dist[r+1][c+1] = d;
    }
}

//
//  This function places the row and column number for the projected
//  center of each RGB image into the array xy.
//
void add_imgs_to_xy (int shrink = 1)
{
    vec2 center;
    double east, north;
    int i;

    if ( xy == 0 ) xy = new double[num_imgs+1][2];
    xy[0][0] = -2*max_dist;
    xy[0][1] = -2*max_dist;
    for ( i = 1; i <= num_imgs; i++ ) {
        center = find_surface_point ( imgs[i] );
        north = center.n[1];
        east = center.n[0];
        if ( output_utm ) {
            //printf("%d, %g %g %g, cam %g %12.8g, proj %g %12.8g\n",
            //q->id, q->omega,q->phi, q->kappa, q->east, q->north, east, north);
            xy[i][0] = (north2 - north)/wid / shrink;
            xy[i][1] = (east - east1)/wid / shrink;
        } else {
            p1.u = east;
            p1.v = north;
            p1 = pj_inv ( p1, pj );
            p1.u *= RAD_TO_DEG;
            p1.v *= RAD_TO_DEG;
            xy[i][0] = (latitude_top-p1.v) / cell_height / shrink;
            xy[i][1] = (p1.u-longitude_left) / cell_width / shrink;
        }
    }
}

//
//  This function fills the distance image for a tile with large values and
//  then adds the centers of all images needed for the tile to the Voronoi
//  image (img) and sets the proper distance squared values for the centers.
//
void fill ( int t, image<short> & img, image<float> & dist, int row1, int col1 )
{
    int r, c;
    int i;
    double d;
    set<short>::iterator it;

    for ( r=0; r < img.rows; r++ ) {
        for ( c=0; c < img.cols; c++ ) {
            img[r][c] = 0;
            dist[r][c] = 100000000000.0;
        }
    }

    it = tiles[t].imgs_needed.begin();
    while ( it != tiles[t].imgs_needed.end() ) {
        i = *it;
        r = (int)xy[i][0];
        c = (int)xy[i][1];
        //printf("i %d, r %d, c %d\n", i, r, c);
        if ( r < row1-1 ) r = row1-1;
        if ( r > row1+block_y ) r = row1+block_y;
        if ( c < col1-1 ) c = col1-1;
        if ( c > col1+block_x ) c = col1+block_x;
        d = ((double)r-xy[i][0])*(r-xy[i][0]) +
            ((double)c-xy[i][1])*(c-xy[i][1]);
        //printf("i %d, r %d, c %d, row1 %d, col1 %d, d %g, dist %g, img %d\n",
                //i, r, c, row1, col1, d, dist[r+1-row1][c+1-col1], img[r+1-row1][c+1-col1]);
        if ( img[r+1-row1][c+1-col1] == 0 || d < dist[r+1-row1][c+1-col1] ) {
            dist[r+1-row1][c+1-col1] = d;
            img[r+1-row1][c+1-col1] = i;
        }
        //printf("i %d, r %d, c %d, row1 %d, col1 %d, d %g, dist %g, img %d\n",
                //i, r, c, row1, col1, d, dist[r+1-row1][c+1-col1], img[r+1-row1][c+1-col1]);
        it++;
    }
}

//
//  This function reads the file containing RGB image file names, position
//  data and orientation data into imgs.
//
void read_image_file()
{
    FILE *fp;
    rgb_image rgb, *q;
    double east, north, alt;
    int i;
    vec3 utm(0,0,0), x(1,0,0), y(0,1,0), z(0,0,1);
    mat4 trans, roll, pitch, yaw;

    row_offset = p.dvalue("row_offset");
    column_offset = p.dvalue("column_offset");
    camera_roll = p.dvalue("camera_roll");
    camera_pitch = p.dvalue("camera_pitch");
    camera_yaw = p.dvalue("camera_yaw");
    roll = rotation3D ( x, camera_roll );
    pitch = rotation3D ( y, camera_pitch );
    yaw = rotation3D ( z, camera_yaw );
    cam = yaw;
    cam = cam * pitch;
    cam = cam * roll;
    fp = fopen ( p.value("images_file").c_str(), "r" );

    if ( !fp ) {
        fprintf(stderr,"Could not open images file: %s\n",
                p.value("images_file").c_str() );
        exit(1);
    }

    imgs.push_back(rgb);        // Don't use imgs[0]

    while ( rgb.read_params(fp) ) {
        imgs.push_back(rgb);
        num_imgs++;
    }
    fclose(fp);

    sea_level = p.dvalue("sea_level");
    for ( i = 1; i <= num_imgs; i++ ) {
        q = &imgs[i];
        //printf("%s\n", q->pathname.c_str());
        //printf("%d %10.2f %10.2f %7.2f %7.2f %7.2f %7.2f\n",p->id,
        //p->east, p->north, p->alt,
        //p->omega, p->phi, p->kappa );
        utm[0]=q->east;
        utm[1]=q->north;
        trans = translation3D(utm);
        pitch = rotation3D ( x, q->omega);
        roll = rotation3D ( y, q->phi);
        yaw = rotation3D ( z, q->kappa);
        q->M = pitch;
        q->M = q->M * roll;
        q->M = q->M * yaw;
        q->zrot = -1.0 * z;
        q->zrot = q->M * q->zrot;
        q->M = trans;
        q->M = q->M * pitch;
        q->M = q->M * roll;
        q->M = q->M * yaw;
        q->M = q->M * cam;
        q->MI = q->M.inverse();
        q->unit_depth = p.dvalue("focal_length")*1000.0/p.dvalue("pixel_size");
        q->cam = utm;
        q->cam[2] = 0;
    }
}

void output_pcolor_geotiff ( image<short> & img, image<float> & dist,
        image<unsigned char> & v )
{
    int r, c;
    int val;
    OGRSpatialReference ref;
    GDALDataset  *df;
    char *wkt = NULL;
    GDALRasterBand *bd;
    double        trans[6];
    GDALDriver   *gt;
    char         **options = NULL;
    int          ov[] = { 2, 4, 8, 16, 32 };
    int          nov;
    int          i, n;
    char         file[1000];
    GDALColorTable ct;
    GDALColorEntry color;

    color.c1 = 0;
    color.c2 = 0;
    color.c3 = 0;
    color.c4 = 255;
    ct.SetColorEntry ( 0, &color );
    for ( i = 1; i < 256; i++ ) {
        color.c1 = 30 + rand() % 240 - rand()%30;
        color.c2 = 20 + rand() % 200 - rand()%20;
        color.c3 = 0 + rand() % 160;
        ct.SetColorEntry ( i, &color );
    }

    options = CSLSetNameValue ( options, "TILED", "YES" );
    options = CSLSetNameValue ( options, "COMPRESS", "None" );

    gt = GetGDALDriverManager()->GetDriverByName("GTiff");
    if ( !gt ) {
        fprintf(stderr,"Could not get GTiff driver\n");
        exit(1);
    }

    strcpy ( file, p.value("blob_file").c_str() );
    df = gt->Create( file, v.cols, v.rows, 1, GDT_Byte, options );
    if( df == NULL ) {
        fprintf(stderr,"Could not create %s\n", file );
        exit(1);
    }

    trans[0] = p.dvalue("easting_left");
    trans[1] = p.dvalue("output_cell_size");
    trans[2] = 0.0;
    trans[3] = p.dvalue("northing_top");
    trans[4] = 0.0;
    trans[5] = -p.dvalue("output_cell_size");
    df->SetGeoTransform ( trans );
    ref.SetUTM ( p.ivalue("utm_zone") );
    ref.SetWellKnownGeogCS ( "WGS84" );
    ref.exportToWkt ( &wkt );
    df->SetProjection(wkt);
    CPLFree ( wkt );

    for ( r=0; r < v.rows; r++ ) {
        for ( c=0; c < v.cols; c++ ) {
            if ( dist[r+1][c+1] == 0 ) v[r][c] = 0;
            else v[r][c] = img[r+1][c+1] % 256;
        }
    }

    bd = df->GetRasterBand(1);
    bd->SetColorTable ( &ct );
    bd->RasterIO( GF_Write, 0, 0, v.cols, v.rows, v.data,
            v.cols, v.rows, GDT_Byte, 0, 0 );

    delete df;

    df = (GDALDataset *)GDALOpen ( file, GA_Update );
    if( df == NULL ) {
        fprintf(stderr,"Could not open for update %s\n", file );
        exit(1);
    }
    nov = p.ivalue("overviews");
    if ( nov > 5 ) nov = 5;
    if ( nov > 0 ) {
        n = 1;
        df->BuildOverviews("NEAREST", nov, ov, 1, &n, NULL, NULL );
    }

}

void output_dist_geotiff ( image<float> & dist, image<unsigned char> & v )
{
    int r, c;
    int val;
    OGRSpatialReference ref;
    GDALDataset  *df;
    char *wkt = NULL;
    GDALRasterBand *bd;
    double        trans[6];
    GDALDriver   *gt;
    char         **options = NULL;
    int          ov[] = { 2, 4, 8, 16, 32 };
    int          nov;
    int          n;
    char         file[1000];

    options = CSLSetNameValue ( options, "TILED", "NO" );
    options = CSLSetNameValue ( options, "COMPRESS", "LZW" );

    gt = GetGDALDriverManager()->GetDriverByName("GTiff");
    if ( !gt ) {
        fprintf(stderr,"Could not get GTiff driver\n");
        exit(1);
    }

    strcpy ( file, p.value("dist_file").c_str() );
    df = gt->Create( file, v.cols, v.rows, 1, GDT_Byte, options );
    if( df == NULL ) {
        fprintf(stderr,"Could not create %s\n", file );
        exit(1);
    }

    trans[0] = p.dvalue("easting_left");
    trans[1] = p.dvalue("output_cell_size");
    trans[2] = 0.0;
    trans[3] = p.dvalue("northing_top");
    trans[4] = 0.0;
    trans[5] = -p.dvalue("output_cell_size");
    df->SetGeoTransform ( trans );
    ref.SetUTM ( p.ivalue("utm_zone") );
    ref.SetWellKnownGeogCS ( "WGS84" );
    ref.exportToWkt ( &wkt );
    df->SetProjection(wkt);
    CPLFree ( wkt );

    for ( r=0; r < v.rows; r++ ) {
        for ( c=0; c < v.cols; c++ ) {
            val = int(sqrt(dist[r+1][c+1])+0.5);
            if ( val > 255 ) val = 255;
            v[r][c] = val;
        }
    }

    bd = df->GetRasterBand(1);
    bd->RasterIO( GF_Write, 0, 0, v.cols, v.rows, v.data,
            v.cols, v.rows, GDT_Byte, 0, 0 );

    delete df;

    df = (GDALDataset *)GDALOpen ( file, GA_Update );
    if( df == NULL ) {
        fprintf(stderr,"Could not open for update %s\n", file );
        exit(1);
    }
    nov = p.ivalue("overviews");
    if ( nov > 5 ) nov = 5;
    if ( nov > 0 ) {
        n = 1;
        df->BuildOverviews("NEAREST", nov, ov, 1, &n, NULL, NULL );
    }

}

void output_geotiff()
{
    int t, k, i;
    int val;
    OGRSpatialReference ref;
    GDALDataset  *df;
    char *wkt = NULL;
    GDALRasterBand *bd;
    double        trans[6];
    GDALDriver   *gt;
    char         **options = NULL;
    int          ov[] = { 2, 4, 8, 16, 32 };
    int          nov;
    int          trows, tcols;
    int          bands[] = { 1, 2, 3 };
    char         file[1000];
    char         s[10];
    set<short>::iterator it;

    options = CSLSetNameValue ( options, "TILED", "YES" );
    sprintf(s,"%d",block_x);
    options = CSLSetNameValue ( options, "BLOCKXSIZE", s );
    sprintf(s,"%d",block_y);
    options = CSLSetNameValue ( options, "BLOCKYSIZE", s );
    options = CSLSetNameValue ( options, "COMPRESS", "None" );

    gt = GetGDALDriverManager()->GetDriverByName("GTIFF");
    if ( !gt ) {
        fprintf(stderr,"Could not get GTiff driver\n");
        pe.send_shutdown();
        exit(1);
    }

    strcpy ( file, p.value("output_file").c_str() );
    df = gt->Create( file, cols, rows, alpha?4:3, GDT_Byte, options );
    if( df == NULL ) {
        fprintf(stderr,"Could not create %s\n", file );
        pe.send_shutdown();
        exit(1);
    }

    if ( output_utm ) {
        trans[0] = p.dvalue("easting_left");
        trans[1] = p.dvalue("output_cell_size");
        trans[2] = 0.0;
        trans[3] = p.dvalue("northing_top");
        trans[4] = 0.0;
        trans[5] = -p.dvalue("output_cell_size");
        df->SetGeoTransform ( trans );
        ref.SetUTM ( p.ivalue("utm_zone") );
        ref.SetWellKnownGeogCS ( "WGS84" );
        ref.exportToWkt ( &wkt );
        df->SetProjection(wkt);
        CPLFree ( wkt );
    } else {
        trans[0] = longitude_left;
        trans[1] = cell_width;
        trans[2] = 0.0;
        trans[3] = latitude_top;
        trans[4] = 0.0;
        trans[5] = -cell_height;
        df->SetGeoTransform ( trans );
        ref.SetWellKnownGeogCS ( "WGS84" );
        ref.exportToWkt ( &wkt );
        df->SetProjection(wkt);
        CPLFree ( wkt );
    }

    for ( t = 0; t < n_blocks; t++ ) {
        k = t % mappers;
        trows = tiles[t].rows;
        tcols = tiles[t].cols;
        pe.wait_for("data",t);
        for ( i = 0; i < 3; i++ ) {
            bd = df->GetRasterBand(i+1);
            //bd->RasterIO( GF_Write, tiles[t].first_col, tiles[t].first_row,
                    //tcols, trows, out[k].img[i].data,
                    //block_x, block_y, GDT_Byte, 0, 0 );
            bd->WriteBlock ( tiles[t].first_col/block_x,
                             tiles[t].first_row/block_y,
                             out[k].img[i].data );
        }
        if ( alpha ) {
            bd = df->GetRasterBand(4);
            bd->WriteBlock ( tiles[t].first_col/block_x,
                             tiles[t].first_row/block_y,
                             out[k].img[3].data );
        }
        it = tiles[t].imgs_to_free.begin();
        while ( it != tiles[t].imgs_to_free.end() ) {
            i = *it;
            if ( img_replacement[i] > 0 ) {
                pe.report_complete ( "ready", img_replacement[i] );
            }
            it++;
        }

        pe.report_complete ( "writer", t );

        printf ("Writer status - %d of %d blocks completed                 \r", t, n_blocks);
        fflush (stdout);
    }

    delete df;

    df = (GDALDataset *)GDALOpen ( file, GA_Update );
    if( df == NULL ) {
        fprintf(stderr,"Could not open for update %s\n", file );
        pe.send_shutdown();
        exit(1);
    }
    nov = p.ivalue("overviews");
    if ( nov > 5 ) nov = 5;
    if ( nov > 0 ) {
        df->BuildOverviews("NEAREST", nov, ov, 3, bands, NULL, NULL );
    }

}

void spread_dist ( image<short> & img, image<float> & dist, int row1=0, int col1=0 )
{
    int r, c;
    long i, j, k;
    double di, dj, dk;
    double d;
    double c1, c2;

    for ( r=1; r < img.rows-1; r++ ) {
        for ( c=1; c < img.cols-1; c++ ) {
            i = img[r][c-1];
            j = img[r-1][c-1];
            k = img[r-1][c];
            di = (xy[i][0]-r-row1+1)*(xy[i][0]-r-row1+1) +
                 (xy[i][1]-c-col1+1)*(xy[i][1]-c-col1+1);
            dj = (xy[j][0]-r-row1+1)*(xy[j][0]-r-row1+1) +
                 (xy[j][1]-c-col1+1)*(xy[j][1]-c-col1+1);
            dk = (xy[k][0]-r-row1+1)*(xy[k][0]-r-row1+1) +
                 (xy[k][1]-c-col1+1)*(xy[k][1]-c-col1+1);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( dk < di ) {
                di = dk;
                i = k;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
        for ( c=img.cols-2; c >= 1; c-- ) {
            j = img[r][c+1];
            i = img[r-1][c+1];
            k = img[r-1][c];
            di = (xy[i][0]-r-row1+1)*(xy[i][0]-r-row1+1) +
                 (xy[i][1]-c-col1+1)*(xy[i][1]-c-col1+1);
            dj = (xy[j][0]-r-row1+1)*(xy[j][0]-r-row1+1) +
                 (xy[j][1]-c-col1+1)*(xy[j][1]-c-col1+1);
            dk = (xy[k][0]-r-row1+1)*(xy[k][0]-r-row1+1) +
                 (xy[k][1]-c-col1+1)*(xy[k][1]-c-col1+1);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( dk < di ) {
                di = dk;
                i = k;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
    }

    for ( r=img.rows-2; r >= 1; r-- ) {
        for ( c=img.cols-2; c >= 1; c-- ) {
            j = img[r][c+1];
            i = img[r+1][c+1];
            k = img[r+1][c];
            di = (xy[i][0]-r-row1+1)*(xy[i][0]-r-row1+1) +
                 (xy[i][1]-c-col1+1)*(xy[i][1]-c-col1+1);
            dj = (xy[j][0]-r-row1+1)*(xy[j][0]-r-row1+1) +
                 (xy[j][1]-c-col1+1)*(xy[j][1]-c-col1+1);
            dk = (xy[k][0]-r-row1+1)*(xy[k][0]-r-row1+1) +
                 (xy[k][1]-c-col1+1)*(xy[k][1]-c-col1+1);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( dk < di ) {
                di = dk;
                i = k;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
        for ( c=1; c < img.cols-1; c++ ) {
            i = img[r][c-1];
            j = img[r+1][c-1];
            k = img[r+1][c];
            di = (xy[i][0]-r-row1+1)*(xy[i][0]-r-row1+1) +
                 (xy[i][1]-c-col1+1)*(xy[i][1]-c-col1+1);
            dj = (xy[j][0]-r-row1+1)*(xy[j][0]-r-row1+1) +
                 (xy[j][1]-c-col1+1)*(xy[j][1]-c-col1+1);
            dk = (xy[k][0]-r-row1+1)*(xy[k][0]-r-row1+1) +
                 (xy[k][1]-c-col1+1)*(xy[k][1]-c-col1+1);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( dk < di ) {
                di = dk;
                i = k;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
    }
}

void read_dem_header ( image<float> & dem )
{
    char file[1000];
    GDALDataset  *df;
    GDALRasterBand *bd;
    double        trans[6];
    int rows, cols;

    strcpy ( file, p.value("dem_file").c_str());
    if ( strlen(file) == 0 ) {
        fprintf(stderr,"Need to specify a dem file\n");
        pe.send_shutdown();
        exit(1);
    }

    df = (GDALDataset *) GDALOpen( file, GA_ReadOnly );
    if( df == NULL ) {
        fprintf(stderr,"Could not open dem file: %s\n", file );
        pe.send_shutdown();
        exit(1);
    }
    rows = df->GetRasterYSize();
    cols = df->GetRasterXSize();
    dem.create(rows,cols);

    if( df->GetGeoTransform( trans ) == CE_None ) {
        dem_east = trans[0];
        dem_north = trans[3];
        dem_wid = trans[1];
    } else {
        fprintf(stderr,"Dem file: %s has no geographic metadata\n", file );
        pe.send_shutdown();
        exit(1);
    }

    delete df;
}

void read_dem ( image<float> & dem )
{
    char file[1000];
    GDALDataset  *df;
    GDALRasterBand *bd;
    double        trans[6];
    int rows, cols;

    strcpy ( file, p.value("dem_file").c_str());
    if ( strlen(file) == 0 ) {
        fprintf(stderr,"Need to specify a dem file\n");
        pe.send_shutdown();
        exit(1);
    }

    int i;

    df = (GDALDataset *) GDALOpen( file, GA_ReadOnly );
    if( df == NULL ) {
        fprintf(stderr,"Could not open dem file: %s\n", file );
        pe.send_shutdown();
        exit(1);
    }
    rows = df->GetRasterYSize();
    cols = df->GetRasterXSize();

    if( df->GetGeoTransform( trans ) == CE_None ) {
        dem_east = trans[0];
        dem_north = trans[3];
        dem_wid = trans[1];
    } else {
        fprintf(stderr,"Dem file: %s has no geographic metadata\n", file );
        pe.send_shutdown();
        exit(1);
    }

    bd = df->GetRasterBand(1);
    bd->RasterIO( GF_Read, 0, 0, cols, rows, dem.data,
            cols, rows, GDT_Float32, 0, 0 );
    delete df;
}

void compute_lens_inverse()
{
    int i, n, r, col;
    double xx, yy, d;
    double xi, yi, ei, chisq;
    ap::real_1d_array X, y, w, c;

    n = (int)sqrt(input_rows*input_rows+input_cols*input_cols)/2;

    X.setbounds(0,n);
    y.setbounds(0,n);
    w.setbounds(0,n);

    c.setbounds(0,r);

    for (i = 0; i < n; i++)
    {
        yi = i;
        xi = yi+lens[0]+lens[1]*yi+lens[2]*yi*yi+lens[3]*yi*yi*yi+lens[4]*yi*yi*yi*yi;
        X(i) = xi;
        y(i) = yi;
        w(i) = 1.0;
    }

    printf("OK\n"); 

    buildchebyshevleastsquares ( X, y, w, X(0), X(n-1), n, 7, c );

    inv_rows = (int)(input_rows * xi / (n-1) );
    inv_cols = (int)(input_cols * xi / (n-1) );
    inverse_scale.create(inv_rows, inv_cols);

    printf("inv rows %d, cols %d\n", inv_rows, inv_cols );
    for ( r = 0; r < inv_rows; r++ ) {
        yy = r - inv_rows/2 + row_offset;
        //printf("Row %d\n",r);
        for ( col = 0; col < inv_cols; col++ ) {
            xx = col - inv_cols/2 + column_offset;
            d = sqrt ( yy*yy + xx*xx );
            if ( d < 10.0 ) inverse_scale[r][col] = 1.0;
            else inverse_scale[r][col] = d/(calculatechebyshevleastsquares(7,c,d));

            if ( inverse_scale[r][col] < 0.5 ||
                 inverse_scale[r][col] > 1.5 ) printf("%d %d %5.3f \n",r,col,inverse_scale[r][col]);
            //if ( col % 10 == 0 ) printf("\n");
        }
    }
}

void map_block ( int t )
{
    int current = -1;
    int j, k, r, c, dr, dc, jr, jc;
    rgb_image rgb;
    double max_north, north, min_east, east, n1, n2, e1, e2;
    double dt, db, dx, dy, ddx, ddy, elev;
    double it, ib, ix, iy, dix, diy;
    double scale;
    int row1, row2, col1, col2;
    vec3 pt, diff;
    int i;
    int block = -1;

    k = t % mappers;

    row1 = tiles[t].first_row;
    row2 = row1 + block_y;
    if ( row2 > rows ) row2 = rows;
    col1 = tiles[t].first_col;
    col2 = col1 + block_x;
    if ( col2 > cols ) col2 = cols;

    rgb.rows = 0;
    max_north = p.dvalue("northing_top");
    min_east = p.dvalue("easting_left");
    fill(t,img[k],dist[k],row1,col1);
    spread_dist(img[k],dist[k],row1,col1);
    for ( r = row1; r < row2; r++ ) {
        if ( output_utm ) {
            north = max_north - r * wid;
        }
        for ( c = col1; c < col2; c++ ) {
            if ( output_utm ) {
                east = min_east + c*wid;
            } else {
                scale = (c%10)/10.0;
                n1 = north_trans[r/10][c/10]*(1.0-scale) +
                     north_trans[r/10][c/10+1]*scale;
                e1 = east_trans[r/10][c/10]*(1.0-scale) +
                     east_trans[r/10][c/10+1]*scale;
                n2 = north_trans[r/10+1][c/10]*(1.0-scale) +
                     north_trans[r/10+1][c/10+1]*scale;
                e2 = east_trans[r/10+1][c/10]*(1.0-scale) +
                     east_trans[r/10+1][c/10+1]*scale;
                scale = (r%10)/10.0;
                north = n1*(1.0-scale) + n2*scale;
                east = e1*(1.0-scale) + e2*scale;
            }
            dy = (dem_north - north) / dem_wid;
            dr = int(dy + 0.5);
            ddy = dy - int(dy);
            if ( dr < -10 || dr >= dem.rows+10 ) {
                //printf("dem north %g, wid %g;  north %g, dy %g\n",
                //dem_north, dem_wid, north, dy );
                fprintf(stderr,"Dem row %d out of range for row %d\n", dr, r );
                continue;
            }
            if ( dr < 0 ) dr = 0;
            if ( dr >= dem.rows ) dr = dem.rows-1;
            if ( img[k][r-row1+1][c-col1+1] < 1 ) {
                out[k].img[0][r-row1][c-col1] = 0;
                out[k].img[1][r-row1][c-col1] = 0;
                out[k].img[2][r-row1][c-col1] = 0;
                if ( alpha ) out[k].img[3][r-row1][c-col1] = 0;
                continue;
            }
            if ( img[k][r-row1+1][c-col1+1] != current ) {
                current = img[k][r-row1+1][c-col1+1];
                //printf("%d %d switching to %d \n", r, c, current );
                rgb = imgs[current];
                if ( !valid_img[current] ) {
                    pe.wait_for ( "image", current );
                    valid_img[current] = 1;
                }
            }
            dx = (east - dem_east) / dem_wid;
            dc = int(dx + 0.5);
            ddx = dx - int(dx);
            if ( dc < -10 || dc >= dem.cols+10 ) {
                fprintf(stderr,"Dem column %d out of range for column %d\n",
                        dc, c );
                continue;
            }
            if ( dc < 0 ) dc = 0;
            if ( dc >= dem.cols ) dc = dem.cols - 1;
            if ( dc == 0 || dr == 0 || dc >= dem.cols-2 || dr >= dem.rows-2 ) {
                elev = -rgb.alt + dem[dr][dc];
            } else {
                dr = (int)dy;
                dc = (int)dx;
                dt = (1.0-ddx)*dem[dr][dc] + ddx*dem[dr][dc+1];
                db = (1.0-ddx)*dem[dr+1][dc] + ddx*dem[dr+1][dc+1];
                elev = -rgb.alt + (1.0-ddy)*dt + ddy*db;
            }
            //printf("r %d c %d, dy %6.2f, dx %6.2f, elev %10.2f, alt %10.2f\n",
            //r,c, dy, dx, elev, rgb.alt );
            if ( elev < -rgb.alt+sea_level || elev >= 0.0 ) elev = -rgb.alt+sea_level;
            pt[0] = east;
            pt[1] = north;
            pt[2] = elev;
            //diff = pt - rgb.cam;
            //printf("pt %8g %12.8g %g, cam %8g %12.8g %g, zrot %g %g %g\n",
            //pt[0], pt[1], pt[2], rgb.cam[0], rgb.cam[1], rgb.cam[2],
            //rgb.zrot[0], rgb.zrot[1], rgb.zrot[2] );
            //dist = diff.length();
            //dist = diff * rgb.zrot;
            //scale = rgb.alt / dist;
            //printf("%d %d %d %d, cam %g, elev %g,  dist %gm scale %g\n",
            //r,c,dr,dc, rgb.cam.n[2], elev, dist, scale);
            //pt = scale * pt + (1-scale)*rgb.cam;
            pt = rgb.MI * pt;
            scale = -rgb.unit_depth / pt[2];
            pt = scale * pt;
            iy = rgb.rows - 1 - (pt[1]+rgb.rows/2+row_offset);
            jr = (int)iy;
            ix = pt[0]+rgb.cols/2+column_offset;
            jc = (int)ix;
            if ( jr < 0 ) jr = 0;
            if ( jc < 0 ) jc = 0;
            if ( jr >= inv_rows ) jr = inv_rows - 1;
            if ( jc >= inv_cols ) jc = inv_cols - 1;
            if ( iy > 0 && ix > 0 && iy < inv_rows-1 && ix < inv_cols-1 ) {
                diy = iy - jr;
                dix = ix - jc;
                it = (1.0-dix)*inverse_scale[jr][jc] +
                        dix * inverse_scale[jr][jc+1];
                ib = (1.0-dix)*inverse_scale[jr+1][jc] +
                        dix * inverse_scale[jr+1][jc+1];
                scale = (1.0-diy)*it + diy*ib;
                //printf("jr %d, jc %d, it %g, ib %g, dix %g, diy %g, scale %g\n",
                        //jr, jc, it, ib, dix, diy, scale );
            } else {
                jr = int(iy+0.5);
                jc = int(ix+0.5);
                if ( jr < 0 ) jr = 0;
                if ( jc < 0 ) jc = 0;
                if ( jr >= inv_rows ) jr = inv_rows - 1;
                if ( jc >= inv_cols ) jc = inv_cols - 1;
                scale = inverse_scale[jr][jc];
                //printf("jr %d, jc %d, scale %g\n", jr, jc, scale );
            }
            iy = (iy-rgb.rows/2-row_offset) * scale + rgb.rows/2+row_offset;
            ix = (ix-rgb.cols/2-column_offset) * scale + rgb.cols/2+column_offset;
            jr = int(iy);
            jc = int(ix);

            //printf("scale %g, jr %g, jc %g\n", scale, iy, ix );
            if ( jr >= 0 && jr < rgb.rows && jc >= 0 && jc < rgb.cols){
                if ( jr == 0 || jc == 0 ||
                        jr == rgb.rows - 1 || jc == rgb.cols - 1 ) {
                    out[k].img[0][r-row1][c-col1] = rgb.img[0][jr][jc];
                    out[k].img[1][r-row1][c-col1] = rgb.img[1][jr][jc];
                    out[k].img[2][r-row1][c-col1] = rgb.img[2][jr][jc];
                } else {
                    diy = iy - jr;
                    dix = ix - jc;
                    for ( j = 0; j < 3; j++ ) {
                        it = (1.0-dix)*rgb.img[j][jr][jc] +
                            dix * rgb.img[j][jr][jc+1];
                        ib = (1.0-dix)*rgb.img[j][jr+1][jc] +
                            dix * rgb.img[j][jr+1][jc+1];
                        out[k].img[j][r-row1][c-col1] = int((1.0-diy)*it +
                                diy*ib + 0.5);
                    }
                }
                if ( alpha ) out[k].img[3][r-row1][c-col1] = current&0xff;
                //out[k].img[1][r-row1][c-col1] = iy/8;
                //out[k].img[2][r-row1][c-col1] = ix/8;
            } else {
                out[k].img[0][r-row1][c-col1] = 0;
                out[k].img[1][r-row1][c-col1] = 0;
                out[k].img[2][r-row1][c-col1] = 0;
                if ( alpha ) out[k].img[3][r-row1][c-col1] = 0;
            }
        }
    }
    rgb.rows = 0;
    rgb.img[0].data = 0;
    rgb.img[1].data = 0;
    rgb.img[2].data = 0;
}

void dem_reader()
{
    read_dem(dem);
    pe.report_complete ( "dem", 0 );
}

void reader()
{
    int i;
    //printf("Reader process %d, %d\n", getpid(), pe.mypid );
    //nice(2);
    while ( (i = pe.get_task("image")) >= 0 ) {
        pe.wait_for ( "ready", i );
        imgs[i].read_image();
        pe.report_complete ( "image", i );
        //printf("Reader process %d, completed %d\n", getpid(), i );
    }
}

void mapper()
{
    int t;
    //printf("Mapper process %d, %d\n", getpid(), pe.mq );
    //nice(4);
    //pe.wait_for ( "dem", 0 );
    while ( (t = pe.get_task("mapper")) >= 0 ) {
        map_block ( t );
        pe.report_complete ( "data", t );
        pe.wait_for ( "writer", t );
    }
}

void writer()
{
    //printf("Writer process %d, %d\n", getpid(), pe.mq );
    output_geotiff();
}

bool comp_row ( const int a, const int b )
{
    return xy[a][0] < xy[b][0];
}

int main ( int argc, char **argv )
{
    int i, j, k, pk, n, pid, t, r, c, rowsx, colsx, rowst, colst;
    int shrink;
    double pix;
    int req, task, value;
    char task_name[16];
    char s[80];

    //log_start();

    printf ("\n\n%s\n\n", VERSION);


    if ( argc < 2 ) {
        fprintf(stderr,"Usage: %s param_file\n", argv[0]);
        exit(1);
    }

    set_defaults();
    p.read_file(argv[1]);

    GDALAllRegister();

    read_dem_header(dem);
    read_dem(dem);

    alpha = p.ivalue("alpha");
    east1 = p.dvalue("easting_left");
    east2 = p.dvalue("easting_right");
    north1 = p.dvalue("northing_bottom");
    north2 = p.dvalue("northing_top");
    wid = p.dvalue("output_cell_size");
    pix = p.dvalue("input_cell_size");
    input_rows = p.ivalue("input_rows");
    input_cols = p.ivalue("input_columns");
    max_dist = 0.6 * pix * sqrt(input_cols*input_cols+input_rows*input_rows) / wid;
    max_dist_2 = max_dist * max_dist;
    readers = p.ivalue("readers");
    mappers = p.ivalue("mappers");
    writers = p.ivalue("writers");
    slack = p.ivalue("slack");

//
//  Make the width of tiles be a power of 2
//
    t = p.ivalue("block_x");
    block_x = 64;
    while ( block_x < t ) block_x *= 2;
//
//  Make the height of tiles be a power of 2
//
    t = p.ivalue("block_y");
    block_y = 64;
    while ( block_y < t ) block_y *= 2;

    output_utm = (p.value("output_projection") == "utm");
    if ( output_utm ) {
//
//      Compute the size of the output file
//
        rows = int((north2 - north1) / wid);
        cols = int((east2 - east1) / wid);
    } else {
//
//  Compute the size of the output file
//
        longitude_left = p.dvalue("longitude_left");
        longitude_right = p.dvalue("longitude_right");
        latitude_bottom = p.dvalue("latitude_bottom");
        latitude_top = p.dvalue("latitude_top");
        compute_geographic_cellsize();
        rows = int((latitude_top - latitude_bottom) / cell_height);
        cols = int((longitude_right - longitude_left) / cell_width);
        compute_utm_trans();
    }

//
//  Determine the number of tiles (blocks) across, down and total.
//
    x_blocks = (cols+block_x-1)/block_x;
    y_blocks = (rows+block_y-1)/block_y;
    n_blocks = x_blocks * y_blocks;

    printf("cols %d, rows %d\n", cols, rows );

//
//  Create a tile object for each tile and fill in values
//
    tiles = new tile[n_blocks];
    for ( t = 0; t < n_blocks; t++ ) {
        tiles[t].first_row = first_row(t);
        tiles[t].first_col = first_col(t);
        tiles[t].cols = block_x;
        if ( tiles[t].first_col + block_x > cols ) {
            tiles[t].cols = cols-tiles[t].first_col;
        }
        tiles[t].rows = block_y;
        if ( tiles[t].first_row + block_x > rows ) {
            tiles[t].rows = rows-tiles[t].first_row;
        }
        //printf("tile %d, %d %d, %d %d\n", t, tiles[t].first_col, tiles[t].first_row,
                //tiles[t].cols, tiles[t].rows );
    }

    srand(time(NULL));

    lens[0] = p.dvalue("k0");
    lens[1] = p.dvalue("k1");
    lens[2] = p.dvalue("k2");
    lens[3] = p.dvalue("k3");
    lens[4] = p.dvalue("k4");

    compute_lens_inverse();
    read_image_file();

    //fill ( img, dist, 0, rows );
    //spread_dist ( img, dist );
    //if ( p.value("dist_file") != "" ) output_dist_geotiff ( dist, v );
    //if ( p.value("blob_file") != "" ) output_pcolor_geotiff ( img, dist, v );
    shrink = 1;
    while ( rows/shrink > 2000 && cols/shrink > 2000 ) shrink *= 2;
    rowsx = rows/shrink;
    colsx = cols/shrink;
    add_imgs_to_xy(shrink);

    distx.create(rowsx+2,colsx+2);
    imgx.create(rowsx+2,colsx+2);

    for ( r = 0; r < rowsx+2; r++ ) {
        for ( c = 0; c < colsx+2; c++ ) {
            imgx[r][c] = 0;
            distx[r][c] = 1000000000.0;
        }
    }

    first_needed = new int[num_imgs+1]();
    last_needed = new int[num_imgs+1]();
    img_order = new int[num_imgs+1]();
    img_free_order = new int[num_imgs+1]();
    img_replacement = new int[num_imgs+1]();
    printf("num_imgs %d\n",num_imgs);
    
    for ( i = 0; i <= num_imgs; i++ ) {
        img_order[i] = -1;
        img_free_order[i] = -1;
        img_replacement[i] = -1;
    }
    for ( i = 1; i <= num_imgs; i++ ) {
        first_needed[i] = -1;
        last_needed[i] = -1;
    }
    for ( i = 1; i <= num_imgs; i++ ) {
        r = (int)xy[i][0];
        c = (int)xy[i][1];
        if ( r < -max_dist ) continue;
        if ( c < -max_dist ) continue;
        if ( r > rowsx+max_dist ) continue;
        if ( c > colsx+max_dist ) continue;
        if ( r < 0 ) r = 0;
        if ( r >= rowsx ) r = rowsx-1;
        if ( c < 0 ) c = 0;
        if ( c >= colsx ) c = colsx-1;
        if ( imgx[r+1][c+1] == 0 ) {
            imgx[r+1][c+1] = i;
            distx[r+1][c+1] = (r-xy[i][0])*(r-xy[i][0]) + (c-xy[i][1])*(c-xy[i][1]);
        }
    }

    spread_dist(imgx,distx);

    image<unsigned char> v;
    v.create(rowsx,colsx);

    if ( p.value("dist_file") != "" ) output_dist_geotiff ( distx, v );
    if ( p.value("blob_file") != "" ) output_pcolor_geotiff ( imgx, distx, v );
    k = 0;
    for ( t = 0; t < n_blocks; t++ ) {
        rowst = (tiles[t].first_row+block_y)/shrink;
        if ( rowst >= imgx.rows ) rowst = imgx.rows-1;
        colst = (tiles[t].first_col+block_x)/shrink;
        if ( colst >= imgx.cols ) colst = imgx.cols-1;
        for ( r = tiles[t].first_row/shrink; r < rowst; r++ ) {
            for ( c = tiles[t].first_col/shrink; c < colst; c++ ) {
                j = imgx[r+1][c+1];
                if ( j == 0 ) continue;
                if ( first_needed[j] < 0 ) {
                    first_needed[j] = t;
                    last_needed[j] = t;
                    tiles[t].imgs_needed.insert(j);
                    pe.add_task ( "image", j );
                    img_order[k++] = j;
                }
                if ( last_needed[j] < t ) {
                    last_needed[j] = t;
                    tiles[t].imgs_needed.insert(j);
                }
            }
        }
        //cout << "tile " << t << ": ";
        //copy ( tiles[t].imgs_needed.begin(), tiles[t].imgs_needed.end(),
        //       ostream_iterator<int>(cout," ") );
        //cout << endl;
    }
    set<short> live, new_live;

    for ( i = 1; i <= num_imgs; i++ ) {
        if ( last_needed[i] >= 0 ) {
            tiles[last_needed[i]].imgs_to_free.insert(i);
        }
    }

    for ( i = 0; i < mappers; i++ ) {
        set_union ( live.begin(), live.end(),
                    tiles[i].imgs_needed.begin(), tiles[i].imgs_needed.end(),
                    inserter(new_live,new_live.begin()) );
        live = new_live;
        new_live.clear();
    }
    max_needed = live.size();
    for ( i = mappers; i < n_blocks; i++ ) {
        set_difference ( live.begin(), live.end(),
                    tiles[i-mappers].imgs_to_free.begin(),
                    tiles[i-mappers].imgs_to_free.end(),
                    inserter(new_live,new_live.begin()) );
        live = new_live;
        new_live.clear();
        set_union ( live.begin(), live.end(),
                    tiles[i].imgs_needed.begin(), tiles[i].imgs_needed.end(),
                    inserter(new_live,new_live.begin()) );
        live = new_live;
        new_live.clear();
        if ( max_needed < live.size() ) max_needed = live.size();
    }

    max_rgb_images = max_needed + slack;
    if ( max_rgb_images > num_imgs ) max_rgb_images = num_imgs;
    printf("Need %d images, with slack %d\n", max_needed, max_rgb_images);

    for ( i = 0; i < max_rgb_images; i++ ) {
        k = img_order[i];
        pe.complete ( "ready", k );
        if ( k > 0 ) imgs[k].create_image(input_rows,input_cols);
    }
    k = 0;
    for ( t = 0; t < n_blocks; t++ ) {
        copy ( tiles[t].imgs_to_free.begin(),
               tiles[t].imgs_to_free.end(),
               img_free_order+k );
        k += tiles[t].imgs_to_free.size();
    }
    for ( i = max_rgb_images; i < num_imgs; i++ ) {
        k = img_order[i];
        if ( k < 0 ) continue;
        pk = img_free_order[i - max_rgb_images];
        img_replacement[pk] = k;
        imgs[k].rows = input_rows;
        imgs[k].cols = input_cols;
        for ( j = 0; j < 3; j++ ) {
            imgs[k].img[j] = imgs[pk].img[j];
        }
    }

    add_imgs_to_xy();

    valid_img = new int[num_imgs+1]();

    for ( i = 0; i < readers; i++ ) {
        pe.launch ( reader );
    }

    dist = new image<float>[mappers];
    img = new image<short>[mappers];
    out = new rgb_image[mappers];
    for ( i = 0; i < mappers; i++ ) {
        dist[i].create(block_y+2,block_x+2);
        img[i].create(block_y+2,block_x+2);
        out[i].create_image(block_y,block_x);
    }

    for ( i = 0; i < n_blocks; i++ ) pe.add_task ( "mapper", i );

    for ( i = 0; i < mappers; i++ ) {
        pe.launch ( mapper );
    }
    for ( i = 0; i < writers; i++ ) {
        pe.launch ( writer );
    }

    while ( pe.children > 0 ) {
        t = pe.receive ( s, 1 );
        if ( t < 0 ) perror("master receive");
        sscanf(s,"%d %d %s %d", &req, &pid, task_name, &value );
        switch ( req ) {
            case SHUTDOWN:
                pe.must_quit = 1;
                pe.children--;
                pe.shutdown();
                break;
            case EXIT:
                fprintf (stderr, "Completed process %d\n", pid);
                //log("exit %d",pid);
                pe.children--;
                break;
            case WAIT:
                if ( pe.must_quit ) {
                    t = pe.send ( "-1", pid );
                    break;
                }
                if ( pe.ready(task_name,value) ) {
                    //printf("Sending %d task %d\n",pid,task);
                    //log("wait %d %s %d",pid,task_name,value);
                    t = pe.send ( "1", pid );
                } else {
                    //log("sleep %d %s %d",pid,task_name,value);
                    pe.enqueue ( task_name, value, pid );
                }
                break;
            case TASK:
                if ( pe.must_quit ) {
                    t = pe.send ( "-1", pid );
                    break;
                }
                task = pe.fetch_task(task_name);
                //log("task %d %s %d",pid,task_name,task);
                sprintf(s,"%d",task);
                //printf("Sending %d task %d\n",pid,task);
                t = pe.send ( s, pid );
                break;
            case REPORT:
                //log("complete %d %s %d",pid,task_name,value);
                pe.complete ( task_name, value );
                break;
        }
    }

    //dump_log();
    return 0;
}
