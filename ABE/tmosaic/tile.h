#ifndef TILE_H
#include "rgb_image.h"
#include <set>

#define TILE_H

class tile {
    public:
    int first_row, first_col;   //  Position in file
    int rows, cols;             //  Size in use
    set<short> imgs_needed;  //  Needed by just this tile
    set<short> imgs_to_free; //  Last needed by this tile
};

#endif
