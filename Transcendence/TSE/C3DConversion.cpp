//	C3DConversion.cpp
//
//	C3DConversion class

#include "PreComp.h"

//static Metric g_rViewAngle =					0.4636448f;
static Metric g_rViewAngle =					0.27925268;		//	16 degree angle
static Metric g_rK1 =							sin(g_rViewAngle);
static Metric g_rK2 =							cos(g_rViewAngle);

#define BRING_TO_FRONT_ATTRIB					CONSTLIT("bringToFront")
#define SEND_TO_BACK_ATTRIB						CONSTLIT("sendToBack")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")
#define Z_ATTRIB								CONSTLIT("z")

void C3DConversion::CalcCoord (int iScale, int iAngle, int iRadius, int iZ, int *retx, int *rety)

//	CalcCoord
//
//	Calculates the coordinate

	{
	Metric rScale = (Metric)iScale;
	CVector vPos = PolarToVector(iAngle, (Metric)iRadius);

	Metric rX = vPos.GetX() / rScale;
	Metric rY = vPos.GetY() / rScale;
	Metric rZ = -(Metric)iZ / rScale;

	//	Convert to global coordinates (which we align on the camera,
	//	in effect we rotate the object to align to the camera).

	Metric rXg = rX;
	Metric rYg = rY * g_rK2 - rZ * g_rK1;
	Metric rZg = rY * g_rK1 + rZ * g_rK2;

	rZg += 2.0f;

	Metric rD = rScale * 2.0f;

	//	Now convert to projection coordinates

	Metric rDen = rZg / rD;
	Metric rXp = rXg / rDen;
	Metric rYp = rYg / rDen;

	//	Done

	*retx = (int)rXp;
	*rety = -(int)rYp;
	}

void C3DConversion::CalcCoord (int iScale, int iAngle, int iRadius, int iZ, CVector *retvPos)

//	CalcCoord
//
//	Calculates the coordinate (inputs are in pixels; output is in kilometers)

	{
	Metric rScale = (Metric)iScale;
	CVector vPos = PolarToVector(iAngle, (Metric)iRadius);

	Metric rX = vPos.GetX() / rScale;
	Metric rY = vPos.GetY() / rScale;
	Metric rZ = -(Metric)iZ / rScale;

	//	Convert to global coordinates (which we align on the camera,
	//	in effect we rotate the object to align to the camera).

	Metric rXg = rX;
	Metric rYg = rY * g_rK2 - rZ * g_rK1;
	Metric rZg = rY * g_rK1 + rZ * g_rK2;

	rZg += 2.0f;

	Metric rD = rScale * 2.0f;

	//	Now convert to projection coordinates

	Metric rDen = rZg / rD;
	Metric rXp = g_KlicksPerPixel * rXg / rDen;
	Metric rYp = g_KlicksPerPixel * rYg / rDen;

	//	Done

	retvPos->SetX(rXp);
	retvPos->SetY(rYp);
	}

void C3DConversion::CalcCoordCompatible (int iAngle, int iRadius, int *retx, int *rety)

//	CalcCoordCompatible
//
//	Old method for position

	{
	CVector vPos = PolarToVector(iAngle, (Metric)iRadius);
	*retx = (int)vPos.GetX();
	*rety = -(int)vPos.GetY();
	}

ALERROR C3DConversion::Init (CXMLElement *pDesc, int iDirectionCount, int iScale)

//	Init
//
//	Initializes from an XML element. We accept the following forms:
//
//	x="nnn" y="nnn"				-> old-style 2D mapping of points
//	x="nnn" y="nnn" z="nnn"		-> use the 3D transformation

	{
	ALERROR error;

	//	Get the position

	int x = pDesc->GetAttributeInteger(X_ATTRIB);
	int y = -pDesc->GetAttributeInteger(Y_ATTRIB);

	//	Convert to polar coordinates

	int iRadius;
	int iAngle = IntVectorToPolar(x, y, &iRadius);

	//	If we have a z attribute then user the new 3D conversion.

	int z;
	if (pDesc->FindAttributeInteger(Z_ATTRIB, &z))
		Init(iDirectionCount, iScale, iAngle, iRadius, z);

	//	Otherwise use the compatible method

	else
		InitCompatible(iDirectionCount, iAngle, iRadius);

	//	Read the sendToBack and bringToFront attributes

	CString sAttrib;
	if (pDesc->FindAttribute(BRING_TO_FRONT_ATTRIB, &sAttrib))
		{
		if (error = OverridePaintFirst(sAttrib, false))
			return error;
		}

	if (pDesc->FindAttribute(SEND_TO_BACK_ATTRIB, &sAttrib))
		{
		if (error = OverridePaintFirst(sAttrib, true))
			return error;
		}

	//	Done

	return NOERROR;
	}

void C3DConversion::Init (int iDirectionCount, int iScale, int iAngle, int iRadius, int iZ)

//	Init
//
//	Initializes the conversion cache. This function may be called multiple times
//	as long as the parameters are the same. If you want different parameters,
//	call CleanUp first.

	{
	int i;

	ASSERT(iScale > 0);

	if (!IsEmpty())
		return;

	m_Cache.InsertEmpty(iDirectionCount);
	for (i = 0; i < iDirectionCount; i++)
		{
		int iRotAngle = (i * 360 / iDirectionCount);
		int iDir = Angle2Direction(iRotAngle, iDirectionCount);

		CalcCoord(iScale, 
				iRotAngle + iAngle, 
				iRadius, 
				iZ, 
				&m_Cache[iDir].x,
				&m_Cache[iDir].y);

		m_Cache[iDir].bPaintFirst = !DirectionFacesUp(iDir, iDirectionCount);
		}
	}

void C3DConversion::InitCompatible (int iDirectionCount, int iAngle, int iRadius)

//	InitCompatible
//
//	Initializes the conversion cache with the old-style 2D rotation of points.
//	[Used for backwards compatibility with old mods.]

	{
	int i;

	if (!IsEmpty())
		return;

	m_Cache.InsertEmpty(iDirectionCount);
	for (i = 0; i < iDirectionCount; i++)
		{
		int iRotAngle = (i * 360 / iDirectionCount);
		int iDir = Angle2Direction(iRotAngle, iDirectionCount);

		CVector vPos = PolarToVector(iRotAngle + iAngle, (Metric)iRadius);
		m_Cache[iDir].x = (int)vPos.GetX();
		m_Cache[iDir].y = -(int)vPos.GetY();

		m_Cache[iDir].bPaintFirst = !DirectionFacesUp(iDir, iDirectionCount);
		}
	}

void C3DConversion::InitCompatibleXY (int iDirectionCount, int iX, int iY)

//	InitCompatibleXY
//
//	Initializes the conversion cache with the old-style 2D rotation of points.
//	[Used for backwards compatibility with old mods.]

	{
	//	Convert to polar coordinates

	int iRadius;
	int iAngle = IntVectorToPolar(iX, iY, &iRadius);

	//	Init the normal way

	InitCompatible(iDirectionCount, iAngle, iRadius);
	}

void C3DConversion::InitXY (int iDirectionCount, int iScale, int iX, int iY, int iZ)

//	InitXY
//
//	Initializes the conversion cache from cartessian coordinates

	{
	//	Convert to polar coordinates

	int iRadius;
	int iAngle = IntVectorToPolar(iX, iY, &iRadius);

	//	Init the normal way

	Init(iDirectionCount, iScale, iAngle, iRadius, iZ);
	}

void C3DConversion::GetCoord (int iRotation, int *retx, int *rety)

//	GetCoord
//
//	Gets the coordinates of the given point

	{
	if (IsEmpty())
		{
		*retx = 0;
		*rety = 0;
		return;
		}

	SEntry *pEntry = &m_Cache[Angle2Direction(iRotation, m_Cache.GetCount())];
	*retx = pEntry->x;
	*rety = pEntry->y;
	}

void C3DConversion::GetCoordFromDir (int iDirection, int *retx, int *rety)

//	GetCoordFromDir
//
//	Gets the coordinates of the given point

	{
	if (IsEmpty())
		{
		*retx = 0;
		*rety = 0;
		return;
		}

	SEntry *pEntry = &m_Cache[iDirection];
	*retx = pEntry->x;
	*rety = pEntry->y;
	}

ALERROR C3DConversion::OverridePaintFirst (const CString &sAttrib, bool bPaintFirstValue)

//	OverridePaintFirst
//
//	Reads a list of integers to override bPaintFirst value.

	{
	ALERROR error;
	int i;

	if (strEquals(sAttrib, CONSTLIT("*")))
		{
		for (i = 0; i < m_Cache.GetCount(); i++)
			m_Cache[i].bPaintFirst = bPaintFirstValue;
		}
	else
		{
		CIntArray List;
		if (error = ParseAttributeIntegerList(sAttrib, &List))
			return error;

		for (i = 0; i < List.GetCount(); i++)
			{
			int iDir = List.GetElement(i);
			if (iDir >= 0 && iDir < m_Cache.GetCount())
				m_Cache[iDir].bPaintFirst = bPaintFirstValue;
			}
		}

	return NOERROR;
	}
