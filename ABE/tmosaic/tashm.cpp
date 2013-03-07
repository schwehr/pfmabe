#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>


int main ( int argc, char **argv )
{
    long i, size;
    unsigned char *p;
    int np, me, pid;

    size = (argc > 1 ) ? atol(argv[1]) : 1000000;
    np = (argc > 2 ) ? atoi(argv[2]) : 2;

    p = (unsigned char *)mmap ( NULL, size, PROT_READ|PROT_WRITE,
                                MAP_SHARED|MAP_ANONYMOUS, -1, 0 );

    printf("p = %p\n", p);
    me = -1;
    for ( i = 0; i < np; i++ ) {
        pid = fork();
        if ( pid == 0 ) {
            me = i;
            break;
        } else if ( pid < 0 ) {
            fprintf(stderr,"Could not fork for %d\n",i);
            exit(1);
        }
    }
    if ( me == -1 ) {
        for ( i = 0; i < np; i++ ) {
            wait(NULL);
        }
    } else {
        for ( i = me; i < size; i += np ) {
            p[i] = i & 255;
        }
    }
    return 0;
}
