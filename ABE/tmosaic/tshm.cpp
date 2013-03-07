#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>


int main ( int argc, char **argv )
{
    int fd;
    long i, size;
    unsigned char *p;

    size = (argc > 1 ) ? atol(argv[1]) : 1000000;

    fd = shm_open ( "/ray_shm", O_RDWR | O_CREAT, 0666 );
    if ( fd < 0 ) {
        fprintf(stderr,"Could not open /ray_shm\n");
        exit(1);
    }

    if ( ftruncate ( fd, size ) ) {
        fprintf(stderr,"Could not size /ray_shm to %ld\n", size );
        exit(1);
    }

    p = (unsigned char *)mmap ( NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 );

    printf("p = %p\n", p);
    for ( i = 0; i < size; i++ ) {
        p[i] = i & 255;
    }

    return 0;
}
