#ifndef MATRIX_HEADER
#define MATRIX_HEADER

#include "Vector.h"

struct Matrix3x4;

class Matrix4f
{
public:
	explicit Matrix4f(const Matrix3x4& ma);

	Matrix4f();
	Matrix4f( bool bZero );
	Matrix4f( const Matrix4f& Matrix );
	Matrix4f( float fM11, float fM12, float fM13, float fM14,
		float fM21, float fM22, float fM23, float fM24,
		float fM31, float fM32, float fM33, float fM34,
		float fM41, float fM42, float fM43, float fM44 );
	Matrix4f Inverse() const;

	Matrix4f lookAt(
		Vec3 const & eye,
		Vec3 const & center,
		Vec3 const & up)
	{
		Vec3 zaxis = (center - eye).getnormal();
		Vec3 xaxis = up.cross(zaxis).getnormal();
		Vec3 yaxis = zaxis.cross(xaxis);

		//Vec3 f = (center - eye).getnormal();
		//Vec3 u = up.getnormal();
		//Vec3 s = f.cross(u).getnormal();//cross(f, u);
		//u = s.cross(f);//cross(s, f);

		Matrix4f Result;
		Result[0] = xaxis.x;//s.x;
		Result[4] = xaxis.y;//s.y;
		Result[8] = xaxis.z;//s.z;
		Result[1] = yaxis.x;//u.x;
		Result[5] = yaxis.y;//u.y;
		Result[9] = yaxis.z;//u.z;
		Result[2] = zaxis.x;//-f.x;
		Result[6] = zaxis.y;//-f.y;
		Result[10] = zaxis.z;//-f.z;
		Result[12] = -xaxis.dot(eye);//-s.dot(eye);//dot(s, eye);
		Result[13] = -yaxis.dot(eye);//-u.dot(eye);//dot(u, eye);
		Result[14] = -zaxis.dot(eye);//f.dot(eye);//dot(f, eye);
		return Result;
	}

	void RotationX( float fRadians );
	void RotationY( float fRadians );
	void RotationZ( float fRadians );
	void Scale( float fScale );
	void Translate( float fX, float fY, float fZ );
	void Translate( const Vec3 v);

	Vec3 GetBasisX() const;
	Vec3 GetBasisY() const;
	Vec3 GetBasisZ() const;

	Vec3 GetTranslation() const;
	//Matrix3f GetRotation() const;

	//void SetRotation( const Matrix3f& Rot );
	void SetTranslation( const Vec3& Trans );

	Matrix4f Perspective( float fovy, float aspect, float near2, float far2 )
	{
		float w,h,Q;

		h = tan(fovy*0.5f);
		w = 1.0f/(h*aspect);
		h = 1.0f/h;
		Q = far2/(far2 - near2);
		//float top = near2 * tanf( fovy / 2.0f );
		//float right = top * aspect;
		Matrix4f ret;
		//memset(&ret, 0, sizeof(ret));
		ret.MakeIdentity();

		ret[0] = w;
		ret[5] = h;
		ret[10] = Q;
		ret[14] = -Q*near2;
		ret[11] = 1.0f;
		return ret;

		//return Frustum( -right, right, -top, top, near2, far2 );
	}
	void RotationAxis(const Vec3& v, float angle)
	{
		Matrix4f* pout = this;

		//v.normalize();//D3DXVec3Normalize(&v,pv);
		this->MakeIdentity();//D3DXMatrixIdentity(pout);
		pout->m_afEntry[0] = (1.0f - cos(angle)) * v.x * v.x + cos(angle);
		pout->m_afEntry[4] = (1.0f - cos(angle)) * v.x * v.y - sin(angle) * v.z;
		pout->m_afEntry[8] = (1.0f - cos(angle)) * v.x * v.z + sin(angle) * v.y;
		pout->m_afEntry[1] = (1.0f - cos(angle)) * v.y * v.x + sin(angle) * v.z;
		pout->m_afEntry[5] = (1.0f - cos(angle)) * v.y * v.y + cos(angle);
		pout->m_afEntry[9] = (1.0f - cos(angle)) * v.y * v.z - sin(angle) * v.x;
		pout->m_afEntry[2] = (1.0f - cos(angle)) * v.z * v.x - sin(angle) * v.y;
		pout->m_afEntry[6] = (1.0f - cos(angle)) * v.z * v.y + sin(angle) * v.x;
		pout->m_afEntry[10] = (1.0f - cos(angle)) * v.z * v.z + cos(angle);
		//return pout;
	}

	void rotateEulerPYR(float angle_x, float angle_y, float angle_z)//does X*Y*Z essentially
	{
		float A = cos(angle_x);
		float B = sin(angle_x);
		float C = cos(angle_y);
		float D = sin(angle_y);
		float E = cos(angle_z);
		float F = sin(angle_z);

		float AD = A * D;
		float BD = B * D;

		m_afEntry[0]  =   C * E;
		m_afEntry[1]  =  -C * F;
		m_afEntry[2]  =  -D;
		m_afEntry[4]  = -BD * E + A * F;
		m_afEntry[5]  =  BD * F + A * E;
		m_afEntry[6]  =  -B * C;
		m_afEntry[8]  =  AD * E + B * F;
		m_afEntry[9]  = -AD * F + B * E;
		m_afEntry[10] =   A * C;

		m_afEntry[3] = m_afEntry[7] = m_afEntry[11] = m_afEntry[12] = m_afEntry[13] = m_afEntry[14] = 0;
		m_afEntry[15] = 1;
	}

	const Matrix4f createRotation(const Vec3& pos, const Vec3& lookat, const Vec3& up)
	{
		Vec3 vz = lookat - pos;
		vz.normalize();
		Vec3 vx = up.cross(vz);//Vec3::cross(Vec3( 0, 1, 0 ), vz);
		vx.normalize();
		Vec3 vy = vz.cross(vx);//Vec3::cross(vz, vx);

		Matrix4f rotation (  vx.x,   vy.x,   vz.x,   0,
			vx.y,   vy.y,   vz.y,   0,
			vx.z,   vy.z,   vz.z,   0,
			0,      0,      0,      1);
		return rotation;
	}

	static Matrix4f RotationMatrixX( float fRadians );
	static Matrix4f RotationMatrixY( float fRadians );
	static Matrix4f RotationMatrixZ( float fRadians );
	static Matrix4f ScaleMatrix( float fScale );
	static Matrix4f ScaleMatrix( const Vec3& scale );
	static Matrix4f ScaleMatrixXYZ( float fX, float fY, float fZ );
	static Matrix4f TranslationMatrix( float fX, float fY, float fZ );
	static Matrix4f TranslationMatrix( Vec3 pos );
	static Matrix4f LookAtLHMatrix( Vec3& eye, Vec3& at, Vec3& up );
	static Matrix4f LookAtRHMatrix( Vec3 eye, Vec3 at, Vec3 up );
	static Matrix4f PerspectiveFovLHMatrix( float fovy, float aspect, float zn, float zf );
	static Matrix4f PerspectiveFovRHMatrix( float fovy, float aspect, float zn, float zf );
	static Matrix4f PerspectiveFovLHMatrixOpenGL( float fovy, float aspect, float zn, float zf );
	static Matrix4f PerspectiveFovRHMatrixOpenGL( float fovy, float aspect, float zn, float zf );

	static Matrix4f BuildMatrix( Vec3& x, Vec3& y, Vec3& z );

	static Matrix4f OrthographicLHMatrix( float zn, float zf, float width, float height );
	static Matrix4f OrthographicOffCenterLHMatrix( float left, float right, float bottom, float top, float near, float far);

	void MakeZero();
	void MakeIdentity();
	void MakeTranspose();

	static Matrix4f Zero();
	static Matrix4f Identity();
	const Matrix4f Transpose() const;

	// Operators
	Matrix4f& operator= ( const Matrix4f& Matrix );

	// member access
	float operator() ( int iRow, int iCol ) const;
	float& operator() ( int iRow, int iCol );
	float operator[] ( int iPos ) const;
	float& operator[] ( int iPos );

	void SetRow( int iRow, const Vec4& Vector );
	void SetRow( int iRow, const Vec3& Vector );
	Vec4 GetRow( int iRow ) const;
	void SetColumn( int iCol, const Vec4& Vector );
	Vec4 GetColumn( int iCol ) const;

	// comparison
	bool operator== ( const Matrix4f& Matrix ) const;
	bool operator!= ( const Matrix4f& Matrix ) const;

	// arithmetic operations
	Matrix4f operator+ ( const Matrix4f& Matrix ) const;
	Matrix4f operator- ( const Matrix4f& Matrix ) const;
	Matrix4f operator* ( const Matrix4f& Matrix ) const;
	Matrix4f operator* ( float fScalar ) const;
	Matrix4f operator/ ( float fScalar ) const;
	Matrix4f operator- () const;

	// arithmetic updates
	Matrix4f& operator+= ( const Matrix4f& Matrix );
	Matrix4f& operator-= ( const Matrix4f& Matrix );
	Matrix4f& operator*= ( const Matrix4f& Matrix );
	Matrix4f& operator*= ( float fScalar );
	Matrix4f& operator/= ( float fScalar );

	// matrix - vector operations
	Vec4 operator* ( const Vec4& V ) const;  // M * v
	Vec3 operator* ( const Vec3& V ) const;

	union
	{
		float m_afEntry[4*4];
		float m[4][4];
		float _m44[4][4];
	};
};

#endif