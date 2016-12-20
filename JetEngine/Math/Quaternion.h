//this file is a modified version of that found in the ogre3d engine

/*
00002 -----------------------------------------------------------------------------
00003 This source file is part of OGRE
00004     (Object-oriented Graphics Rendering Engine)
00005 For the latest info, see http://www.ogre3d.org/
00006
00007 Copyright (c) 2000-2012 Torus Knot Software Ltd
00008
00009 Permission is hereby granted, free of charge, to any person obtaining a copy
00010 of this software and associated documentation files (the "Software"), to deal
00011 in the Software without restriction, including without limitation the rights
00012 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
00013 copies of the Software, and to permit persons to whom the Software is
00014 furnished to do so, subject to the following conditions:
00015
00016 The above copyright notice and this permission notice shall be included in
00017 all copies or substantial portions of the Software.
00018
00019 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
00020 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
00021 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
00022 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
00023 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
00024 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
00025 THE SOFTWARE.
00026 -----------------------------------------------------------------------------
00027 */
#ifndef __Quaternion_H__
#define __Quaternion_H__

#include "Vector.h"
#include "Matrix.h"
#include "geom.h"
#include <memory.h>
#include <string.h>

class Quaternion
{
public:
	inline Quaternion ()
		: w(1), x(0), y(0), z(0)
	{
	}
	inline Quaternion (
		float fW,
		float fX, float fY, float fZ)
		: w(fW), x(fX), y(fY), z(fZ)
	{
	}
	inline Quaternion(const Matrix4& rot)
	{
		this->FromRotationMatrix(rot);
	}

	//Construct a quaternion from an angle/axis
	inline Quaternion(const float& rfAngle, const Vec3& rkAxis)
	{
		this->FromAngleAxis(rfAngle, rkAxis);
	}
	//Construct a quaternion from 3 orthonormal local axes
	inline Quaternion(const Vec3& xaxis, const Vec3& yaxis, const Vec3& zaxis)
	{
		this->FromAxes(xaxis, yaxis, zaxis);
	}
	//construct a quaternion from 3 orthonormal local axes
	inline Quaternion(const Vec3* akAxis)
	{
		this->FromAxes(akAxis);
	}
	/// Construct a quaternion from 4 manual x/y/z/w values
	inline Quaternion(float* valptr)
	{
		memcpy(&x, valptr, sizeof(float)*4);
	}

	/** Exchange the contents of this quaternion with another.
	*/
	inline void swap(Quaternion& other)
	{
		std::swap(w, other.w);
		std::swap(x, other.x);
		std::swap(y, other.y);
		std::swap(z, other.z);
	}

	inline float operator [] ( const size_t i ) const
	{
		//assert( i < 4 );

		return *(&w+i);
	}

	inline float& operator [] ( const size_t i )
	{
		//assert( i < 4 );

		return *(&w+i);
	}

	inline float* ptr()
	{
		return &w;
	}

	inline const float* ptr() const
	{
		return &w;
	}

	void FromRotationMatrix (const Matrix4& kRot);
	void ToRotationMatrix (Matrix4& kRot) const;
	/** Setups the quaternion using the supplied vector, and "roll" around
	that vector by the specified floats.
	*/
	void FromAngleAxis (const float& rfAngle, const Vec3& rkAxis);
	void ToAngleAxis (float& rfAngle, Vec3& rkAxis) const;
	/*inline void ToAngleAxis (float& dAngle, Vec3& rkAxis) const {
	float rAngle;
	ToAngleAxis ( rAngle, rkAxis );
	dAngle = rAngle;
	}*/
	/** Constructs the quaternion using 3 axes, the axes are assumed to be orthonormal
	@see FromAxes
	*/
	void FromAxes (const Vec3* akAxis);
	void FromAxes (const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis);
	/** Gets the 3 orthonormal axes defining the quaternion. @see FromAxes */
	void ToAxes (Vec3* akAxis) const;
	void ToAxes (Vec3& xAxis, Vec3& yAxis, Vec3& zAxis) const;

	/** Returns the X orthonormal axis defining the quaternion. Same as doing
	xAxis = Vec3::UNIT_X * this. Also called the local X-axis
	*/
	Vec3 xAxis(void) const;

	/** Returns the Y orthonormal axis defining the quaternion. Same as doing
	yAxis = Vec3::UNIT_Y * this. Also called the local Y-axis
	*/
	Vec3 yAxis(void) const;

	/** Returns the Z orthonormal axis defining the quaternion. Same as doing
	zAxis = Vec3::UNIT_Z * this. Also called the local Z-axis
	*/
	Vec3 zAxis(void) const;

	inline Quaternion& operator= (const Quaternion& rkQ)
	{
		w = rkQ.w;
		x = rkQ.x;
		y = rkQ.y;
		z = rkQ.z;
		return *this;
	}
	Quaternion operator+ (const Quaternion& rkQ) const;
	Quaternion operator- (const Quaternion& rkQ) const;
	Quaternion operator* (const Quaternion& rkQ) const;
	Quaternion operator* (float fScalar) const;
	friend Quaternion operator* (float fScalar,
		const Quaternion& rkQ);
	Quaternion operator- () const;
	inline bool operator== (const Quaternion& rhs) const
	{
		return (rhs.x == x) && (rhs.y == y) &&
			(rhs.z == z) && (rhs.w == w);
	}
	inline bool operator!= (const Quaternion& rhs) const
	{
		return !operator==(rhs);
	}
	Quaternion normalize() const { return *this * (1.0f / sqrt(Norm())); }
	// functions of a quaternion
	/// Returns the dot product of the quaternion
	float Dot (const Quaternion& rkQ) const;
	/* Returns the normal length of this quaternion.
	@note This does <b>not</b> alter any values.
	*/
	float Norm () const;
	/// Normalises this quaternion, and returns the previous length
	float normalise(void);
	Quaternion Inverse () const;  // apply to non-zero quaternion
	Quaternion UnitInverse () const;  // apply to unit-length quaternion
	Quaternion Exp () const;
	Quaternion Log () const;

	/// Rotation of a vector by a quaternion
	Vec3 operator* (const Vec3& rkVector) const;

	/** Calculate the local roll element of this quaternion.
	@param reprojectAxis By default the method returns the 'intuitive' result
	that is, if you projected the local Y of the quaternion onto the X and
	Y axes, the angle between them is returned. If set to false though, the
	result is the actual yaw that will be used to implement the quaternion,
	which is the shortest possible path to get to the same orientation and
	may involve less axial rotation.  The co-domain of the returned value is
	from -180 to 180 degrees.
	*/
	float getRoll(bool reprojectAxis = true) const;
	/** Calculate the local pitch element of this quaternion
	@param reprojectAxis By default the method returns the 'intuitive' result
	that is, if you projected the local Z of the quaternion onto the X and
	Y axes, the angle between them is returned. If set to true though, the
	result is the actual yaw that will be used to implement the quaternion,
	which is the shortest possible path to get to the same orientation and
	may involve less axial rotation.  The co-domain of the returned value is
	from -180 to 180 degrees.
	*/
	float getPitch(bool reprojectAxis = true) const;
	/** Calculate the local yaw element of this quaternion
	@param reprojectAxis By default the method returns the 'intuitive' result
	that is, if you projected the local Y of the quaternion onto the X and
	Z axes, the angle between them is returned. If set to true though, the
	result is the actual yaw that will be used to implement the quaternion,
	which is the shortest possible path to get to the same orientation and
	may involve less axial rotation. The co-domain of the returned value is
	from -180 to 180 degrees.
	*/
	float getYaw(bool reprojectAxis = true) const;
	/// Equality with tolerance (tolerance is max angle difference)
	bool equals(const Quaternion& rhs, const float& tolerance) const;

	/** Performs Spherical linear interpolation between two quaternions, and returns the result.
	Slerp ( 0.0f, A, B ) = A
	Slerp ( 1.0f, A, B ) = B
	@return Interpolated quaternion
	@remarks
	Slerp has the proprieties of performing the interpolation at constant
	velocity, and being torque-minimal (unless shortestPath=false).
	However, it's NOT commutative, which means
	Slerp ( 0.75f, A, B ) != Slerp ( 0.25f, B, A );
	therefore be careful if your code relies in the order of the operands.
	This is specially important in IK animation.
	*/
	static Quaternion Slerp (float fT, const Quaternion& rkP,
		const Quaternion& rkQ, bool shortestPath = false);

	/** @see Slerp. It adds extra "spins" (i.e. rotates several times) specified
	by parameter 'iExtraSpins' while interpolating before arriving to the
	final values
	*/
	static Quaternion SlerpExtraSpins (float fT,
		const Quaternion& rkP, const Quaternion& rkQ,
		int iExtraSpins);

	// setup for spherical quadratic interpolation
	static void Intermediate (const Quaternion& rkQ0,
		const Quaternion& rkQ1, const Quaternion& rkQ2,
		Quaternion& rka, Quaternion& rkB);

	// spherical quadratic interpolation
	static Quaternion Squad (float fT, const Quaternion& rkP,
		const Quaternion& rkA, const Quaternion& rkB,
		const Quaternion& rkQ, bool shortestPath = false);

	/** Performs Normalised linear interpolation between two quaternions, and returns the result.
	nlerp ( 0.0f, A, B ) = A
	nlerp ( 1.0f, A, B ) = B
	@remarks
	Nlerp is faster than Slerp.
	Nlerp has the proprieties of being commutative (@see Slerp;
	commutativity is desired in certain places, like IK animation), and
	being torque-minimal (unless shortestPath=false). However, it's performing
	the interpolation at non-constant velocity; sometimes this is desired,
	sometimes it is not. Having a non-constant velocity can produce a more
	natural rotation feeling without the need of tweaking the weights; however
	if your scene relies on the timing of the rotation or assumes it will point
	at a specific angle at a specific weight value, Slerp is a better choice.
	*/
	static Quaternion nlerp(float fT, const Quaternion& rkP,
		const Quaternion& rkQ, bool shortestPath = false);

	/// Cutoff for sine near zero
	static const float msEpsilon;

	// special values
	static const Quaternion ZERO;
	static const Quaternion IDENTITY;

	float x, y, z, w;
};
/** @} */
/** @} */
#endif