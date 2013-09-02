//	CEffectGroupCreator.cpp
//
//	CEffectGroupCreator class

#include "PreComp.h"

#define ANGLE_OFFSET_ATTRIB						CONSTLIT("angleOffset")
#define RADIUS_OFFSET_ATTRIB					CONSTLIT("radiusOffset")
#define ROTATION_ADJ_ATTRIB						CONSTLIT("rotationAdj")
#define X_OFFSET_ATTRIB							CONSTLIT("xOffset")
#define Y_OFFSET_ATTRIB							CONSTLIT("yOffset")

//	CEffectGroupPainter -------------------------------------------------------

class CEffectGroupPainter : public IEffectPainter
	{
	public:
		CEffectGroupPainter (CEffectGroupCreator *pCreator, CCreatePainterCtx &Ctx);

		virtual ~CEffectGroupPainter (void);
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void);
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void);
		virtual void OnMove (bool *retbBoundsChanged);
		virtual void OnUpdate (SEffectUpdateCtx &Ctx);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;
		virtual void SetVariants (int iVariants);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		SViewportPaintCtx *AdjustCtx (SViewportPaintCtx &Ctx, SViewportPaintCtx &NewCtx, int *retx, int *rety);

		CEffectGroupCreator *m_pCreator;
		IEffectPainter **m_pPainters;
	};

CEffectGroupPainter::CEffectGroupPainter (CEffectGroupCreator *pCreator, CCreatePainterCtx &Ctx) : m_pCreator(pCreator)

//	CEffectGroupPainter constructor

	{
	m_pPainters = new IEffectPainter * [m_pCreator->GetCount()];

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i] = m_pCreator->CreateSubPainter(Ctx, i);
	}

CEffectGroupPainter::~CEffectGroupPainter (void)

//	CEffectGroupPainter destructor

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->Delete();

	delete [] m_pPainters;
	}

SViewportPaintCtx *CEffectGroupPainter::AdjustCtx (SViewportPaintCtx &Ctx, SViewportPaintCtx &NewCtx, int *retx, int *rety)

//	AdjustCtx
//
//	Adjusts the context to account for any offsets

	{
	if (m_pCreator->HasOffsets())
		{
		NewCtx = Ctx;

		int xOffset;
		int yOffset;
		m_pCreator->ApplyOffsets(&NewCtx, &xOffset, &yOffset);

		*retx += xOffset;
		*rety += yOffset;

		return &NewCtx;
		}
	else
		return &Ctx;
	}

int CEffectGroupPainter::GetFadeLifetime (void)

//	GetFadeLifetime
//
//	Returns the lifetime to paint fading state

	{
	int iMaxLifetime = 0;

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			iMaxLifetime = Max(iMaxLifetime, m_pPainters[i]->GetFadeLifetime());

	return iMaxLifetime;
	}

void CEffectGroupPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Return the bounding rect

	{
	retRect->left = 0;
	retRect->top = 0;
	retRect->bottom = 0;
	retRect->right = 0;

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			{
			RECT rcRect;
			m_pPainters[i]->GetRect(&rcRect);

			::UnionRect(retRect, retRect, &rcRect);
			}
	}

void CEffectGroupPainter::OnBeginFade (void)

//	OnBeginFade

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->OnBeginFade();
	}

void CEffectGroupPainter::OnMove (bool *retbBoundsChanged)

//	OnMove

	{
	bool bBoundsChanged = false;

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			{
			bool bSingleBoundsChanged;

			m_pPainters[i]->OnMove(&bSingleBoundsChanged);

			if (bSingleBoundsChanged)
				bBoundsChanged = true;
			}

	if (retbBoundsChanged)
		*retbBoundsChanged = bBoundsChanged;
	}

void CEffectGroupPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			{
			//	The UNID is ignored (because it is the UNID of the creator)

			ReadUNID(Ctx);

			//	Validate the class

			if (IEffectPainter::ValidateClass(Ctx, m_pPainters[i]->GetCreator()->GetTag()) != NOERROR)
				{
				m_pPainters[i]->Delete();
				m_pPainters[i] = NULL;
				continue;
				}

			//	Read the painter

			m_pPainters[i]->ReadFromStream(Ctx);
			}
	}

void CEffectGroupPainter::OnUpdate (SEffectUpdateCtx &Ctx)

//	OnUpdate
//
//	Update painters

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->OnUpdate(Ctx);
	}

void CEffectGroupPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		CEffectCreator::WritePainterToStream(pStream, m_pPainters[i]);
	}

void CEffectGroupPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	SViewportPaintCtx NewCtx;
	SViewportPaintCtx *pCtx = AdjustCtx(Ctx, NewCtx, &x, &y);

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->Paint(Dest, x, y, *pCtx);
	}

void CEffectGroupPainter::PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintFade
//
//	Paint

	{
	SViewportPaintCtx NewCtx;
	SViewportPaintCtx *pCtx = AdjustCtx(Ctx, NewCtx, &x, &y);

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->PaintFade(Dest, x, y, *pCtx);
	}

void CEffectGroupPainter::PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx)

//	PaintHit
//
//	Paints a hit

	{
	SViewportPaintCtx NewCtx;
	SViewportPaintCtx *pCtx = AdjustCtx(Ctx, NewCtx, &x, &y);

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->PaintHit(Dest, x, y, vHitPos, *pCtx);
	}

bool CEffectGroupPainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the point is in the image

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			if (m_pPainters[i]->PointInImage(x, y, iTick, iVariant, iRotation))
				return true;

	return false;
	}

void CEffectGroupPainter::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the variants

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i])
			m_pPainters[i]->SetVariants(iVariants);
	}

//	CEffectGroupCreator --------------------------------------------------------

CEffectGroupCreator::CEffectGroupCreator (void) : m_pCreators(NULL), m_iCount(0)

//	CEffectGroupCreator constructor

	{
	}

CEffectGroupCreator::~CEffectGroupCreator (void)

//	CEffectGroupCreator destructor

	{
	if (m_pCreators)
		delete [] m_pCreators;
	}

void CEffectGroupCreator::ApplyOffsets (SViewportPaintCtx *ioCtx, int *retx, int *rety)

//	ApplyOffsets
//
//	Adds appropriate offsets to Ctx

	{
	int xOffset = m_xOffset;
	int yOffset = m_yOffset;
	if (m_iAngleOffset || m_iRadiusOffset)
		{
		CVector vVec = PolarToVector(360 + ioCtx->iRotation + m_iAngleOffset, (Metric)m_iRadiusOffset);
		xOffset += (int)vVec.GetX();
		yOffset += (int)vVec.GetY();
		}

	//	Adjust the Xform

	ioCtx->XForm.Offset(xOffset, -yOffset);

	//	Adjust the rotation

	ioCtx->iRotation = (360 + ioCtx->iRotation + m_iRotationAdj) % 360;

	//	Done

	if (retx)
		*retx = xOffset;

	if (rety)
		*rety = -yOffset;
	}

ALERROR CEffectGroupCreator::CreateEffect (CSystem *pSystem,
										   CSpaceObject *pAnchor,
										   const CVector &vPos,
										   const CVector &vVel,
										   int iRotation,
										   int iVariant,
										   CSpaceObject **retpEffect)

//	CreateEffect
//
//	Creates an effect object

	{
	ALERROR error;
	CSpaceObject *pLastEffect = NULL;

	//	Create the sub effects

	for (int i = 0; i < m_iCount; i++)
		{
		if (error = m_pCreators[i]->CreateEffect(pSystem, pAnchor, vPos, vVel, iRotation, iVariant, &pLastEffect))
			return error;
		}

	//	Play sound associated with the group (if any)
	//	(We use the effect object only to get a position).

	if (pLastEffect)
		CEffectCreator::PlaySound(pLastEffect);

	//	Done

	if (retpEffect)
		*retpEffect = pLastEffect;

	return NOERROR;
	}

IEffectPainter *CEffectGroupCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a painter

	{
	return new CEffectGroupPainter(this, Ctx);
	}

int CEffectGroupCreator::GetLifetime (void)

//	GetLifetime
//
//	Returns the lifetime of the effect
	
	{
	int iTotalLifetime = 0;
	for (int i = 0; i < m_iCount; i++)
		{
		int iLifetime = m_pCreators[i]->GetLifetime();
		if (iLifetime == -1)
			{
			iTotalLifetime = -1;
			break;
			}
		else if (iLifetime > iTotalLifetime)
			iTotalLifetime = iLifetime;
		}

	return iTotalLifetime;
	}

ALERROR CEffectGroupCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Creates from XML

	{
	ALERROR error;
	int i;

	ASSERT(m_pCreators == NULL);

	//	Allocate the creator array

	m_iCount = pDesc->GetContentElementCount();
	if (m_iCount == 0)
		return ERR_FAIL;

	m_pCreators = new CEffectCreatorRef [m_iCount];

	for (i = 0; i < m_iCount; i++)
		{
		CString sSubUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, i);

		if (error = m_pCreators[i].LoadSimpleEffect(Ctx, sSubUNID, pDesc->GetContentElement(i)))
			return error;
		}

	//	Load any offsets or transforms

	m_iAngleOffset = pDesc->GetAttributeIntegerBounded(ANGLE_OFFSET_ATTRIB, -360, 360, 0);
	m_iRadiusOffset = pDesc->GetAttributeInteger(RADIUS_OFFSET_ATTRIB);
	m_xOffset = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
	m_yOffset = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);
	m_iRotationAdj = pDesc->GetAttributeIntegerBounded(ROTATION_ADJ_ATTRIB, -360, 360, 0);

	m_bHasOffsets = (m_iAngleOffset != 0 || m_iRadiusOffset != 0
			|| m_xOffset != 0 || m_yOffset != 0 || m_iRotationAdj != 0);

	return NOERROR;
	}

ALERROR CEffectGroupCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	for (int i = 0; i < m_iCount; i++)
		if (error = m_pCreators[i].Bind(Ctx))
			return error;

	return NOERROR;
	}

void CEffectGroupCreator::OnEffectPlaySound (CSpaceObject *pSource)

//	OnEffectPlaySound
//
//	Play sounds

	{
	//	Let subclass play sounds associated with the group

	CEffectCreator::OnEffectPlaySound(pSource);

	//	Play additional sounds

	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->PlaySound(pSource);
	}

void CEffectGroupCreator::OnMarkImages (void)

//	MarkImages
//
//	Mark images used by this effect

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->MarkImages();
	}

void CEffectGroupCreator::SetLifetime (int iLifetime)

//	SetLifetime
//
//	Sets the lifetime

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->SetLifetime(iLifetime);
	}

void CEffectGroupCreator::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the variants

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->SetVariants(iVariants);
	}

