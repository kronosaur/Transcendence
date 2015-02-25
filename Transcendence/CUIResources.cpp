//	CUIResources.cpp
//
//	CUIResources class

#include "PreComp.h"
#include "Transcendence.h"

const int DAMAGE_TYPE_ICON_WIDTH =			16;
const int DAMAGE_TYPE_ICON_HEIGHT =			16;

const int DAMAGE_ADJ_ICON_SPACING_X =		2;
const int DAMAGE_ADJ_SPACING_X =			6;

void CUIResources::CreateTitleAnimation (int x, int y, int iDuration, IAnimatron **retpAni)

//	CreateTitleAnimation
//
//	Creates an animation of the main title

	{
	CAniText *pText = new CAniText;
	pText->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pText->SetPropertyColor(CONSTLIT("color"), m_pFonts->rgbTitleColor);
	pText->SetPropertyString(CONSTLIT("text"), CONSTLIT("TRANSCENDENCE"));

	pText->SetPropertyFont(CONSTLIT("font"), &m_pFonts->LogoTitle);
	pText->SetFontFlags(CG16bitFont::AlignCenter);

	pText->AnimateLinearFade(iDuration, 30, 30);

	//	Done

	*retpAni = pText;
	}

void CUIResources::CreateLargeCredit (const CString &sCredit, const CString &sName, int x, int y, int iDuration, IAnimatron **retpAni)

//	CreateLargeCredit
//
//	Creates an animation of a title credit

	{
	CAniText *pCredit = new CAniText;
	pCredit->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pCredit->SetPropertyColor(CONSTLIT("color"), m_pFonts->rgbTitleColor);
	pCredit->SetPropertyString(CONSTLIT("text"), sCredit);

	pCredit->SetPropertyFont(CONSTLIT("font"), &m_pFonts->Header);
	pCredit->SetFontFlags(CG16bitFont::AlignCenter);

	pCredit->AnimateLinearFade(iDuration, 30, 30);

	CAniText *pName = new CAniText;
	pName->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)(y + m_pFonts->Header.GetHeight())));
	pName->SetPropertyColor(CONSTLIT("color"), m_pFonts->rgbTitleColor);
	pName->SetPropertyString(CONSTLIT("text"), sName);

	pName->SetPropertyFont(CONSTLIT("font"), &m_pFonts->Title);
	pName->SetFontFlags(CG16bitFont::AlignCenter);

	pName->AnimateLinearFade(iDuration, 30, 30);

	//	Add them both to a sequencer

	CAniSequencer *pSeq = new CAniSequencer;
	pSeq->AddTrack(pCredit, 0);
	pSeq->AddTrack(pName, 0);

	//	Done

	*retpAni = pSeq;
	}

void CUIResources::CreateMediumCredit (const CString &sCredit, TArray<CString> &Names, int x, int y, int iDuration, IAnimatron **retpAni)

//	CreateMediumCredit
//
//	Creates a credit animation

	{
	int i;

	CAniSequencer *pSeq = new CAniSequencer;

	//	The credit label lasts for the whole time

	CAniText *pCredit = new CAniText;
	pCredit->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pCredit->SetPropertyColor(CONSTLIT("color"), m_pFonts->rgbLightTitleColor);
	pCredit->SetPropertyString(CONSTLIT("text"), sCredit);

	pCredit->SetPropertyFont(CONSTLIT("font"), &m_pFonts->Header);
	pCredit->SetFontFlags(CG16bitFont::AlignCenter);

	pCredit->AnimateLinearFade(iDuration * Names.GetCount(), 20, 20);

	pSeq->AddTrack(pCredit, 0);

	//	Each line of names comes and goes

	int iTime = 0;
	for (i = 0; i < Names.GetCount(); i++)
		{
		CAniText *pName = new CAniText;
		pName->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)(y + m_pFonts->SubTitle.GetHeight())));
		pName->SetPropertyColor(CONSTLIT("color"), m_pFonts->rgbTitleColor);
		pName->SetPropertyString(CONSTLIT("text"), Names[i]);

		pName->SetPropertyFont(CONSTLIT("font"), &m_pFonts->SubTitle);
		pName->SetFontFlags(CG16bitFont::AlignCenter);

		pName->AnimateLinearFade(iDuration, 20, 20);

		pSeq->AddTrack(pName, iTime);
		iTime += iDuration;
		}

	//	Done

	*retpAni = pSeq;
	}

ALERROR CUIResources::Init (SFontTable *pFonts)

//	Init
//
//	Initializes the resources

	{
	//	Fonts

	m_pFonts = pFonts;

	return NOERROR;
	}
