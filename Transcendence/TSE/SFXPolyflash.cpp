//	SFXPolyflash.cpp
//
//	Polyflash SFX

#include "PreComp.h"

class CPolyflashPainter : public IEffectPainter
	{
	public:
		CPolyflashPainter (CPolyflashEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CreateFlame (int iAngle, int iLength, SPoint *Poly);

		CPolyflashEffectCreator *m_pCreator;
	};

//	CPolyflashEffectCreator object

IEffectPainter *CPolyflashEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CPolyflashPainter(this);
	}

//	CPolyflashPainter object

CPolyflashPainter::CPolyflashPainter (CPolyflashEffectCreator *pCreator) : m_pCreator(pCreator)

//	CPolyflashPainter constructor

	{
	}

void CPolyflashPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Reads from a stream

	{
	DWORD dwLoad;

	//	Previous versions used to store a direction here

	if (Ctx.dwVersion < 51)
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	}

void CPolyflashPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes to a stream

	{
	}

void CPolyflashPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paints the flash

	{
	CG16bitBinaryRegion Region;
	SPoint Poly[6];

	//	Create the broad outer flame

	CreateFlame(Ctx.iRotation, 40, Poly);
	Region.CreateFromConvexPolygon(6, Poly);
	Region.FillTrans(Dest, x, y, CG16bitImage::RGBValue(0xff, 0x80, 0x40), 0x80);

	//	Create the central bright area

	CreateFlame(Ctx.iRotation, 30, Poly);
	Region.CreateFromConvexPolygon(6, Poly);
	Region.Fill(Dest, x, y, CG16bitImage::RGBValue(0xff, 0xff, 0xa0));
	}

void CPolyflashPainter::CreateFlame (int iAngle, 
									 int iLength,
									 SPoint *Poly)

//	CreateFlame
//
//	Creates a single lick of flame

	{
	CVector vPos;

	int iSmallLen = max(1, iLength / 10);
	int iMediumLen = iSmallLen * 3;

	//	Start at the origin

	Poly[0].x = 0;
	Poly[0].y = 0;

	//	The left side

	vPos = PolarToVector(iAngle + 300, iSmallLen);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	vPos = PolarToVector(iAngle + 330, iMediumLen);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	The tip

	vPos = PolarToVector(iAngle, iLength);
	Poly[3].x = (int)vPos.GetX();
	Poly[3].y = -(int)vPos.GetY();

	//	The right side

	vPos = PolarToVector(iAngle + 30, iMediumLen);
	Poly[4].x = (int)vPos.GetX();
	Poly[4].y = -(int)vPos.GetY();

	vPos = PolarToVector(iAngle + 60, iSmallLen);
	Poly[5].x = (int)vPos.GetX();
	Poly[5].y = -(int)vPos.GetY();
	}
