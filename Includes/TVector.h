//	TVector.h
//
//	Templetized vector class

#ifndef INCL_TVECTOR
#define INCL_TVECTOR

#ifndef _INC_MATH
#include <math.h>
#endif

template <class Scalar> class TVector2D
	{
	public:
		TVector2D (void) : x(0), y(0) { }
		TVector2D (Scalar ix, Scalar iy) : x(ix), y(iy) { }

		inline Scalar Dot (const TVector2D<Scalar> &vA) const { return x * vA.x + y * vA.y; }
		inline const Scalar &GetX (void) const { return x; }
		inline const Scalar &GetY (void) const { return y; }
		inline Scalar Length (void) const { return sqrt(x * x + y * y); }
		inline void SetX (Scalar nx) { x = nx; }
		inline void SetY (Scalar ny) { y = ny; }

	private:
		Scalar x;
		Scalar y;
	};

template <class Scalar> inline const TVector2D<Scalar> operator+ (const TVector2D<Scalar> &op1, const TVector2D<Scalar> &op2)
	{
	return TVector2D<Scalar> (op1.GetX() + op2.GetX(), op1.GetY() + op2.GetY());
	}

#endif