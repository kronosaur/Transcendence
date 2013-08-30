//	Geometry.h
//
//	Transcendence Space Engine geometry

#ifndef INCL_TSE_GEOMETRY
#define INCL_TSE_GEOMETRY

class ViewportTransform
	{
	public:
		ViewportTransform (void) : m_xScale(1.0), m_yScale(1.0), m_xCenterTrans(0), m_yCenterTrans(0) { }
		ViewportTransform (const CVector &vCenter, Metric xScale, Metric yScale, int xCenter, int yCenter);
		ViewportTransform (const CVector &vCenter, Metric rScale, int xCenter, int yCenter);
		void Offset (int x, int y);
		void Transform (const CVector &vP, int *retx, int *rety) const;

	private:
		Metric m_xScale;
		Metric m_yScale;
		int m_xCenterTrans;
		int m_yCenterTrans;
	};

//	Utilities

Metric CalcDistanceToPath (const CVector &Pos, 
						   const CVector &Path1, 
						   const CVector &Path2, 
						   CVector *retvNearestPoint = NULL,
						   CVector *retvAway = NULL);
Metric CalcInterceptTime (const CVector &vTarget, const CVector &vTargetVel, Metric rMissileSpeed, Metric *retrRange = NULL);
bool IntersectLine (const CVector &vStart1, const CVector &vEnd1, const CVector &vStart2, const CVector &vEnd2, CVector *retvIntersection = NULL, Metric *retIntersectFraction = NULL);
bool IntersectRect (const CVector &vUR1, const CVector &vLL1,
					const CVector &vUR2, const CVector &vLL2);
bool IntersectRect (const CVector &vUR, const CVector &vLL, const CVector &vPoint);

#endif
