//	C3DConversion.cpp
//
//	C3DConversion class

#include "PreComp.h"

static Metric g_rViewAngle =					0.4636448f;		//	26.56 degrees (z=12 x=6)
static Metric g_rK1 =							sin(g_rViewAngle);
static Metric g_rK2 =							cos(g_rViewAngle);
static Metric g_MinZg =							0.1;

#define BRING_TO_FRONT_ATTRIB					CONSTLIT("bringToFront")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define POS_Z_ATTRIB							CONSTLIT("posZ")
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

	rZg = Max(g_MinZg, rZg + 2.0f);

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

	rZg = Max(g_MinZg, rZg + 2.0f);

	Metric rD = rScale * 2.0f;

	//	Now convert to projection coordinates

	Metric rDen = rZg / rD;
	Metric rXp = g_KlicksPerPixel * rXg / rDen;
	Metric rYp = g_KlicksPerPixel * rYg / rDen;

	//	Done

	retvPos->SetX(rXp);
	retvPos->SetY(rYp);
	}

void C3DConversion::CalcCoord (Metric rScale, const CVector &vPos, Metric rPosZ, CVector *retvPos)

//	CalcCoord
//
//	Calculates the coordinate

	{
	Metric rX = vPos.GetX() / rScale;
	Metric rY = vPos.GetY() / rScale;
	Metric rZ = -rPosZ / rScale;

	//	Convert to global coordinates (which we align on the camera,
	//	in effect we rotate the object to align to the camera).

	Metric rXg = rX;
	Metric rYg = rY * g_rK2 - rZ * g_rK1;
	Metric rZg = rY * g_rK1 + rZ * g_rK2;

	rZg = Max(g_MinZg, rZg + 2.0f);

	Metric rD = rScale * 2.0f;

	//	Now convert to projection coordinates

	Metric rDen = rZg / rD;
	Metric rXp = rXg / rDen;
	Metric rYp = rYg / rDen;

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

ALERROR C3DConversion::Init (CXMLElement *pDesc)

//	Init
//
//	Initializes from an XML element. We accept the following forms:
//
//	x="nnn" y="nnn"				-> old-style 2D mapping of points
//	x="nnn" y="nnn" z="nnn"		-> use the 3D transformation
//
//	OR
//
//	posAngle="nnn"	posRadius="nnn"	posZ="nnn"
//
//	NOTE: Must call InitComplete to finish initialization

	{
	//	Initialize based on which of the formats we've got. If we have posAngle
	//	then we have polar coordinates.

	if (pDesc->FindAttributeInteger(POS_ANGLE_ATTRIB, &m_iAngle))
		{
		m_iRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
		m_iZ = pDesc->GetAttributeInteger(POS_Z_ATTRIB);
		m_bUseCompatible = false;
		}

	//	Otherwise, we expect Cartessian coordinates

	else
		{
		//	Get the position

		int x = pDesc->GetAttributeInteger(X_ATTRIB);
		int y = -pDesc->GetAttributeInteger(Y_ATTRIB);

		//	Convert to polar coordinates

		m_iAngle = IntVectorToPolar(x, y, &m_iRadius);

		//	If we have a z attribute then user the new 3D conversion.

		m_bUseCompatible = !pDesc->FindAttributeInteger(Z_ATTRIB, &m_iZ);
		if (m_bUseCompatible)
			m_iZ = 0;
		}

	//	Read the sendToBack and bringToFront attributes

	m_sBringToFront = pDesc->GetAttribute(BRING_TO_FRONT_ATTRIB);
	m_sSendToBack = pDesc->GetAttribute(SEND_TO_BACK_ATTRIB);

	//	Done

	return NOERROR;
	}

ALERROR C3DConversion::Init (CXMLElement *pDesc, int iDirectionCount, int iScale, int iFacing)

//	Init
//
//	Initializes from an XML element. We accept the following forms:
//
//	x="nnn" y="nnn"				-> old-style 2D mapping of points
//	x="nnn" y="nnn" z="nnn"		-> use the 3D transformation
//
//	OR
//
//	posAngle="nnn"	posRadius="nnn"	posZ="nnn"

	{
	ALERROR error;

	//	First load the data

	if (error = Init(pDesc))
		return error;

	//	Finish initialization

	InitComplete(iDirectionCount, iScale, iFacing);

	//	Done

	return NOERROR;
	}

void C3DConversion::Init (int iDirectionCount, int iScale, int iAngle, int iRadius, int iZ, int iFacing)

//	Init
//
//	Initializes the conversion cache. This function may be called multiple times
//	as long as the parameters are the same. If you want different parameters,
//	call CleanUp first.

	{
	int i;

	ASSERT(iScale > 0);

	if (!InitCache(iDirectionCount))
		return;

	for (i = 0; i < iDirectionCount; i++)
		{
		int iRotAngle = (i * 360 / iDirectionCount);
		int iDir = Angle2Direction(iRotAngle, iDirectionCount);
		int iDestAngle = AngleMod(iRotAngle + iAngle);
		int iFacingAngle = AngleMod(iRotAngle + iFacing);

		CalcCoord(iScale, 
				iDestAngle, 
				iRadius, 
				iZ, 
				&m_Cache[iDir].x,
				&m_Cache[iDir].y);

		m_Cache[iDir].bPaintFirst = (iFacingAngle >= 0 && iFacingAngle <= 180);
		}
	}

bool C3DConversion::InitCache (int iDirectionCount)

//	InitCache
//
//	Prepares the cache for initialization. Returns FALSE if the cache is already
//	set up for this number of facings.

	{
	if (m_Cache.GetCount() == iDirectionCount)
		return false;

	if (m_Cache.GetCount() > 0)
		m_Cache.DeleteAll();

	m_Cache.InsertEmpty(iDirectionCount);
	return true;
	}

void C3DConversion::InitCompatible (int iDirectionCount, int iAngle, int iRadius, int iFacing)

//	InitCompatible
//
//	Initializes the conversion cache with the old-style 2D rotation of points.
//	[Used for backwards compatibility with old mods.]

	{
	int i;

	if (!InitCache(iDirectionCount))
		return;

	for (i = 0; i < iDirectionCount; i++)
		{
		int iRotAngle = (i * 360 / iDirectionCount);
		int iDir = Angle2Direction(iRotAngle, iDirectionCount);
		int iFacingAngle = AngleMod(iRotAngle + iFacing);

		CVector vPos = PolarToVector(iRotAngle + iAngle, (Metric)iRadius);
		m_Cache[iDir].x = (int)vPos.GetX();
		m_Cache[iDir].y = -(int)vPos.GetY();

		m_Cache[iDir].bPaintFirst = (iFacingAngle >= 0 && iFacingAngle <= 180);
		}
	}

void C3DConversion::InitCompatibleXY (int iDirectionCount, int iX, int iY, int iFacing)

//	InitCompatibleXY
//
//	Initializes the conversion cache with the old-style 2D rotation of points.
//	[Used for backwards compatibility with old mods.]

	{
	//	Convert to polar coordinates

	int iRadius;
	int iAngle = IntVectorToPolar(iX, iY, &iRadius);

	//	Init the normal way

	InitCompatible(iDirectionCount, iAngle, iRadius, iFacing);
	}

void C3DConversion::InitXY (int iDirectionCount, int iScale, int iX, int iY, int iZ, int iFacing)

//	InitXY
//
//	Initializes the conversion cache from cartessian coordinates

	{
	//	Convert to polar coordinates

	int iRadius;
	int iAngle = IntVectorToPolar(iX, iY, &iRadius);

	//	Init the normal way

	Init(iDirectionCount, iScale, iAngle, iRadius, iZ, iFacing);
	}

void C3DConversion::InitComplete (int iDirectionCount, int iScale, int iFacing)

//	InitComplete
//
//	Completes the initialization based on stored parameters.

	{
	if (m_bUseCompatible)
		InitCompatible(iDirectionCount, m_iAngle, m_iRadius, iFacing);
	else
		Init(iDirectionCount, iScale, m_iAngle, m_iRadius, m_iZ, iFacing);

	if (!m_sBringToFront.IsBlank())
		OverridePaintFirst(m_sBringToFront, false);

	if (!m_sSendToBack.IsBlank())
		OverridePaintFirst(m_sSendToBack, true);
	}

void C3DConversion::GetCoord (int iRotation, int *retx, int *rety) const

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

void C3DConversion::GetCoordFromDir (int iDirection, int *retx, int *rety) const

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
