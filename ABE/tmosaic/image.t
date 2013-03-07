#ifndef IMAGE_T
#define IMAGE_T

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

template<class T>
class image
{
    public:
        T *data;
        long size;
        int rows;
        int cols;
        int colors;
        T * operator [] (unsigned r) { return data + r*cols; }
        void unmap ();
        void create ( int r, int c );
        image ( int r, int c );
        image ();
        ~image();
};

template<class T>
image<T>::image()
{
    rows = 0;
    cols = 0;
    data = 0;
    size = 0;
}

template<class T>
image<T>::image(int r, int c)
{
    create(r,c);
}

template<class T>
image<T>::~image()
{
    // unmap();  // Not safe with re-used data
}
    
template<class T>
void image<T>::unmap()
{
    if ( data ) {
        munmap(data,size);
        data = 0;
    }
}

template<class T>
void image<T>::create(int r, int c)
{
    rows = r;
    cols = c;
    size = sizeof(T);
    size = size * rows * cols;
    data = (T *)mmap ( NULL, size, PROT_READ|PROT_WRITE,
                                MAP_SHARED|MAP_ANONYMOUS, -1, 0 );
}
#endif
