//
// Header: Vector.h
// Author: Gary Morris
//
// This is the header file for Vector Class. The cpp file is Vector.cpp
// A Vector class encapsulates x y z coords.
// Also typedef'ed as Vertex
//

#pragma once

// include files

#include <math.h>


//
// class:		Vector
//
// This class represents a vector which is simply a 3 dimensional grouping.  It
// can be a point, a direction, a velocity, etc.  
//

class Vector {
public:
	float x, y, z;		// the coordinates
	Vector(void);			// blank constructor
	Vector (float x, float y, float z);		// constuct and Set
	Vector (float * v);		// constuct and Set

	void Set( float x, float y, float z); // set points to supplied values
	void Set( float *v) ; // set points to supplied point array;

	Vector OffSet( float x, float y, float z);

	float Magnitude( );
	
	void Normalize();
	
	bool IsInfinite();
	bool IsZero(float threshold=.0f);

	static float GetDistance(Vector Vector1, Vector Vector2);
	
	static float DotProduct(Vector Vector1, Vector Vector2);
	
	static Vector Cross(Vector Vector1, Vector Vector2);

	static Vector ClosestPtOnLine (Vector vA, Vector vB, Vector vPoint);

	Vector operator - ();
	Vector operator - (Vector Vector2);
	void operator -= (Vector Vector2);
	Vector operator + (Vector Vector2);
	void operator += (Vector Vector2);
	bool operator == (Vector Vector2);
	bool operator != (Vector Vector2);
	Vector operator * (float f);
	Vector operator * (Vector V2);
	void operator *= (Vector V2);
	void operator *= (float n);
	Vector operator / (Vector V2);
	Vector operator / (float f);
	void operator /= (Vector V2);
	float &operator[] ( int i );

private:

	bool isNaN(double x);
};



typedef Vector Vertex; 
typedef Vector Rotations;
