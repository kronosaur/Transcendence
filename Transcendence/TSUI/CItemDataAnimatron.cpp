//	CItemDataAnimatron.cpp
//
//	CItemDataAnimatron class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int INDEX_VISIBLE =					0;
#define PROP_VISIBLE						CONSTLIT("visible")

const int INDEX_POSITION =					1;
#define PROP_POSITION						CONSTLIT("position")

const int INDEX_SCALE =						2;
#define PROP_SCALE							CONSTLIT("scale")

const int INDEX_ROTATION =					3;
#define PROP_ROTATION						CONSTLIT("rotation")

const int INDEX_COLOR =						4;
#define PROP_COLOR							CONSTLIT("color")

const int INDEX_OPACITY =					5;
#define PROP_OPACITY						CONSTLIT("opacity")

const int INDEX_TEXT_ALIGN_HORZ =			6;
#define PROP_TEXT_ALIGN_HORZ				CONSTLIT("textAlignHorz")

const int INDEX_TEXT_ALIGN_VERT =			7;
#define PROP_TEXT_ALIGN_VERT				CONSTLIT("textAlignVert")

const int INDEX_FONT =						8;
#define PROP_FONT							CONSTLIT("font")

#define ALIGN_BOTTOM						CONSTLIT("bottom")
#define ALIGN_CENTER						CONSTLIT("center")
#define ALIGN_LEFT							CONSTLIT("left")
#define ALIGN_RIGHT							CONSTLIT("right")
#define ALIGN_TOP							CONSTLIT("top")

const int DAMAGE_TYPE_ICON_WIDTH =			16;
const int DAMAGE_TYPE_ICON_HEIGHT =			16;

const int DAMAGE_ADJ_ICON_SPACING_X =		2;
const int DAMAGE_ADJ_SPACING_X =			6;

CItemDataAnimatron::CItemDataAnimatron (const CVisualPalette &VI, const CItem &Item) : 
		m_VI(VI), 
		m_Item(Item)

//	CItemDataAnimatron constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(0.0, 0.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	m_Properties.SetString(PROP_TEXT_ALIGN_HORZ, ALIGN_LEFT);
	m_Properties.SetString(PROP_TEXT_ALIGN_VERT, ALIGN_TOP);
	m_Properties.SetFont(PROP_FONT, &VI.GetFont(fontMedium));

	//	Get the data from the item

	CItemType *pType = Item.GetType();
	if (pType)
		{
		m_bIsWeapon = (pType->GetCategory() == itemcatWeapon || pType->GetCategory() == itemcatLauncher);
		if (m_bIsWeapon)
			{
			SDamageAdjCell *pCell = m_DamageAdj.Insert();

			if (!m_Item.GetReferenceDamageType(NULL, -1, CItemType::FLAG_ACTUAL_ITEM, &pCell->iDamage, &pCell->sText))
				m_DamageAdj.DeleteAll();
			}
		else
			CalcDamageAdj(m_Item, &m_DamageAdj);
		}
	else
		m_bIsWeapon = true;
	}

void CItemDataAnimatron::CalcDamageAdj (const CItem &Item, TArray<SDamageAdjCell> *retDamageAdj)

//	CalcDamageAdj
//
//	Calculates the display of damage adjustment.

	{
	int i;
	bool bSortByDamageType = true;
	bool bShowDamageAdjAsHP = false;

	//	Get info for the item

	int iLevel = Item.GetType()->GetLevel();
	int iHP;
	int DamageAdj[damageCount];
	if (!Item.GetReferenceDamageAdj(NULL, CItemType::FLAG_ACTUAL_ITEM, &iHP, DamageAdj))
		return;

	//	Must have positive HP

	if (iHP == 0)
		return;

	//	Sort damage types from highest to lowest

	TSortMap<CString, SDamageAdjCell> Sorted;
	bool bPrevImmune = false;
	for (i = 0; i < damageCount; i++)
		{
		//	Skip if this damage type is not appropriate to our level

		int iDamageLevel = GetDamageTypeLevel((DamageTypes)i);
		if (iDamageLevel < iLevel - 5 || iDamageLevel > iLevel + 3)
			continue;

		//	Skip if the damage adj is 100%

		if (DamageAdj[i] == iHP)
			continue;

		//	If we're immune to this damage type, handle differently.

		CString sText;
		if (DamageAdj[i] == -1)
			{
			//	If we were immune to the previous damage type, then don't bother
			//	printing this one.

			if (!bPrevImmune)
				sText = CONSTLIT("immune");

			bPrevImmune = true;
			}
		else
			{
			//	Compute the % adjust

			int iPercentAdj = (100 * (DamageAdj[i] - iHP) / iHP);

			//	Prettify the % by rounding to a number divisible by 5

			int iPrettyPercent = 5 * ((iPercentAdj + 2 * Sign(iPercentAdj)) / 5);

			//	Skip if prettify results in 0%

			if (bShowDamageAdjAsHP && iPrettyPercent == 0)
				continue;

			//	Figure out what to print

			if (bShowDamageAdjAsHP)
				sText = strFromInt(DamageAdj[i]);
			else
				sText = strPatternSubst(CONSTLIT("%s%d%%"), (iPrettyPercent > 0 ? CONSTLIT("+") : NULL_STR), iPrettyPercent);

			bPrevImmune = false;
			}

		//	Figure out the sort order

		CString sKey;
		if (bSortByDamageType)
			sKey = strPatternSubst(CONSTLIT("%02d"), i);
		else
			{
			DWORD dwHighToLow = (DamageAdj[i] == -1 ? 0 : 1000000 - DamageAdj[i]);
			sKey = strPatternSubst(CONSTLIT("%08x %02d"), dwHighToLow, i);
			}

		//	Add to list

		SDamageAdjCell *pCell = Sorted.Insert(sKey);
		pCell->iDamage = (DamageTypes)i;
		pCell->sText = sText;
		}

	//	Add to result. The first entry is always HP.

	retDamageAdj->DeleteAll();
	SDamageAdjCell *pCell = retDamageAdj->Insert();
	pCell->iDamage = damageGeneric;
	pCell->sText = strPatternSubst(CONSTLIT("hp: %d"), iHP);

	//	Add the remainder

	int iInsertAt = retDamageAdj->GetCount();
	retDamageAdj->InsertEmpty(Sorted.GetCount());
	for (i = 0; i < Sorted.GetCount(); i++)
		{
		SDamageAdjCell &Cell = retDamageAdj->GetAt(i + iInsertAt);
		Cell.iDamage = Sorted[i].iDamage;
		Cell.sText = Sorted[i].sText;
		}
	}

void CItemDataAnimatron::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the spacing rect

	{
	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	if (pFont == NULL)
		return IAnimatron::GetSpacingRect(retrcRect);

	retrcRect->left = 0;
	retrcRect->top = 0;
	retrcRect->right = MeasureDamageAdj(*pFont, m_DamageAdj);
	retrcRect->bottom = pFont->GetHeight();
	}

void CItemDataAnimatron::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints

	{
	int i;

	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	if (pFont == NULL)
		pFont = &CReanimator::GetDefaultFont();

	//	Get the rect size

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();

	RECT rcRect;
	rcRect.left = (int)vPos.GetX();
	rcRect.top = (int)vPos.GetY();
	rcRect.right = rcRect.left + (int)vSize.GetX();
	rcRect.bottom = rcRect.top + (int)vSize.GetY();

	//	This is always one line high.

	int cyOutput = Max(pFont->GetHeight(), DAMAGE_TYPE_ICON_HEIGHT);
	int cxOutput = MeasureDamageAdj(*pFont, m_DamageAdj);

	//	Figure out where we want to paint.

	int x;
	CString sAlignHorz = m_Properties[INDEX_TEXT_ALIGN_HORZ].GetString();
	if (strEquals(sAlignHorz, ALIGN_CENTER))
		x = rcRect.left + (RectWidth(rcRect) - cxOutput) / 2;
	else if (strEquals(sAlignHorz, ALIGN_RIGHT))
		x = rcRect.left + RectWidth(rcRect) - cxOutput;
	else
		x = rcRect.left;

	int y;
	CString sAlignVert = m_Properties[INDEX_TEXT_ALIGN_VERT].GetString();
	if (strEquals(sAlignVert, ALIGN_CENTER))
		y = rcRect.top + (RectHeight(rcRect) - cyOutput) / 2;
	else if (strEquals(sAlignVert, ALIGN_BOTTOM))
		y = rcRect.top + RectHeight(rcRect) - cyOutput;
	else
		y = rcRect.top;

	//	Get color & opacity

	WORD wColor = m_Properties[INDEX_COLOR].GetColor();
	DWORD dwOpacity = m_Properties[INDEX_OPACITY].GetOpacity() * Ctx.dwOpacityToDest / 255;

	//	Paint

	for (i = 0; i < m_DamageAdj.GetCount(); i++)
		{
		SDamageAdjCell *pCell = &m_DamageAdj[i];

		//	Spacing, if necessary

		if (i != 0)
			x += DAMAGE_ADJ_SPACING_X;

		//	Paint the icon

		if (pCell->iDamage != damageGeneric)
			{
			m_VI.DrawDamageTypeIcon(Ctx.Dest, x, y, pCell->iDamage);
			x += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;
			}

		//	Paint the text

		if (!pCell->sText.IsBlank())
			pFont->DrawText(Ctx.Dest, x, y, wColor, dwOpacity, pCell->sText, 0, &x);
		}
	}

int CItemDataAnimatron::MeasureDamageAdj (const CG16bitFont &Font, const TArray<SDamageAdjCell> &DamageAdj)

//	MeasureDamageAdj
//
//	Measure the horizontal size of the damage adj text and icons.

	{
	int i;

	int cx = 0;
	for (i = 0; i < DamageAdj.GetCount(); i++)
		{
		SDamageAdjCell *pCell = &DamageAdj[i];

		//	Spacing, if necessary

		if (i != 0)
			cx += DAMAGE_ADJ_SPACING_X;

		//	Add the icon

		if (pCell->iDamage != damageGeneric)
			cx += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;

		//	Add the text

		if (!pCell->sText.IsBlank())
			cx += Font.MeasureText(pCell->sText);
		}

	return cx;
	}
