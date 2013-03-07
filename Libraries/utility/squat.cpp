/*! <pre>

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                     *
 *                       SIMPLE QUATERNION LIBRARY                     *
 *                       version 1.1.0                                 *
 *                       by Daniel Green                               *
 *                       Mindscape, Inc.                               *
 *                       January 1996                                  *
 *                                                                     *
 * DESCRIPTION                                                         *
 *    Implements a quaternion class called "SQuat".                    *
 *    A quaternion represents a rotation about an axis.                *
 *    Squats can be concatenated together via the '*' and '*='         *
 *    operators.                                                       *
 *                                                                     *
 *    SQuats operate in a left handed coordinate system (i.e. positive *
 *    Z is coming out of the screen). The direction of a rotation by   *
 *    a positive value follows the right-hand-rule meaning that if the *
 *    thumb of your right hand is pointing along the axis of rotation, *
 *    the rotation is in the direction that your fingers curl.         *
 *                                                                     *
 *    Simple 3D vector and matrix classes are also defined mostly as   *
 *    ways to get 3D data into and out of quaternions. Methods for     *
 *    matrix multiplication, inversion, etc. are purposely not         *
 *    implemented because they are fully covered in vec.h the vector   *
 *    macro library which will work with these vector and matrix       *
 *    classes and with any other objects that can be indexed into.     *
 *                                                                     *
 *    The GetAxis and GetRotation methods extract the current axis and *
 *    rotation in radians.  A rotation matrix can also be extracted    *
 *    via the Matrix3 constructor which takes a SQuat argument.        *
 *    A SQuat can also be initialized from a rotation matrix and       *
 *    thereby extract its axis of rotation and amount.                 *
 *                                                                     *
 *    The GetHomoAxis and GetHomoRoation methods return the raw values *
 *    stored in a SQuat which have no useful geometric interpretation  *
 *    and are probably only useful to the application programmer       *
 *    wanting to archive and restore SQuats as quickly as possible.    *
 *    When reconstructing a SQuat from homogenous values, use the      *
 *    five-value constructor and pass a non-zero "homo-vals" flag.     *
 *    Otherwise, that flag should always be allowed to default to      *
 *    zero.                                                            *
 *                                                                     *
 * BUGS                                                                *
 *    Constructing from a matrix is only partially tested.             *
 *                                                                     *
 * MODIFIED                                                            *
 *    10/22/08                                                         *
 *    Jan Depner                                                       *
 *    Added Get1DMatrix4 to go from a Matrix3 structure to a 1D, 4x4   *
 *    matrix to be used in OpenGL.                                     *
 *    Added Vector3::Normalize because Normal didn't do what it should *
 *    (I think ;-)                                                     *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
</pre>*/

#include "squat.hpp"

#define X 0
#define Y 1
#define Z 2


Vector3::Vector3()
{
	//ZEROVEC3(elements);
}

Vector3::Vector3(float x, float y, float z)
{
	elements[X] = x;
	elements[Y] = y;
	elements[Z] = z;
}

float Vector3::Length() const
{
	return sqrt(NORMSQRD3(elements));
}

Vector3 Vector3::Normal() const
{
	Vector3 norm(*this);
	float len = Length ();
        VDS3(norm, norm, len);
	return norm;
}

void Vector3::Normalize()
  {
    float fMag = ( elements[0] * elements[0] + elements[1] * elements[1] + elements[2] * elements[2] );
    if (fMag == 0) {return;}

    float fMult = 1.0f/sqrt(fMag);            
    elements[0] *= fMult;
    elements[1] *= fMult;
    elements[2] *= fMult;
    return;
  }

void SQuat::InitFromVector(const Vector3 &a, float r)
{
	Vector3 normvec(a.Normal());
	float half_angle = r * 0.5;
	float sin_half_angle = sin(half_angle);
	rot = cos(half_angle);
	VXS3(axis, normvec, sin_half_angle);
}

SQuat::SQuat(const Vector3 &a, float r, int homo_vals)
{
	if(homo_vals) {
		axis = a;
		rot = r;
	}
	else
		InitFromVector(a, r);
}

SQuat::SQuat(float x, float y, float z, float r, int homo_vals)
{
	if(homo_vals) {
		rot = r;
		axis = Vector3(x, y, z);
	}
	else
		InitFromVector(Vector3(x, y, z), r);
}


SQuat::SQuat(const SQuat &q) : axis(q.axis), rot(q.rot)
{
}

SQuat::SQuat(const class Matrix3 &m)
{
	float trace = TRACE3(m);
#ifdef PREMULT
	if(trace > 0.0) {
		float s = sqrt(trace + 1.0);
		rot = s * 0.5;
		s = 0.5 / s;
		const Vector3 a(
			m[2][1] - m[1][2],
			m[0][2] - m[2][0],
			m[1][0] - m[0][1]);
		VXS3(axis, a, s);
	}
	else {
		int i, j, k;
		i = 0;
		if(m[1][1] > m[0][0])
			i = 1;
		if(m[2][2] > m[i][i])
			i = 2;
		j = (i+1) % 3;
		k = (i+2) % 3;
		float s = sqrt(m[i][i] - m[j][j] - m[k][k] + 1.0);
		axis[i] = s * 0.5;
		s = 0.5 / s;
		rot = s * (m[k][j] - m[j][k]);
		axis[j] = s * m[j][i] + m[i][j];
		axis[k] = s * m[k][i] + m[i][k];
	}
#else
	if(trace > 0.0) {
		float s = sqrt(trace + 1.0);
		rot = s * 0.5;
		s = 0.5 / s;
		const Vector3 a(
			m[1][2] - m[2][1],
			m[2][0] - m[0][2],
			m[0][1] - m[1][0]);
		VXS3(axis, a, s);
	}
	else {
		int i, j, k;
		i = 0;
		if(m[1][1] > m[0][0])
			i = 1;
		if(m[2][2] > m[i][i])
			i = 2;
		j = (i+1) % 3;
		k = (i+2) % 3;
		float s = sqrt(m[i][i] - m[j][j] - m[k][k] + 1.0);
		axis[i] = s * 0.5;
		s = 0.5 / s;
		rot = s * (m[j][k] - m[k][j]);
		axis[j] = s * m[i][j] + m[j][i];
		axis[k] = s * m[i][k] + m[k][i];
	}
#endif
}


SQuat SQuat::operator * (const SQuat &q) const
{
	Vector3 v1, v2, scaled_sum, cross, new_axis;
	VXS3(v1, q.axis,   rot);
	VXS3(v2,   axis, q.rot);
	VPV3(scaled_sum, v1, v2); // scaled_sum = rot*q.axis + q.rot*axis
#ifdef PREMULT
	VXV3(cross, axis, q.axis);
#else
	VXV3(cross, q.axis, axis);
#endif
	VPV3(new_axis, scaled_sum, cross); // new_axis = scaled_sum + cross
	return SQuat(new_axis, rot*q.rot - DOT3(axis, q.axis), 1);
}


SQuat& SQuat::operator *= (const SQuat &q)
{
	return *this = *this * q;
}

float SQuat::GetRotation() const
{
	return 2.0 * acos(rot);
}

void SQuat::GetAxis(Vector3 &v) const
{
	float theta = GetRotation();
	float f = 1.0 / sin(theta * 0.5);
	VXS3(v, axis, f);
}

Vector3 SQuat::GetAxis() const
{
	Vector3 vec;
	GetAxis(vec);
	return vec;
}

float SQuat::GetHomoRotation() const
{
	return rot;
}

void SQuat::GetHomoAxis(Vector3 &v) const
{
	SET3(v, axis);
}

Matrix3::Matrix3()
{
	IDENTMAT3(rows);
}

Matrix3::Matrix3(const Matrix3 &from)
{
	SET3(rows, from.rows);
}

Matrix3::Matrix3(const Vector3 &x, const Vector3 &y, const Vector3 &z)
{
	rows[X] = x;
	rows[Y] = y;
	rows[Z] = z;
}

Matrix3::Matrix3(const SQuat &q)
{
	float W = q.GetHomoRotation();
	Vector3 axis;
	q.GetHomoAxis(axis);

	float XX = axis[X] + axis[X];
	float YY = axis[Y] + axis[Y];
	float ZZ = axis[Z] + axis[Z];

	float WXX = W * XX;
	float XXX = axis[X] * XX;

	float WYY = W * YY;
	float XYY = axis[X] * YY;
	float YYY = axis[Y] * YY;

	float WZZ = W * ZZ;
	float XZZ = axis[X] * ZZ;
	float YZZ = axis[Y] * ZZ;
	float ZZZ = axis[Z] * ZZ;

#ifdef PREMULT
	rows[X][X] = 1.0 - (YYY + ZZZ);
	rows[X][Y] = XYY - WZZ;
	rows[X][Z] = XZZ + WYY;

	rows[Y][X] = XYY + WZZ;
	rows[Y][Y] = 1.0 - (XXX + ZZZ);
	rows[Y][Z] = YZZ - WXX;

	rows[Z][X] = XZZ - WYY;
	rows[Z][Y] = YZZ + WXX;
	rows[Z][Z] = 1.0 - (XXX + YYY);
#else
	rows[X][X] = 1.0 - (YYY + ZZZ);
	rows[Y][X] = XYY - WZZ;
	rows[Z][X] = XZZ + WYY;

	rows[X][Y] = XYY + WZZ;
	rows[Y][Y] = 1.0 - (XXX + ZZZ);
	rows[Z][Y] = YZZ - WXX;

	rows[X][Z] = XZZ - WYY;
	rows[Y][Z] = YZZ + WXX;
	rows[Z][Z] = 1.0 - (XXX + YYY);
#endif
}


//  Get a 4x4 matrix in 1D for OpenGL to use.  The last row and column are set to identity.

void Matrix3::Get1DMatrix4 (float *m4)
{
  int k = 0;
  for (int i = 0 ; i < 3 ; i++)
    {
      for (int j = 0 ; j < 3 ; j++)
        {
          m4[k] = rows[i][j];
          k++;
        }
      m4[k] = 0.0;
      k++;
    }
  m4[12] = 0.0;
  m4[13] = 0.0;
  m4[14] = 0.0;
  m4[15] = 1.0;
}
