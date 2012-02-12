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
	pText->SetPropertyColor(CONSTLIT("color"), m_pFonts->wTitleColor);
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
	pCredit->SetPropertyColor(CONSTLIT("color"), m_pFonts->wTitleColor);
	pCredit->SetPropertyString(CONSTLIT("text"), sCredit);

	pCredit->SetPropertyFont(CONSTLIT("font"), &m_pFonts->Header);
	pCredit->SetFontFlags(CG16bitFont::AlignCenter);

	pCredit->AnimateLinearFade(iDuration, 30, 30);

	CAniText *pName = new CAniText;
	pName->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)(y + m_pFonts->Header.GetHeight())));
	pName->SetPropertyColor(CONSTLIT("color"), m_pFonts->wTitleColor);
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
	pCredit->SetPropertyColor(CONSTLIT("color"), m_pFonts->wLightTitleColor);
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
		pName->SetPropertyColor(CONSTLIT("color"), m_pFonts->wTitleColor);
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

void CUIResources::DrawReferenceDamageAdj (CG16bitImage &Dest, int x, int y, int iLevel, int iHP, const int *iDamageAdj) const

//	DrawReferenceDamageAdj
//
//	Takes an array of damage type adj values and displays them

	{
	int i;
	bool bSortByDamageType = true;

	//	Must have positive HP

	if (iHP == 0)
		return;

	//	Sort damage types from highest to lowest

	CSymbolTable Sorted;
	int iLengthEstimate = 0;
	int iImmuneCount = 0;
	for (i = 0; i < damageCount; i++)
		{
		//	Skip if this damage type is not appropriate to our level

		int iDamageLevel = GetDamageTypeLevel((DamageTypes)i);
		if (iDamageLevel < iLevel - 5 || iDamageLevel > iLevel + 3)
			continue;

		//	Skip if the damage adj is 100%

		if (iDamageAdj[i] == iHP)
			continue;

		//	Figure out the sort order

		CString sKey;
		if (bSortByDamageType)
			sKey = strPatternSubst(CONSTLIT("%02d"), i);
		else
			{
			DWORD dwHighToLow = (iDamageAdj[i] == -1 ? 0 : 1000000 - iDamageAdj[i]);
			sKey = strPatternSubst(CONSTLIT("%08x %02d"), dwHighToLow, i);
			}

		//	Add to list

		DWORD dwValue = MAKELONG((WORD)i, (WORD)(short)iDamageAdj[i]);

		Sorted.AddEntry(sKey, (CObject *)dwValue);

		//	Estimate how many entries we will have (so we can decide the font size)
		//	We assume that immune entries get collapsed.

		if (iDamageAdj[i] != -1)
			iLengthEstimate++;
		else
			iImmuneCount++;
		}

	//	If we have six or more icons, then we need to paint smaller

	iLengthEstimate += Min(2, iImmuneCount);
	const CG16bitFont &TheFont = (iLengthEstimate >= 6 ? m_pFonts->Small : m_pFonts->Medium);
	int cyOffset = (m_pFonts->Medium.GetHeight() - TheFont.GetHeight()) / 2;
	
	//	Paint the icons

	for (i = 0; i < Sorted.GetCount(); i++)
		{
		DWORD dwValue = (DWORD)Sorted.GetValue(i);
		int iDamageType = LOWORD(dwValue);
		int iDamageAdj = (int)(short)HIWORD(dwValue);
		int iPercentAdj = (100 * (iDamageAdj - iHP) / iHP);

		//	Prettify the % by rounding to a number divisible by 5

		int iPrettyPercent = 5 * ((iPercentAdj + 2 * Sign(iPercentAdj)) / 5);

		//	Skip if prettify results in 0%

		if (m_bOptionShowDamageAdjAsHP && iPrettyPercent == 0)
			continue;

		//	Draw icon

		g_pHI->GetVisuals().DrawDamageTypeIcon(Dest, x, y, (DamageTypes)iDamageType);
		x += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;

		//	If we have a bunch of entries with "immune", then compress them

		if (i < (Sorted.GetCount() - 1)
				&& iDamageAdj == -1
				&& (iDamageAdj == (int)(short)HIWORD((DWORD)Sorted.GetValue(i + 1))))
			continue;

		//	Figure out how to display damage adj

		CString sStat;
		if (iDamageAdj == -1)
			sStat = CONSTLIT("immune");
		else if (m_bOptionShowDamageAdjAsHP)
			sStat = strFromInt(iDamageAdj);
		else
			sStat = strPatternSubst(CONSTLIT("%s%d%%"), (iPrettyPercent > 0 ? CONSTLIT("+") : NULL_STR), iPrettyPercent);
		
		//	Draw

		Dest.DrawText(x,
				y + cyOffset,
				TheFont,
				m_pFonts->wItemRef,
				sStat,
				0,
				&x);

		x += DAMAGE_ADJ_SPACING_X;
		}
	}

void CUIResources::DrawReferenceDamageType (CG16bitImage &Dest, int x, int y, int iDamageType, const CString &sRef) const

//	DrawReferenceDamageType
//
//	Draws a single damage type icon (if not damageError) followed by reference text

	{
	//	Paint the icon first

	if (iDamageType != damageError)
		{
		g_pHI->GetVisuals().DrawDamageTypeIcon(Dest, x, y, (DamageTypes)iDamageType);
		x += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;
		}

	//	Paint the reference text

	Dest.DrawText(x,
			y,
			m_pFonts->Medium,
			m_pFonts->wItemRef,
			sRef,
			0);
	}

void CUIResources::GetDamageTypeIconRect (RECT *retRect) const

//	GetDamageTypeIconRect
//
//	Returns the height and width of a damage type icon

	{
	retRect->left = 0;
	retRect->top = 0;
	retRect->right = DAMAGE_TYPE_ICON_WIDTH;
	retRect->bottom = DAMAGE_TYPE_ICON_HEIGHT;
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
