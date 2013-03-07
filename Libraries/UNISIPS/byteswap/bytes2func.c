#include "bytes2func.h"

typedef union
{
    unsigned char bytes[8];
    long num;
} slong;

typedef union
{
    unsigned char bytes[4];
    int num;
} sint;

typedef union
{
    unsigned char  bytes[2];
    short num;
} sshort;

typedef union
{
    unsigned char  bytes[4];
    float num;
} sfloat;

typedef union
{
    unsigned char   bytes[8];
    double num;
} sdouble;

long bytes2long(unsigned char bytes[8], int swap)
{
    slong swapl;
    int i;

    if(swap)
    {
        for(i=0;i<8;i++)
        {
            swapl.bytes[7-i] = bytes[i];
        }
    }
    else
    {
        for(i=0;i<8;i++)
        {
            swapl.bytes[i] = bytes[i];
        }
    }

    return(swapl.num);
}

int bytes2int(unsigned char bytes[4], int swap)
{
    sint swapi;
    int i;

    if(swap)
    {
        for(i=0;i<4;i++)
        {
            swapi.bytes[3-i] = bytes[i];
        }
    }
    else
    {
        for(i=0;i<4;i++)
        {
            swapi.bytes[i] = bytes[i];
        }
    }

    return(swapi.num);
}

short bytes2short(unsigned char bytes[2], int swap)
{
    sshort swaps;
    int i;

    if (swap)
    {
        for(i=0;i<2;i++)
        {
            swaps.bytes[1-i] = bytes[i];
        }
    }
    else
    {
        for(i=0;i<2;i++)
        {
            swaps.bytes[i] = bytes[i];
        }
    }

    return(swaps.num);
}

float bytes2float(unsigned char bytes[4], int swap)
{
    sfloat swapf;
    int i;

    if (swap)
    {
        for(i=0;i<4;i++)
        {
            swapf.bytes[3-i] = bytes[i];
        }
    }
    else
    {
        for(i=0;i<4;i++)
        {
            swapf.bytes[i] = bytes[i];
        }
    }

    return(swapf.num);
}

double bytes2double(unsigned char bytes[8], int swap)
{
    sdouble swapd;
    int i;

    if (swap)
    {
        for(i=0;i<8;i++)
        {
            swapd.bytes[7-i] = bytes[i];
        }
    }
    else
    {
        for(i=0;i<8;i++)
        {
            swapd.bytes[i] = bytes[i];
        }
    }

    return(swapd.num);
}

int endian()
{
    sint number;

    number.num = 0xFF000000;

    if (number.bytes[0])
        return (ENDIAN_BIG);
    else
        return (ENDIAN_LITTLE);
}
