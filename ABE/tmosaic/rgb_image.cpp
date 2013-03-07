#include "rgb_image.h"

#include <cstdio>

bool rgb_image::read_params ( FILE *fp )
{
    char t[1000];
    strstream ss;
    int id;

    if ( !fgets ( t, 1000, fp ) ) return false;

    ss << t;

    try {
        ss >> id
           >> pathname
           >> east
           >> north
           >> alt
           >> omega
           >> phi
           >> kappa
           >> zone;
           rows = 0;
           cols = 0;
    } catch (...) {
        return false;
    }
    return true;
}

bool rgb_image::create_image ( int nrows, int ncols, int alpha )
{
    delete_image();
    this->alpha = alpha;
    img[0].create(nrows,ncols);
    img[1].create(nrows,ncols);
    img[2].create(nrows,ncols);
    if ( alpha ) img[3].create(nrows,ncols);
    rows = nrows;
    cols = ncols;
}

void rgb_image::delete_image ()
{
    int i;
    if ( rows < 1 ) return;
    rows = 0;
    cols = 0;
    for ( i = 0; i < 3; i++ ) {
        img[i].unmap();
    }
    if ( alpha ) img[3].unmap();
}

bool rgb_image::read_image()
{
    int fd;
    int i;
    unsigned char header[16];
    unsigned char jfif[] = {
        0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46,
        0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01
    };
    GDALDataset *df;
    GDALRasterBand *bd;
    int bands;

    fd = open ( pathname.c_str(), O_RDWR );
    if ( fd < 0 ) {
        fprintf(stderr,"Could not open %s to patch jpeg header\n",
                pathname.c_str() );
        return false;
    }
    read ( fd, header, 16 );
    if ( bcmp(header,jfif,4) != 0 ) {
        fprintf(stderr,"Apparently %s is not a jpeg file\n",
                pathname.c_str() );
        return false;
    }
    if ( bcmp(header,jfif,16) != 0 ) {
        lseek ( fd, (off_t)0, SEEK_SET );
        write ( fd, jfif, 16 );
    }
    close ( fd );

    df = (GDALDataset *) GDALOpen( pathname.c_str(), GA_ReadOnly );
    if( df == NULL ) {
        fprintf(stderr,"Could not open %s\n", pathname.c_str() );
        exit(1);
    }

    rows = df->GetRasterYSize();
    cols = df->GetRasterXSize();
    bands = df->GetRasterCount();
    //create_image(rows,cols);

    if ( bands < 3 ) {
        fprintf(stderr,"%s does not have 3 bands\n",
                pathname.c_str() );
        delete df;
        return false;
    }

    for ( i = 0; i < 3; i++ ) {
        bd = df->GetRasterBand(i+1);
        bd->RasterIO ( GF_Read, 0,0, cols, rows, img[i].data, cols, rows,
                       GDT_Byte, 0,0);
    }
    delete df;
}
