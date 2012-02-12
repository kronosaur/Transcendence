//	AGArea.cpp
//
//	Implementation of AGArea class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

AGArea::AGArea (void) : CObject(NULL),
		m_pScreen(NULL),
		m_pController(NULL),
		m_bVisible(true),
		m_iEffectCount(0),
		m_pEffects(NULL),
		m_pParent(NULL)

//	AGArea constructor

	{
	}

AGArea::~AGArea (void)

//	AGArea destructor

	{
	if (m_pEffects)
		delete [] m_pEffects;
	}

void AGArea::AddEffect (const SEffectDesc &Effect)

//	AddEffect
//
//	Adds an effect

	{
	int i;
	int iAlloc = m_iEffectCount + 1;

	SEffectDesc *pNew = new SEffectDesc [iAlloc];
	for (i = 0; i < m_iEffectCount; i++)
		pNew[i] = m_pEffects[i];

	pNew[i] = Effect;

	if (m_pEffects)
		delete [] m_pEffects;

	m_pEffects = pNew;
	m_iEffectCount = iAlloc;
	}

void AGArea::AddShadowEffect (void)

//	AddShadowEffect
//
//	Adds a shadow effect

	{
	SEffectDesc Effect;
	utlMemSet(&Effect, sizeof(Effect), 0);

	Effect.iType = effectShadow;
	AddEffect(Effect);
	}

ALERROR AGArea::Init (AGScreen *pScreen, IAreaContainer *pParent, const RECT &rcRect, DWORD dwTag)

//	Init
//
//	This is called when the area is added to a screen

	{
	ASSERT(m_pScreen == NULL);
	ASSERT(m_pParent == NULL);

	m_pScreen = pScreen;
	m_pParent = pParent;
	m_rcRect = rcRect;
	m_dwTag = dwTag;

	return NOERROR;
	}

void AGArea::SetRect (const RECT &rcRect)

//	SetRect
//
//	Set the rect

	{
	m_rcRect = rcRect;
	OnSetRect();
	m_pParent->OnAreaSetRect();
	}

void AGArea::SignalAction (DWORD dwData)

//	SignalAction
//
//	Signals an action for this area

	{
	if (m_pController)
		m_pController->Action(m_dwTag, dwData);
	else
		m_pScreen->GetController()->Action(m_dwTag, dwData);
	}

void AGArea::ShowHide (bool bShow)

//	ShowHide
//
//	Shows or hides the area

	{
	if (bShow != m_bVisible)
		{
		m_bVisible = bShow;
		Invalidate();
		OnShowHide(bShow);
		}
	}
