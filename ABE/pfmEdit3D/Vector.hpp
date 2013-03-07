
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

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
  - Header: Vector.h
  - Author: Gary Morris

  - This is the header file for Vector Class. The cpp file is Vector.cpp.  A Vector class encapsulates x y z coords.
    Also typedef'ed as Vertex.
*/
#pragma once

// used for sqrt function
#include <math.h>

class Vector {
public:
	float x, y, z;		//!< the coordinates
	Vector(void);			//!< blank constructor
	Vector (float x, float y, float z);		//!< constuct and Set
	Vector (float * v);		//!< constuct and Set

	void Set( float x, float y, float z); //!< set points to supplied values
	void Set( float *v) ; //!< set points to supplied point array;

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
