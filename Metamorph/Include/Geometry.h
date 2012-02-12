//	Geometry.h
//
//	Classes for vectors and other geometric concepts
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#ifndef INCL_GEOMETRY
#define INCL_GEOMETRY

//	Scalars

typedef double Dimension;
const Dimension EPSILON = 1e-06;
const Dimension INFINITY = 1e20;
const Dimension PI = 3.14159265;

inline bool EqZero (const Dimension &x) { return (x < EPSILON && x > -EPSILON); }

enum AxisNames
	{
	axisX = 0,
	axisY = 1,
	axisZ = 2,
	};

//	2D Vectors

class C2Vector
	{
	public:
		C2Vector (void) : m_x(0.0), m_y(0.0) { }
		C2Vector (Dimension x, Dimension y) : m_x(x), m_y(y) { }

		inline const Dimension &X (void) const { return m_x; }
		inline const Dimension &Y (void) const { return m_y; }

		inline void SetX (Dimension x) { m_x = x; }
		inline void SetY (Dimension y) { m_y = y; }

	private:
		Dimension m_x;
		Dimension m_y;
	};

//	3D Vectors

class CVector
	{
	public:
		CVector (void) : m_x(0.0), m_y(0.0), m_z(0.0) { }
		CVector (Dimension x, Dimension y, Dimension z) : m_x(x), m_y(y), m_z(z) { }

		const CVector Cross (const CVector &op2) const;
		const Dimension Dot (const CVector &op2) const;
		const Dimension Length (void) const;
		inline const Dimension Length2 (void) const { return (m_x * m_x + m_y * m_y + m_z * m_z); };
		const CVector Normal (void) const;

		inline const Dimension &X (void) const { return m_x; }
		inline const Dimension &Y (void) const { return m_y; }
		inline const Dimension &Z (void) const { return m_z; }

		inline void SetX (Dimension x) { m_x = x; }
		inline void SetY (Dimension y) { m_y = y; }
		inline void SetZ (Dimension z) { m_z = z; }

	private:
		Dimension m_x;
		Dimension m_y;
		Dimension m_z;
	};

//	Vector-Vector operations
inline const CVector operator+ (const CVector &op1, const CVector &op2) { return CVector(op1.X() + op2.X(), op1.Y() + op2.Y(), op1.Z() + op2.Z()); }
inline const CVector operator- (const CVector &op1, const CVector &op2) { return CVector(op1.X() - op2.X(), op1.Y() - op2.Y(), op1.Z() - op2.Z()); }
inline const CVector operator* (const CVector &op1, const CVector &op2) { return CVector(op1.X() * op2.X(), op1.Y() * op2.Y(), op1.Z() * op2.Z()); }
inline const CVector operator- (const CVector &op) { return CVector(-op.X(), -op.Y(), -op.Z()); }

//	Vector-scalar operations
inline const CVector operator* (const CVector &op1, const Dimension op2) { return CVector(op1.X() * op2, op1.Y() * op2, op1.Z() * op2); }
inline const CVector operator* (const Dimension op2, const CVector &op1) { return CVector(op1.X() * op2, op1.Y() * op2, op1.Z() * op2); }
inline const CVector operator/ (const CVector &op1, const Dimension op2) { return CVector(op1.X() / op2, op1.Y() / op2, op1.Z() / op2); }

//	Ray

struct SRay
	{
	CVector vOrigin;				//	Origin of the ray in global coordinates
	CVector vDir;					//	Direction of ray (unit vector)
	};

//	Functions

const C2Vector OrthoProject (const CVector &vVec, int iAxis);
void PlaneEq (const SRay &Normal, 
			  Dimension *retA,
			  Dimension *retB,
			  Dimension *retC,
			  Dimension *retD);
int PlaneOrtho (const SRay &Normal);
bool PointInPolygon (const C2Vector &vVec, const TArray<C2Vector> &Polygon);
Dimension PointToLineDistance (const CVector &vPoint, const SRay &Line);
Dimension RayIntersectionWithBox (const SRay &Ray, const CVector &vNearCorner, const CVector &vFarCorner);
Dimension RayIntersectionWithPlane (const SRay &Ray, const SRay &PlaneNormal);
Dimension RayNearestPointToPoint (const SRay &Ray, const CVector &vPoint);

#endif

