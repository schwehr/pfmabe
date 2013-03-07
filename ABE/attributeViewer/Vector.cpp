//
// source:		Vector.cpp
// author: 		Gary Morris
//
// This source file supports all practical applications of a three dimensional vector.
//

// include files

#include "Vector.hpp"

//
// construct:		Vector
//
// This constructor does nothing.
//

Vector::Vector(void) {} // blank contstructor

//
// construct:		Vector
//
// this constructor will initiate the x, y, and z members to the 3 incoming arguments.
//

Vector::Vector(float x, float y, float z) { 

	Set(x,y,z); 
} 

//
// construct:		Vector
//
// This constructor will set the xyz parameters to an incoming array.
//

Vector::Vector(float * v) { 

	Set(v); 
} 


//
// method:		Set
// 
// This method will set the x,y,z values to either the 3 floats coming in or the first,
// second, and third positions of an array.
//

void Vector::Set(float x, float y, float z) { this->x=x; this->y=y; this->z=z; } // set points to supplied values
void Vector::Set(float *v) { x=v[0]; y=v[1]; z=v[2]; } // set points to supplied values

//
// method:		Offset
//
// This method will add/subtract the vector by essentially another vector specified by
// the 3 arguments.
//

Vector Vector::OffSet(float x, float y, float z) { return Vector( this->x+x, this->y+y, this->z+z); }

//
// method:		GetDistance
//
// This method will take in 2 Vertices and will use the distance calculation to return
// the distance.
//

float Vector::GetDistance(Vector Vector1, Vector Vector2) {

	return sqrt( pow((Vector1.x - Vector2.x),2) + pow((Vector1.y - Vector2.y),2) + pow((Vector1.z - Vector2.z),2) );
}


//
// method:		magnitude
//
// This method will give us the magnitude or "Norm" as some say, of our normal.
// Here is the equation:  magnitude = sqrt(x^2 + y^2 + z^2)  
//

float Vector::Magnitude( ) { return (float)sqrt( (x * x) + (y * y) + (z * z) ); }

//
// method:		Normalize
// 
// This method will make the vector to be that of unit length 1.
// 

void Vector::Normalize() {

	float magnitude = Magnitude();				// Get the magnitude of our normal
	// Now that we have the magnitude, we can divide our normal by that magnitude.
	// That will make our normal a total length of 1.  This makes it easier to work with too.
	x /= magnitude;								// Divide the X value of our normal by it's magnitude
	y /= magnitude;								// Divide the Y value of our normal by it's magnitude
	z /= magnitude;								// Divide the Z value of our normal by it's magnitude
}


//
// method:		IsInfinite
//
// This method will return a true for infinity if the NaN property evaluates to true
// for either the x,y, or z member of the Vector.
//

bool Vector::IsInfinite() { 

	return ( isNaN((double)x) || isNaN((double)y) || isNaN((double)z) ); 
}

//
// method:		IsZero
//
// This method will return a true if the Vector is at the origin.  A threshold value
// is sent in for precision.
//

bool Vector::IsZero(float threshold) {

	return 
		( fabs( (double) x ) <= threshold )
		&& ( fabs( (double) y) <= threshold )
		&& ( fabs( (double) z) <= threshold );
}


//
// method:		DotProduct
//
// This method will calculate the dot product of two vectors.
//

float Vector::DotProduct(Vector V1, Vector V2) { 

	return V1.x*V2.x + V1.y*V2.y + V1.z*V2.z; 
}


//
// method:		Cross
//
// This method will calculate the cross product of 2 vectors.  If we are given 2 vectors
// (directions of 2 sides of a polygon), then we have a plane defined.  The cross product
// will find the vector that is perpendicular to that plane meangin it points straight
// out of the plane at a 90 degree angle.
//

Vector Vector::Cross(Vector V1, Vector V2) {
	
	// formula is  x= y1*z2-y2*z1 :  y = z1*x2-z2*x1 : z = x1*y2-x2*y1
	return Vector ( (V1.y * V2.z) - (V1.z * V2.y), (V1.z * V2.x) - (V1.x * V2.z), (V1.x * V2.y) - (V1.y * V2.x) );
}



//
// operator:		-
//
// This operator will negate the calling vector.
//

Vector Vector::operator - () { return Vector(-x, -y, -z); }

//
// operator:		-
//
// This operator will subtract the incoming vector from the calling vector.
//

Vector Vector::operator - (Vector V2) { return Vector(x-V2.x, y-V2.y, z-V2.z); }

//
// operator:		-=
//
// This operator will subtract the incoming vector from the calling vector.  It will then
// store the results in the calling vector.
//
void Vector::operator -= (Vector V2) { x-=V2.x; y-=V2.y; z-=V2.z; }

//
// operator:		+
//
// This operator will add the incoming vector to the calling vector.
//

Vector Vector::operator + (Vector V2) { return Vector(x+V2.x, y+V2.y, z+V2.z); }

//
// operator:		+=
//
// This operator will add the incoming vector to the calling vector.  It then stores the
// result in the calling vector.
//

void Vector::operator += (Vector V2) {x+=V2.x; y+=V2.y; z+=V2.z; }

//
// operator:		==
//
// This operator will test the equality of the 2 vectors and return a true/false for 
// equality.
//

bool Vector::operator == (Vector V2) { return (x==V2.x && y==V2.y && z == V2.z); }

//
// operator:		!=
//
// This operator will test the inequality of the 2 vectors and return a true/false for 
// inequality.
//

bool Vector::operator != (Vector V2) { return (x!=V2.x || y!=V2.y || z != V2.z); }


//
// operator:		*
//
// This operator will multiply the incoming scalar to the calling vector.
//

Vector Vector::operator * (float f) { return Vector(x*f, y*f, z*f); }

//
// operator:		*
//
// This operator will multiply the incoming vector to the calling vector.
//

Vector Vector::operator * (Vector V2) { return Vector(x*V2.x, y*V2.y, z*V2.z); }

//
// operator:		*=
//
// This operator will multiply the incoming vector to the calling vector.  It then stores
// the result into the calling vector.
//

void Vector::operator *= (Vector V2) { x*=V2.x; y*=V2.y; z*=V2.z; }

//
// operator:		*=
//
// This operator will multiply the incoming scalar to the calling vector.  It then stores
// the result into the calling vector.
//

void Vector::operator *= (float n) { x*=n; y*=n; z*=n; }

//
// operator:		/
//
// This operator will divide the calling vector by the incoming vector.
//

Vector Vector::operator / (Vector V2) { return Vector(x/V2.x, y/V2.y, z/V2.z); }

//
// operator:		/
//
// This operator will divide the calling vector by the incoming scalar.
//

Vector Vector::operator / (float f) { return Vector(x/f, y/f, z/f); }


//
// operator:		/
//
// This operator will divide the calling vector by the incoming vector.  It then stores
// the result in the calling vector.
//

void Vector::operator /= (Vector V2) { x/=V2.x; y/=V2.y; z/=V2.z; }

//
// operator:		[]
//
// This operator allows the x, y, or z member to be accessed in array notation. 
// [0] = x, [1] = y, [2] = z
//

float &Vector::operator[] ( int i ) { return (i==0)? x : (i==1)? y : z; }


//
// method:		isNaN
//
// This method is the isNaN (is not a number) method.  Not quite sure I understand what
// is going on here.
//

bool Vector::isNaN (double x) { 

	return x != x;
}


//
// method:	ClosestPtOnLine
//
// This method will ingest a line segment which are the first two arguments.  The
// third argument refers to the point of comparison.  It will return the closest
// point on the line to the point of comparison.
//

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
