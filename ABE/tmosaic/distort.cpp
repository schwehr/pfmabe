#include <cstdio>
#include <math.h>
#include <stdlib.h>

double K[] = { 0.000687854, -0.00000575118, 0.0000000432508, -0.0000000000187 };

double evaluate ( double x )
{
    return x+K[0]*pow(x,3)+K[1]*pow(x,5)+K[2]*pow(x,7)+K[3]*pow(x,9);
}

double derivative ( double x )
{
    return 1+3*K[0]*pow(x,2)+5*K[1]*pow(x,4)+7*K[2]*pow(x,6)+9*K[3]*pow(x,8);
}

double solve ( double y, double curr_x )
{
    double curr_y, diff;
    double eps = 0.0000001;

    curr_y = evaluate(curr_x);
    diff = y - curr_y;
    while ( fabs(diff) > eps ) {
        //printf("x %g, fx %g, y %g, diff %g", curr_x, curr_y, y, diff );
        //printf(", deriv %g", derivative(curr_x));
        curr_x = curr_x - (curr_y-y)/derivative(curr_x);
        //printf(", new_x %g\n",curr_x);
        curr_y = evaluate(curr_x);
        diff = y - curr_y;
        if ( curr_x > 2 ) return -1;
    }
    return curr_x;
}

double solve_squares ( double x[], int max_dist, double unit_radius )
{
    int i, max;

    max = max_dist * max_dist;

    x[0] = 0;
    for ( i = 1; i <= max; i++ ) x[i] = solve(sqrt((double)i)/unit_radius,x[i-1]);
}

int main ( int argc, char **argv )
{
    double *x, unit_radius=600;
    int i, max_dist=1000;

    for ( i = 0; i < 4; i++ ) if ( argc > i+1 ) K[i] = atof(argv[i+1]);

    x = new double[max_dist*max_dist+1];

    solve_squares ( x, max_dist, unit_radius );
    
    for ( i = 0; i <= max_dist*max_dist; i++ ) {
        printf("i %7d, dist %g, x %g", i, sqrt((double)i), x[i]*unit_radius);
        if ( i == 0 ) printf("\n");
        else printf(", diff %g\n", (x[i] - x[i-1])*unit_radius );
    }
    return 0;
}
