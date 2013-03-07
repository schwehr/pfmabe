
//
// header:			UnitRays.hpp
// author:			Gary Morris
//
// This header file is strictly for storing the unit rays that will be used to populate
// the spatial neighbor table.  There is a maximum of 48 total rays that we will be 
// testing in the event of the maximum size chosen (7 x 7).  I placed them in a separate
// file to avoid clutter and to fully explain the array structure.  These values will
// be hard-coded and are of unit length.
//

#ifndef _UNIT_RAYS_HPP
#define _UNIT_RAYS_HPP

#include "nvtypes.h"

// the array indices correspond to the table in the following way
//
// --------------------------------------------------
// | 24   |  25  |  26  |  27  | 28   |  29  |  30  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  31  |  8   |  9   | 10   |  11  |  12  |  32  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  33  |  13  |  0   |  1   |  2   |  14  |  34  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  35  |  15  |  3   |  X   |  4   |  16  |  36  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  37  |  17  |  5   |  6   |  7   |  18  |  38  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  39  |  19  |  20  |  21  | 22   |  23  |  40  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------
// |  41  |  42  |  43  |  44  |  45  |  46  |  47  |
// |      |      |      |      |      |      |      |
// --------------------------------------------------



static NV_FLOAT32 unitRays[48][2] = {{-.707107f, -.707107}, {0.0f, -1.0f}, {.707107f, -.707107f},
							{-1.0f, 0.0f}, {1.0f, 0.0f}, {-.707107f, .707107f},
							{0.0f, 1.0f}, {.707107f, .707107f}, {-.707107f, -.707107f},
							{-.447214f, -.894427f}, {0.0f, -1.0f}, {.447214f, -.894427f},
							{.707107f, -.707107f}, {-.894427f, -.447214f}, {.894427f, -.447214f},
							{-1.0f, 0.0f}, {1.0f, 0.0f}, {-.894427f, .447214f},
							{.894427f, .447214f}, {-.707107f, .707107f}, {-.447214f, .894427f},
							{0.0f, 1.0f}, {.447214f, .894427f}, {.707107f, .707107f},
							{-.707107f, -.707107f}, {-.5547f, -.83205f}, {-.316228f, -.948683f},
							{0.0f, -1.0f}, {.316228f, -.948683f}, {.5547f, -.83205f},
							{.707107f, -.707107f}, {-.83205f, -.5547f}, {.83205f, -.5547f},
							{-.948683f, -.316228f}, {.948683f, -.316228f}, { -1.0f, 0.0f},
							{1.0f, 0.0f}, {-.948683f, .316228f}, {.948683f, .316228f},
							{-.83205f, .5547f}, {.83205f, .5547f}, {-.707107f, .707107f},
							{-.5547f, .83205f}, {-.316228f, .948683f}, { 0.0f, 1.0f},
							{.316228f, .948683f}, {.5547f, .83205f}, {.707107f, .707107f}};

static NV_INT16 tableMapping3x3[8] = {0, 1, 2,
										3, 4,
										5, 6, 7};

static NV_INT16 tableMapping5x5[24] = {6, 7, 8, 11, 12,
										15, 16, 17, 0, 1,
										2, 3, 4, 5, 9,
										10, 13, 14, 18, 19,
										20, 21, 22, 23};

static NV_INT16 tableMapping7x7[48] = {16, 17, 18, 23, 24, 29, 30,
										31, 8, 9, 10, 11, 12, 15,
										19, 22, 25, 28, 32, 35, 36,
										37, 38, 39, 0, 1, 2, 3, 
										4, 5, 6, 7, 13, 14, 20,
										21, 26, 27, 33, 34, 40,
										41, 42, 43, 44, 45, 46, 47};

#endif
