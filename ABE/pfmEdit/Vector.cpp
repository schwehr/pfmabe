
/*********************************************************************************************

    This is public domain software that was developed for the U.S. Army Corps of Engineers
    in support of the Coastal Zone Mapping and Imaging LiDAR (CZMIL) project.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/*!
  - Header: Vector.cpp
  - Author: Gary Morris

  - This is the source file for Vector Class. A Vector class encapsulates x y z coords.  Also typedef'ed as Vertex
*/

// include files

#include "Vector.hpp"

Vector::Vector(void) {} // blank contstructor

Vector::Vector(float x, float y, float z) { Set(x,y,z); } // construct and set points to supplied vals

Vector::Vector(float * v) { Set(v); } // construct and set points to supplied point array


/*!
 - Function:	Set
 
 - set points to supplied values

 - Returns:		Vertex
*/

void Vector::Set(float x, float y, float z) { this->x=x; this->y=y; this->z=z; } // set points to supplied values
void Vector::Set(float *v) { x=v[0]; y=v[1]; z=v[2]; } // set points to supplied values


//! offset vector by supplied vals. add each x y z to this vector's x y z
Vector Vector::OffSet(float x, float y, float z) { return Vector( this->x+x, this->y+y, this->z+z); }

/*!
  - method:	GetDistance

  - This method will take in 2 Vertices and will return the distance between them.
*/

float Vector::GetDistance(Vector Vector1, Vector Vector2) {

	return sqrt( pow((Vector1.x - Vector2.x),2) + pow((Vector1.y - Vector2.y),2) + pow((Vector1.z - Vector2.z),2) );
}



/*!
  This will give us the magnitude or "Norm" as some say, of our normal.
  Here is the equation:  magnitude = sqrt(x^2 + y^2 + z^2)
*/
float Vector::Magnitude( ) { return (float)sqrt( (x * x) + (y * y) + (z * z) ); }


//! normalize this vector. formula is x/magnitude, y/magnitude, z/magnitude
void Vector::Normalize() {
	float magnitude = Magnitude();				// Get the magnitude of our normal
	// Now that we have the magnitude, we can divide our normal by that magnitude.
	// That will make our normal a total length of 1.  This makes it easier to work with too.
	x /= magnitude;								// Divide the X value of our normal by it's magnitude
	y /= magnitude;								// Divide the Y value of our normal by it's magnitude
	z /= magnitude;								// Divide the Z value of our normal by it's magnitude
}


//! returns true if x y or z _isnan
bool Vector::IsInfinite() { return ( isNaN((double)x) || isNaN((double)y) || isNaN((double)z) ); }

//! returns true if all values x y and z are 0
bool Vector::IsZero(float threshold) {

	return 
		( fabs( (double) x ) <= threshold )
		&& ( fabs( (double) y) <= threshold )
		&& ( fabs( (double) z) <= threshold );
}



//! calc dot product and return float val. formula is x1*x2 + y1*y2 + z1*z2
float Vector::DotProduct(Vector V1, Vector V2) { return V1.x*V2.x + V1.y*V2.y + V1.z*V2.z; }


/*!
  if we are given 2 vectors (directions of 2 sides of a polygon)
  then we have a plane defined.  The cross product finds a vector that is perpendicular
  to that plane, which means it's point straight out of the plane at a 90 degree angle.
	
  - formula is  x= y1*z2-y2*z1 :  y = z1*x2-z2*x1 : z = x1*y2-x2*y1
*/

Vector Vector::Cross(Vector V1, Vector V2) {
	return Vector ( (V1.y * V2.z) - (V1.z * V2.y), (V1.z * V2.x) - (V1.x * V2.z), (V1.x * V2.y) - (V1.y * V2.x) );
}



//! Negation operation
Vector Vector::operator - () { return Vector(-x, -y, -z); }

//! subtract on vector's (V2) coords from this
Vector Vector::operator - (Vector V2) { return Vector(x-V2.x, y-V2.y, z-V2.z); }

//! subtract on vector's (V2) coords from this
void Vector::operator -= (Vector V2) { x-=V2.x; y-=V2.y; z-=V2.z; }

//! add one vector's (V2) coords to this vector
Vector Vector::operator + (Vector V2) { return Vector(x+V2.x, y+V2.y, z+V2.z); }

//! add one vector's (V2) coords to this vector
void Vector::operator += (Vector V2) {x+=V2.x; y+=V2.y; z+=V2.z; }

//! equality operator
bool Vector::operator == (Vector V2) { return (x==V2.x && y==V2.y && z == V2.z); }

//! inequality operator
bool Vector::operator != (Vector V2) { return (x!=V2.x || y!=V2.y || z != V2.z); }


//! multiply all 3 coords by a float value
Vector Vector::operator * (float f) { return Vector(x*f, y*f, z*f); }

//! multiply this by V2
Vector Vector::operator * (Vector V2) { return Vector(x*V2.x, y*V2.y, z*V2.z); }

//! set this = this * V2
void Vector::operator *= (Vector V2) { x*=V2.x; y*=V2.y; z*=V2.z; }

//! set this = this * n
void Vector::operator *= (float n) { x*=n; y*=n; z*=n; }

//! divide this by V2
Vector Vector::operator / (Vector V2) { return Vector(x/V2.x, y/V2.y, z/V2.z); }

//! multiply all 3 coords by a float value
Vector Vector::operator / (float f) { return Vector(x/f, y/f, z/f); }


//! set this = this / V2
void Vector::operator /= (Vector V2) { x/=V2.x; y/=V2.y; z/=V2.z; }

//! return by ref x y or z denoted by [0] [1] and [anythingelse] respectively
float &Vector::operator[] ( int i ) { return (i==0)? x : (i==1)? y : z; }


bool Vector::isNaN (double x) { 

	return x != x;
}


/*!
  - method:	ClosestPtOnLine

  - This method will ingest a line segment which are the first two arguments.  The
    third argument refers to the point of comparison.  It will return the closest
    point on the line to the point of comparison.
*/

Vertex Vector::ClosestPtOnLine (Vector vA, Vector vB, Vector vPoint) {

	Vector vVector1 = vPoint - vA;
	Vector vVector2 = vB - vA;
	vVector2.Normalize();

	float d = Vector::GetDistance (vA, vB);
	float t = Vector::DotProduct (vVector2, vVector1);

	if (t <= 0) return vA;
	if (t >= d) return vB;

	Vector vVector3 = vVector2 * t;
	Vertex vClosestPoint = vA + vVector3;

	return vClosestPoint;
}
