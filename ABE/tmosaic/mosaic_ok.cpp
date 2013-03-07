#include "image.t"
#include "params.h"
#include "rgb_image.h"
#include "algebra3.h"
#include "process_mgr.h"
#include <map>

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdarg.h>

process_env pe;
process_env *process_event::ppe = &pe;
timeval now;

int bs = 32;
int me;
int readers;
int mappers;
int writers;
int *cindex;

using namespace std;

typedef map<int,rgb_image> img_map;
img_map imgs;

double east1, east2, north1, north2, wid;
double max_dist, max_dist_2;
double dem_east, dem_north, dem_wid;
double sea_level;
int rows, cols;
int input_rows, input_cols;

double xy[10000][2];

int *img_indices;
int max_img_id=0, img_ct=0;
int *valid_img;

image<short> dem;
rgb_image out;
image<float> dist;
image<short> img;
params p;

double xcross ( double r1, double c1, double r2, double c2, double r )
{
    double rm, cm, dxy, diff;

    rm = (r1 + r2) / 2.0;
    cm = (c1 + c2) / 2.0;
    dxy = -(r2 - r1) / (c2 - c1);
    diff = r - rm;
    return cm + dxy * diff;
}

char log_data[1000000][80];
int log_ct;
int start_time;

void log_start()
{
    gettimeofday ( &now, NULL );
    start_time = now.tv_sec*1000000 + now.tv_usec;
}

void log ( char *fmt, ... )
{
    va_list ap;
    va_start ( ap, fmt );

    gettimeofday ( &now, NULL );
    sprintf(log_data[log_ct], "%12d ", now.tv_sec*1000000 + now.tv_usec - start_time );
    vsprintf(log_data[log_ct]+13,fmt,ap);
    log_ct++;
}

void dump_log()
{
    int i;
    for ( i = 0; i < log_ct; i++ ) printf("%s\n",log_data[i]);
}

void set_defaults()
{
    p.set("images_file",     "centers" );
    p.set("easting_left",     "379000.0" );
    p.set("easting_right",    "382500.0" );
    p.set("northing_top",     "2938000.0" );
    p.set("northing_bottom",  "2937000.0" );
    p.set("utm_zone",         "17" );
    p.set("output_cell_size", "5.0" );
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
    p.set("writers",          "2" );
    p.set("mappers",          "2" );
}

void add ( image<short> & img, image<float> & dist, int i, int r, int c )
{
    double d;

    d = (r-xy[i][0])*(r-xy[i][0]) + (c-xy[i][1])*(c-xy[i][1]);
    if ( d < dist[r][c] ) {
        img[r][c] = i;
        dist[r][c] = d;
    }
}

void fill ( image<short> & img, image<float> & dist, int row1, int row2 )
{
    int r, c;
    rgb_image *q;
    vec3 center(0,0,0);
    double east, north, alt;
    img_map::iterator i;

    for ( r=row1; r < row2; r++ ) {
        for ( c=0; c < img.cols; c++ ) {
            img[r][c] = 0;
            dist[r][c] = (float)max_dist;
        }
    }

    i = imgs.begin();
    while ( i != imgs.end() ) {
        q = &i->second;
        center[0] = 0;
        center[1] = 0;
        center[2] = -q->alt + sea_level;
        center = q->M * center;
        north = center.n[1];
        east = center.n[0];
        //printf("%d, %g %g %g, cam %g %12.8g, proj %g %12.8g\n",
                //q->id, q->omega, q->phi, q->kappa, q->east, q->north, east, north);
        r = int((north2 - north)/wid);
        c = int((east - east1)/wid);
        xy[q->id][0] = r;
        xy[q->id][1] = c;
        if ( r < row1 ) r = row1;
        if ( r >= row2 ) r = row2 - 1;
        if ( c < 0 ) c = 0;
        if ( c >= cols ) c = cols - 1;
	    add ( img, dist, q->id, r, c );
        i++;
    }
}


void read_image_file(image<short> & img)
{
    double camera_roll, camera_pitch, camera_yaw;
    FILE *fp;
    rgb_image rgb, *q;
    double east, north, alt;
    img_map::iterator i;
    vec3 utm(0,0,0), x(1,0,0), y(0,1,0), z(0,0,1);
    mat4 trans, roll, pitch, yaw;

    camera_roll = p.dvalue("camera_roll");
    camera_pitch = p.dvalue("camera_pitch");
    camera_yaw = p.dvalue("camera_yaw");
    fp = fopen ( p.value("images_file").c_str(), "r" );

    if ( !fp ) {
        fprintf(stderr,"Could not open images file: %s\n",
                p.value("images_file").c_str() );
        exit(1);
    }

    xy[0][0] = -2*img.rows;
    xy[0][1] = -2*img.cols;

    while ( rgb.read_params(fp) ) {
        if ( rgb.id > max_img_id ) max_img_id = rgb.id;
        imgs[rgb.id] = rgb;
    }
    fclose(fp);

    img_indices = new int[max_img_id+1];

    sea_level = p.dvalue("sea_level");
    i = imgs.begin();
    while ( i != imgs.end() ) {
        q = &i->second;
        q->create_image(input_rows,input_cols);
        //printf("%s\n", q->pathname.c_str());
        //printf("%d %10.2f %10.2f %7.2f %7.2f %7.2f %7.2f\n",p->id,
                //p->east, p->north, p->alt,
                //p->omega, p->phi, p->kappa );
        img_indices[img_ct] = q->id;
        img_ct++;
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
        q->MI = q->M.inverse();
        q->unit_depth = p.dvalue("focal_length")*1000.0/p.dvalue("pixel_size");
        q->cam = utm;
        q->cam[2] = 0;
        i++;
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

    options = CSLSetNameValue ( options, "TILED", "NO" );
    options = CSLSetNameValue ( options, "COMPRESS", "LZW" );

    gt = GetGDALDriverManager()->GetDriverByName("GTiff");
    if ( !gt ) {
        fprintf(stderr,"Could not get GTiff driver\n");
        exit(1);
    }

    strcpy ( file, p.value("blob_file").c_str() );
    df = gt->Create( file, dist.cols, dist.rows, 1, GDT_Byte, options );
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
    ref.SetWellKnownGeogCS ( "NAD27" );
    ref.exportToWkt ( &wkt );
    df->SetProjection(wkt);
    CPLFree ( wkt );

    for ( r=0; r < dist.rows; r++ ) {
        for ( c=0; c < dist.cols; c++ ) {
            if ( dist[r][c] == 0 ) v[r][c] = 0;
            else v[r][c] = img[r][c] % 256;
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
    df = gt->Create( file, dist.cols, dist.rows, 1, GDT_Byte, options );
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
    ref.SetWellKnownGeogCS ( "NAD27" );
    ref.exportToWkt ( &wkt );
    df->SetProjection(wkt);
    CPLFree ( wkt );

    for ( r=0; r < dist.rows; r++ ) {
        for ( c=0; c < dist.cols; c++ ) {
            val = int(sqrt(dist[r][c])+0.5);
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

void output_geotiff ( rgb_image & out )
{
    int i, r, c;
    int val;
    char s[2];
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
    int          bands[] = { 1, 2, 3 };
    char         file[1000];
    int          block, ir, rows;

    options = CSLSetNameValue ( options, "TILED", "NO" );
    options = CSLSetNameValue ( options, "BLOCKXSIZE", "256" );
    options = CSLSetNameValue ( options, "BLOCKYSIZE", "256" );
    options = CSLSetNameValue ( options, "COMPRESS", "LZW" );

    gt = GetGDALDriverManager()->GetDriverByName("GTiff");
    if ( !gt ) {
        fprintf(stderr,"Could not get GTiff driver\n");
        exit(1);
    }

    strcpy ( file, p.value("output_file").c_str() );
    df = gt->Create( file, out.cols, out.rows, 3, GDT_Byte, options );
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
    ref.SetWellKnownGeogCS ( "NAD27" );
    ref.exportToWkt ( &wkt );
    df->SetProjection(wkt);
    CPLFree ( wkt );

    for ( ir = 0; ir < out.rows; ir += bs ) {
        rows = out.rows - ir;
        if ( rows > bs ) rows = bs;
        //printf("Writer waiting for %d\n",ir );
        pe.wait_for("data",ir);
        for ( i = 0; i < 3; i++ ) {
            bd = df->GetRasterBand(i+1);
            bd->RasterIO( GF_Write, 0, ir, out.cols, rows,
                          out.img[i].data+ir*out.cols,
                          out.cols, rows, GDT_Byte, 0, 0 );
        }
    }

    delete df;

    df = (GDALDataset *)GDALOpen ( file, GA_Update );
    if( df == NULL ) {
        fprintf(stderr,"Could not open for update %s\n", file );
        exit(1);
    }
    nov = p.ivalue("overviews");
    if ( nov > 5 ) nov = 5;
    if ( nov > 0 ) {
        df->BuildOverviews("NEAREST", nov, ov, 3, bands, NULL, NULL );
    }

}

void spread_dist ( image<short> & img, image<float> & dist, int row1, int row2 )
{
    int r, c, i, j, k;
    double di, dj, dk;
    double d;
    double c1, c2;
    
//
//  Start by fixing the first and last rows
//
    //r = row1;
    //c = 0;
//
//  Find the first center on row1
//
    //while ( c < img.cols && img[r][c] == 0 ) c++;

//
//  If there is no center on row1, there is nothing to patch.
//
    //if ( c < img.cols ) {
        //i = img[r][c];
        //while ( c < img.cols && img[r][c] == 0 ) c++;
        //if ( c < img.cols ) {
            //j = img[r][c];
            //while ( c < img.cols ) {
            //}
        //}
    //}


    r = row1;
    for ( c=1; c < img.cols; c++ ) {
        i = img[r][c-1];
        di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
        if ( di < dist[r][c] ) {
            dist[r][c] = di;
            img[r][c] = i;
        }
    }
    c = 0;
    for ( r=row1+1; r < row2; r++ ) {
        i = img[r-1][c];
        di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
        if ( di < dist[r][c] ) {
            dist[r][c] = di;
            img[r][c] = i;
        }
    }
    for ( r=row1+1; r < row2; r++ ) {
        for ( c=1; c < img.cols; c++ ) {
            i = img[r][c-1];
            j = img[r-1][c-1];
            k = img[r-1][c];
            di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
            dj = (xy[j][0]-r)*(xy[j][0]-r) + (xy[j][1]-c)*(xy[j][1]-c);
            dk = (xy[k][0]-r)*(xy[k][0]-r) + (xy[k][1]-c)*(xy[k][1]-c);
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
        for ( c=img.cols-2; c >= 0; c-- ) {
            j = img[r][c+1];
            i = img[r-1][c+1];
            di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
            dj = (xy[j][0]-r)*(xy[j][0]-r) + (xy[j][1]-c)*(xy[j][1]-c);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
    }

    r = row2-1;
    for ( c=img.cols-2; c >= 0; c-- ) {
        i = img[r][c+1];
        di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
        if ( di < dist[r][c] ) {
            dist[r][c] = di;
            img[r][c] = i;
        }
    }
    c = img.cols-1;
    for ( r=row2-2; r >= row1; r-- ) {
        i = img[r+1][c];
        di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
        if ( di < dist[r][c] ) {
            dist[r][c] = di;
            img[r][c] = i;
        }
    }
    for ( r=row2-2; r >= row1; r-- ) {
        for ( c=img.cols-2; c >= 0; c-- ) {
            j = img[r][c+1];
            i = img[r+1][c+1];
            k = img[r+1][c];
            di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
            dj = (xy[j][0]-r)*(xy[j][0]-r) + (xy[j][1]-c)*(xy[j][1]-c);
            dk = (xy[k][0]-r)*(xy[k][0]-r) + (xy[k][1]-c)*(xy[k][1]-c);
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
        for ( c=1; c < img.cols; c++ ) {
            i = img[r][c-1];
            j = img[r+1][c-1];
            di = (xy[i][0]-r)*(xy[i][0]-r) + (xy[i][1]-c)*(xy[i][1]-c);
            dj = (xy[j][0]-r)*(xy[j][0]-r) + (xy[j][1]-c)*(xy[j][1]-c);
            if ( dj < di ) {
                di = dj;
                i = j;
            }
            if ( di < dist[r][c] ) {
                dist[r][c] = di;
                img[r][c] = i;
            }
        }
    }
}

void read_dem_header ( image<short> & dem )
{
    char file[1000];
    GDALDataset  *df;
    GDALRasterBand *bd;
    double        trans[6];
    int rows, cols;

    strcpy ( file, p.value("dem_file").c_str());
    if ( strlen(file) == 0 ) {
        fprintf(stderr,"Need to specify a dem file\n");
        exit(1);
    }

    df = (GDALDataset *) GDALOpen( file, GA_ReadOnly );
    if( df == NULL ) {
        fprintf(stderr,"Could not open dem file: %s\n", file );
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
        exit(1);
    }

    delete df;
}

void read_dem ( image<short> & dem )
{
    char file[1000];
    GDALDataset  *df;
    GDALRasterBand *bd;
    double        trans[6];
    int rows, cols;

    strcpy ( file, p.value("dem_file").c_str());
    if ( strlen(file) == 0 ) {
        fprintf(stderr,"Need to specify a dem file\n");
        exit(1);
    }

    int i;

    df = (GDALDataset *) GDALOpen( file, GA_ReadOnly );
    if( df == NULL ) {
        fprintf(stderr,"Could not open dem file: %s\n", file );
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
        exit(1);
    }

    bd = df->GetRasterBand(1);
    bd->RasterIO( GF_Read, 0, 0, cols, rows, dem.data,
                  cols, rows, GDT_Int16, 0, 0 );
    delete df;
}

void map_block ( int row1 )
{
    int current = -1;
    int j, r, c, dr, dc, jr, jc;
    int n=0;
    rgb_image rgb, *q;
    double max_north, north, min_east, east;
    double dt, db, dx, dy, ddx, ddy, elev;
    double it, ib, ix, iy, dix, diy;
    double scale;
    double row_offset, column_offset;
    char s[2];
    int br, bc, brlim, bclim;
    vec3 pt, diff;
    img_map::iterator i;
    int block = -1;

    rgb.rows = 0;
    row_offset = p.dvalue("row_offset");
    column_offset = p.dvalue("column_offset");
    max_north = p.dvalue("northing_top");
    min_east = p.dvalue("easting_left");
    r = row1;
    brlim = r + bs;
    if ( brlim > img.rows ) brlim = img.rows;
    fill(img,dist,r,brlim);
    //printf("%d filled\n",getpid());
    spread_dist(img,dist,r,brlim);
    //printf("%d spread\n",getpid());
    for ( c = 0; c < img.cols; c += bs ) {
        for ( br = r; br < brlim; br++ ) {
            bclim = c + bs;
            if ( bclim > img.cols ) bclim = img.cols;
            north = max_north - br * wid;
            dy = (dem_north - north) / dem_wid;
            dr = int(dy + 0.5);
            ddy = dy - int(dy);
            if ( dr < 0 || dr >= dem.rows ) {
                //printf("dem north %g, wid %g;  north %g, dy %g\n",
                        //dem_north, dem_wid, north, dy );
                fprintf(stderr,"Dem row %d out of range for row %d\n", dr, r );
                exit(1);
            }
            for ( bc = c; bc < bclim; bc++ ) {
                if ( img[br][bc] < 1 ) continue;
                east = min_east + bc*wid;
                if ( img[br][bc] != current ) {
                    current = img[br][bc];
                    //printf("%d %d switching to %d after %d pixels\n",
                            //br, bc, current, n );
                    n = 1;
                    rgb = imgs[current];
                    if ( !valid_img[current] ) {
                        pe.wait_for ( "image", current );
                        valid_img[current] = 1;
                    }
                    rgb.used = 1;
                    imgs[current] = rgb;
                }
                n++;
                dx = (east - dem_east) / dem_wid;
                dc = int(dx + 0.5);
                ddx = dx - int(dx);
                if ( dc < 0 || dc >= dem.cols ) {
                    fprintf(stderr,"Dem column %d out of range for column %d\n",
                        dc, c );
                    exit(1);
                }
                if ( dc == 0 || dr == 0 ||
                     dc == dem.cols-1 || dr == dem.rows-1 ) {
                    elev = -rgb.alt + dem[dr][dc];
                } else {
                    dr = (int)dy;
                    dc = (int)dx;
                    dt = (1.0-ddx)*dem[dr][dc] + ddx*dem[dr][dc+1];
                    db = (1.0-ddx)*dem[dr+1][dc] + ddx*dem[dr+1][dc+1];
                    elev = -rgb.alt + (1.0-ddy)*dt + ddy*db;
                }
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
                        //br,bc,dr,dc, rgb.cam.n[2], elev, dist, scale);
                //pt = scale * pt + (1-scale)*rgb.cam;
                pt = rgb.MI * pt;
                scale = -rgb.unit_depth / pt[2];
                pt = scale * pt;
                iy = rgb.rows - 1 - (pt[1]+rgb.rows/2+row_offset);
                jr = int(iy+0.5);
                ix = int(pt[0]+rgb.cols/2+column_offset+0.5);
                jc = int(ix+0.5);
                //printf("scale %g, jr %g, jc %g, %g\n",
                        //scale, pt[1]+600, pt[0]+800, pt[2] );
                if ( jr >= 0 && jr < rgb.rows && jc >= 0 && jc < rgb.cols){
                    if ( jr == 0 || jc == 0 ||
                         jr == rgb.rows - 1 || jc == rgb.cols - 1 ) {
                        out.img[0][br][bc] = rgb.img[0][jr][jc];
                        out.img[1][br][bc] = rgb.img[1][jr][jc];
                        out.img[2][br][bc] = rgb.img[2][jr][jc];
                     } else {
                        jr = int(iy);
                        jc = int(ix);
                        diy = iy - jr;
                        dix = ix - jc;
                        for ( j = 0; j < 3; j++ ) {
                            it = (1.0-dix)*rgb.img[j][jr][jc] +
                                 dix * rgb.img[j][jr][jc+1];
                            ib = (1.0-dix)*rgb.img[j][jr+1][jc] +
                                 dix * rgb.img[j][jr+1][jc+1];
                            out.img[j][br][bc] = int((1.0-diy)*it +
                                 diy*ib + 0.5);
                        }
                    }
                }
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
    //printf("Reader process %d, %d\n", getpid(), pe.mq );
    nice(19);
    while ( (i = pe.get_task("image")) >= 0 ) {
        imgs[i].read_image();
        pe.report_complete ( "image", i );
        //printf("Reader process %d, completed %d\n", getpid(), i );
    }
}

void mapper()
{
    int row1;
    //printf("Mapper process %d, %d\n", getpid(), pe.mq );
    nice(10);
    pe.wait_for ( "dem", 0 );
    while ( (row1 = pe.get_task("mapper")) >= 0 ) {
        //printf("Mapper process %d, starting %d\n", getpid(), row1 );
        map_block ( row1 );
        pe.report_complete ( "data", row1 );
        //printf("Mapper process %d, completed %d\n", getpid(), row1 );
    }
}

void writer()
{
    //printf("Writer process %d, %d\n", getpid(), pe.mq );
    output_geotiff ( out );
}

bool comp_row ( const int a, const int b )
{
    return xy[a][0] < xy[b][0];
}

int main ( int argc, char **argv )
{
    int i, j, n, pid, t;
    double pix;
    double ir, ic;
    int req, task, value;
    char task_name[16];

    log_start();
    if ( argc < 2 ) {
        fprintf(stderr,"Usage: %s param_file\n", argv[0]);
        exit(1);
    }

    pe.mq = msgget ( getpid(), 0777|IPC_CREAT );

    set_defaults();
    p.read_file(argv[1]);

    GDALAllRegister();

    east1 = p.dvalue("easting_left");
    east2 = p.dvalue("easting_right");
    north1 = p.dvalue("northing_bottom");
    north2 = p.dvalue("northing_top");
    wid = p.dvalue("output_cell_size");
    pix = p.dvalue("input_cell_size");
    ir = p.dvalue("input_rows");
    ic = p.dvalue("input_columns");
    input_rows = (int)ir;
    input_cols = (int)ic;
    max_dist = 1.2 * pix * sqrt(ir*ir+ic*ic) / wid;
    max_dist_2 = max_dist/2;
    max_dist = max_dist * max_dist;
    readers = p.ivalue("readers");
    mappers = p.ivalue("mappers");
    writers = p.ivalue("writers");

    rows = int((north2 - north1) / wid);
    cols = int((east2 - east1) / wid);

    cindex = new int[cols];

    dist.create(rows,cols);
    img.create(rows,cols);

    srand(time(NULL));

    read_image_file(img);
    fill ( img, dist, 0, rows );
    //spread_dist ( img, dist );
    //if ( p.value("dist_file") != "" ) output_dist_geotiff ( dist, v );
    //if ( p.value("blob_file") != "" ) output_pcolor_geotiff ( img, dist, v );
    sort ( img_indices, img_indices+img_ct, comp_row );

    valid_img = new int[max_img_id];
    for ( i = 0; i < img_ct; i++ ) {
        j = img_indices[i];
        valid_img[j] = 0;
        if ( xy[j][0]+max_dist_2 >= 0 && xy[j][0]-max_dist_2 < rows ) {
            pe.add_task ( "image", j );
        }
    }

    read_dem_header(dem);
    pe.launch ( dem_reader );
    out.create_image(rows,cols);

    for ( i = 0; i < out.rows; i += bs ) pe.add_task ( "mapper", i );

    for ( i = 0; i < readers; i++ ) {
        pe.launch ( reader );
    }

    for ( i = 0; i < mappers; i++ ) {
        pe.launch ( mapper );
    }
    for ( i = 0; i < writers; i++ ) {
        pe.launch ( writer );
    }

    while ( pe.children > 0 ) {
        t = msgrcv ( pe.mq, &pe.msg, MSG_SIZE, 1, 0 );
        if ( t < 0 ) perror("");
        sscanf(pe.msg.s,"%d %d %s %d", &req, &pid, task_name, &value );
        switch ( req ) {
            case EXIT:
		log("exit %d",pid);
                pe.children--;
		break;
            case WAIT:
                if ( pe.ready(task_name,value) ) {
                    sprintf(pe.msg.s,"1");
                    pe.msg.t = pid;
                    //printf("Sending %d task %d\n",pid,task);
		    log("wait %d %s %d",pid,task_name,value);
                    t = msgsnd ( pe.mq, &pe.msg, MSG_SIZE, 0 );
                } else {
		    log("sleep %d %s %d",pid,task_name,value);
                    pe.enqueue ( task_name, value, pid );
                }
                break;
            case TASK:
                task = pe.fetch_task(task_name);
		log("task %d %s %d",pid,task_name,task);
                sprintf(pe.msg.s,"%d",task);
                pe.msg.t = pid;
                //printf("Sending %d task %d\n",pid,task);
                t = msgsnd ( pe.mq, &pe.msg, MSG_SIZE, 0 );
                break;
            case REPORT:
		log("complete %d %s %d",pid,task_name,value);
                pe.complete ( task_name, value );
                break;
        }
    }

    msgctl(pe.mq,IPC_RMID,NULL);

    dump_log();
    return 0;
}
