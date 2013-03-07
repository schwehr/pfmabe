#include <cstdio>
#include <stdlib.h>

double xcross ( double r1, double c1, double r2, double c2, double r )
{
    double rm, cm, dxy, diff;

    rm = (r1 + r2) / 2.0;
    cm = (c1 + c2) / 2.0;
    dxy = -(r2 - r1) / (c2 - c1);
    diff = r - rm;
    return cm + dxy * diff;
}

int main ( int argc, char **argv )
{
    int r1, r2, c1, c2, r;

    r1 = argc > 1 ? atoi(argv[1]) : 0;
    c1 = argc > 2 ? atoi(argv[2]) : 10;
    r2 = argc > 3 ? atoi(argv[3]) : 0;
    c2 = argc > 4 ? atoi(argv[4]) : 20;
    r  = argc > 5 ? atoi(argv[5]) : 20;

    printf("%10.2f\n", xcross(r1,c1,r2,c2,r) );
    return 0;
}
