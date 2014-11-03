//	CGSelectorArea.cpp
//
//	CGSelectorArea class

#include "PreComp.h"
#include "Transcendence.h"

const int ITEM_ICON_WIDTH =					64;
const int ITEM_ICON_HEIGHT =				64;

const int ITEM_ENTRY_WIDTH =				140;
const int ITEM_ENTRY_HEIGHT =				(ITEM_ICON_HEIGHT + 40);

const int ITEM_ENTRY_PADDING_TOP =			2;
const int ITEM_ENTRY_PADDING_LEFT =			4;
const int ITEM_ENTRY_PADDING_RIGHT =		4;

const int WIDE_COLUMN_SPACING =				200;
const int NARROW_COLUMN_SPACING =			170;
const int INTER_SPACING_Y =					30;

const int NARROW_COLUMN_LEFT_X =			-NARROW_COLUMN_SPACING - (ITEM_ENTRY_WIDTH / 2);
const int NARROW_COLUMN_RIGHT_X =			NARROW_COLUMN_SPACING - (ITEM_ENTRY_WIDTH / 2);
const int WIDE_COLUMN_LEFT_X =				-WIDE_COLUMN_SPACING - (ITEM_ENTRY_WIDTH / 2);
const int WIDE_COLUMN_RIGHT_X =				WIDE_COLUMN_SPACING - (ITEM_ENTRY_WIDTH / 2);

enum ELayoutPositions
	{
	posFront,
	posMiddleFront,
	posMiddle,
	posMiddleBack,
	posBack,

	posLeft,
	posCenter,
	posRight,
	};

struct SLayoutDesc
	{
	DeviceNames iSlotType;					//	Reserved for this slot type
	int xLeft;								//	Left coordinate of area (relative to center)
	int yTop;								//	Top coordinate of area (relative to center)

	ELayoutPositions iFrontPos;
	ELayoutPositions iSidePos;
	};

const SLayoutDesc g_MiscDevicesLayout[] =
	{
		{
			devReactor,
			-(ITEM_ENTRY_WIDTH / 2),
			-(ITEM_ENTRY_HEIGHT / 2),
			posMiddle,
			posCenter,
			},
		{
			devDrive,
			-(ITEM_ENTRY_WIDTH / 2),
			-(ITEM_ENTRY_HEIGHT / 2) + INTER_SPACING_Y + ITEM_ENTRY_HEIGHT,
			posBack,
			posCenter,
			},
		{
			devCargo,
			-(ITEM_ENTRY_WIDTH / 2),
			-(ITEM_ENTRY_HEIGHT / 2) - INTER_SPACING_Y - ITEM_ENTRY_HEIGHT,
			posFront,
			posCenter,
			},

		{
			devNone,
			WIDE_COLUMN_RIGHT_X,
			-ITEM_ENTRY_HEIGHT - (INTER_SPACING_Y / 2),
			posMiddleFront,
			posRight,
			},
		{
			devNone,
			WIDE_COLUMN_LEFT_X,
			-ITEM_ENTRY_HEIGHT - (INTER_SPACING_Y / 2),
			posMiddleFront,
			posLeft,
			},
		{
			devNone,
			WIDE_COLUMN_RIGHT_X,
			(INTER_SPACING_Y / 2),
			posMiddleBack,
			posRight,
			},
		{
			devNone,
			WIDE_COLUMN_LEFT_X,
			(INTER_SPACING_Y / 2),
			posMiddleBack,
			posLeft,
			},

		{
			devNone,
			NARROW_COLUMN_RIGHT_X,
			(INTER_SPACING_Y / 2) + ITEM_ENTRY_HEIGHT + INTER_SPACING_Y,
			posBack,
			posRight,
			},
		{
			devNone,
			NARROW_COLUMN_LEFT_X,
			(INTER_SPACING_Y / 2) + ITEM_ENTRY_HEIGHT + INTER_SPACING_Y,
			posBack,
			posLeft,
			},
		{
			devNone,
			NARROW_COLUMN_RIGHT_X,
			-(INTER_SPACING_Y / 2) - (2 * ITEM_ENTRY_HEIGHT) - INTER_SPACING_Y,
			posFront,
			posRight
			},
		{
			devNone,
			NARROW_COLUMN_LEFT_X,
			-(INTER_SPACING_Y / 2) - (2 * ITEM_ENTRY_HEIGHT) - INTER_SPACING_Y,
			posFront,
			posLeft
			},
	};

const int MISC_DEVICES_LAYOUT_COUNT =		(sizeof(g_MiscDevicesLayout) / sizeof(g_MiscDevicesLayout[0]));
const int REACTOR_SLOT_INDEX =				0;
const int DRIVE_SLOT_INDEX =				1;
const int CARGO_SLOT_INDEX =				2;
const int FIRST_UNNAMED_SLOT_INDEX =		3;

CGSelectorArea::CGSelectorArea (const CVisualPalette &VI) :
		m_VI(VI),
		m_pSource(NULL),
		m_iCursor(-1)

//	CGSelectorArea constructor

	{
	}

CGSelectorArea::~CGSelectorArea (void)

//	CGSelectorArea destructor

	{
	}

void CGSelectorArea::CalcRegionRect (const SEntry &Entry, int xCenter, int yCenter, RECT *retrcRect)

//	CalcRegionRect
//
//	Calculates the region rectangle

	{
	retrcRect->left = xCenter + Entry.rcRect.left;
	retrcRect->top = yCenter + Entry.rcRect.top;
	retrcRect->right = retrcRect->left + RectWidth(Entry.rcRect);
	retrcRect->bottom = retrcRect->top + RectHeight(Entry.rcRect);
	}

void CGSelectorArea::CleanUp (void)

//	CleanUp
//
//	Reset

	{
	m_Regions.DeleteAll();
	m_pSource = NULL;
	m_iCursor = -1;
	}

bool CGSelectorArea::FindLayoutForPos (const CVector &vPos, const TArray<bool> &SlotStatus, int *retiIndex)

//	FindLayoutForPos
//
//	Returns the nearest layout index for the given device position.

	{
	int i;

	ELayoutPositions iDesiredFront;
	if (Absolute(vPos.GetY()) <= 1.0)
		iDesiredFront = posMiddle;
	else if (vPos.GetY() > 0.0)
		iDesiredFront = posFront;
	else
		iDesiredFront = posBack;

	ELayoutPositions iDesiredSide;
	if (Absolute(vPos.GetX()) < 1.0)
		iDesiredSide = posCenter;
	else if (vPos.GetX() > 0.0)
		iDesiredSide = posRight;
	else
		iDesiredSide = posLeft;

	int iBestFit = 0;
	int iBestLayout = -1;
	for (i = 0; i < MISC_DEVICES_LAYOUT_COUNT; i++)
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[i];

		//	Skip slots already in use.

		if (!SlotStatus[i])
			continue;

		//	Compute our fit

		int iFit = 0;
		switch (pLayout->iFrontPos)
			{
			case posFront:
				if (iDesiredFront == posFront)
					iFit += 10;
				else if (iDesiredFront == posBack)
					iFit -= 10;
				break;

			case posMiddleFront:
				if (iDesiredFront == posFront)
					iFit += 5;
				else if (iDesiredFront == posBack)
					iFit -= 5;
				break;

			case posMiddle:
				if (iDesiredFront == posMiddle)
					iFit += 5;
				break;

			case posMiddleBack:
				if (iDesiredFront == posFront)
					iFit -= 5;
				else if (iDesiredFront == posBack)
					iFit += 5;
				break;

			case posBack:
				if (iDesiredFront == posFront)
					iFit -= 10;
				else if (iDesiredFront == posBack)
					iFit += 10;
				break;
			}

		switch (pLayout->iSidePos)
			{
			case posLeft:
				if (iDesiredSide == posLeft)
					iFit += 10;
				else if (iDesiredSide == posRight)
					iFit -= 10;
				break;

			case posCenter:
				if (iDesiredSide == posCenter)
					iFit += 10;
				else
					iFit -= 5;
				break;

			case posRight:
				if (iDesiredSide == posLeft)
					iFit -= 10;
				else if (iDesiredSide == posRight)
					iFit += 10;
				break;
			}

		//	Compare

		if (iBestLayout == -1 || iFit > iBestFit)
			{
			iBestFit = iFit;
			iBestLayout = i;
			}
		}

	//	Done

	if (iBestLayout == -1)
		return false;

	if (retiIndex)
		*retiIndex = iBestLayout;

	return true;
	}

bool CGSelectorArea::FindRegionInDirection (EDirections iDir, int *retiIndex) const

//	FindRegionInDirection
//
//	Looks for the region in the specified direction from the currently selected
//	region.
//
//	Returns TRUE if the region is found.

	{
	int i;

	//	Get the current position of the selected region. If we have no selection,
	//	then we take an arbitrary point at the edge of the area.

	int xCur;
	int yCur;
	if (m_iCursor != -1)
		{
		const SEntry &Entry = m_Regions[m_iCursor];
		xCur = Entry.rcRect.left + (RectWidth(Entry.rcRect) / 2);
		yCur = Entry.rcRect.top + (RectHeight(Entry.rcRect) / 2);
		}
	else
		{
		const RECT &rcRect = GetRect();
		switch (iDir)
			{
			case moveDown:
				xCur = rcRect.left + (RectWidth(rcRect) / 2);
				yCur = rcRect.top;
				break;

			case moveLeft:
				xCur = rcRect.right;
				yCur = rcRect.top + (RectHeight(rcRect) / 2);
				break;

			case moveRight:
				xCur = rcRect.left;
				yCur = rcRect.top + (RectHeight(rcRect) / 2);
				break;

			case moveUp:
				xCur = rcRect.left + (RectWidth(rcRect) / 2);
				yCur = rcRect.bottom;
				break;

			default:
				ASSERT(false);
				return false;
			}
		}

	//	Loop over all regions and find the one the is closest to the
	//	given region.

	int iBest = -1;
	int xBestDist;
	int yBestDist;
	for (i = 0; i < m_Regions.GetCount(); i++)
		if (i != m_iCursor)
			{
			const SEntry &Entry = m_Regions[i];
			int xDist = Absolute(Entry.rcRect.left + (RectWidth(Entry.rcRect) / 2) - xCur);
			int yDist = Absolute(Entry.rcRect.top + (RectHeight(Entry.rcRect) / 2) - yCur);
			bool bCloser = false;

			//	See if this entry is better than the best so far

			switch (iDir)
				{
				case moveDown:
					bCloser = (Entry.rcRect.top > yCur
							&& (yDist >= xDist)
							&& (iBest == -1 
								|| yDist < yBestDist 
								|| (yDist == yBestDist && xDist < xBestDist)));
					break;

				case moveLeft:
					bCloser = (Entry.rcRect.left < xCur
							&& (xDist >= yDist)
							&& (iBest == -1
								|| xDist < xBestDist
								|| (xDist == xBestDist && yDist < yBestDist)));
					break;

				case moveRight:
					bCloser = (Entry.rcRect.left > xCur
							&& (xDist >= yDist)
							&& (iBest == -1
								|| xDist < xBestDist
								|| (xDist == xBestDist && yDist < yBestDist)));
					break;

				case moveUp:
					bCloser = (Entry.rcRect.top < yCur
							&& (yDist >= xDist)
							&& (iBest == -1 
								|| yDist < yBestDist 
								|| (yDist == yBestDist && xDist < xBestDist)));
					break;

				default:
					ASSERT(false);
					return false;
				}

			if (bCloser)
				{
				iBest = i;
				xBestDist = xDist;
				yBestDist = yDist;
				}
			}

	//	Found?

	if (iBest == -1)
		return false;

	if (retiIndex)
		*retiIndex = iBest;

	return true;
	}

ICCItem *CGSelectorArea::GetEntryAtCursor (void)

//	GetEntryAtCursor
//
//	Returns a struct representing the selection. The struct has the following
//	fields:
//
//	category: If this is an empty slot, this describes the category of item
//		that can be installed.
//	item: The item selected (if appropriate)
//	type: One of the following values:
//		'installedItem: An installed item (armor or device)
//		'emptySlot: An empty device slot

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (m_iCursor == -1)
		return CC.CreateNil();

	const SEntry &Entry = m_Regions[m_iCursor];

	ICCItem *pResult = CC.CreateSymbolTable();
	CCSymbolTable *pStruct = (CCSymbolTable *)pResult;

	switch (Entry.iType)
		{
		case typeEmptySlot:
			pStruct->SetStringValue(CC, CONSTLIT("type"), CONSTLIT("emptySlot"));
			switch (Entry.iSlotType)
				{
				case devPrimaryWeapon:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatWeapon));
					break;

				case devMissileWeapon:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatLauncher));
					break;

				case devShields:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatShields));
					break;

				case devDrive:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatDrive));
					break;

				case devCargo:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatCargoHold));
					break;

				case devReactor:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatReactor));
					break;

				default:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatMiscDevice));
				}
			break;

		case typeInstalledItem:
			{
			pStruct->SetStringValue(CC, CONSTLIT("type"), CONSTLIT("installedItem"));
			ICCItem *pItem = ::CreateListFromItem(CC, Entry.pItemCtx->GetItem());
			pStruct->SetValue(CC, CONSTLIT("item"), pItem);
			pItem->Discard(&CC);
			break;
			}

		default:
			ASSERT(false);
			return CC.CreateNil();
		}

	return pResult;
	}

const CItem &CGSelectorArea::GetItemAtCursor (void)

//	GetItemAtCursor
//
//	Returns the currently selected item (if any)

	{
	if (m_iCursor == -1)
		return CItem::NullItem();

	const SEntry &Entry = m_Regions[m_iCursor];
	switch (Entry.iType)
		{
		case typeInstalledItem:
			return Entry.pItemCtx->GetItem();

		default:
			return CItem::NullItem();
		}
	}

bool CGSelectorArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle mouse button

	{
	int i;

	//	All coordinates are relative to the center of the area.

	const RECT &rcRect = GetRect();
	int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;

	//	See if we've clicked on something

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		const SEntry &Entry = m_Regions[i];

		//	Convert the rect to destination coordinates

		RECT rcRegion;
		CalcRegionRect(Entry, xCenter, yCenter, &rcRegion);

		//	If we clicked in the region, signal an action

		if (x >= rcRegion.left && x < rcRegion.right
				&& y >= rcRegion.top && y < rcRegion.bottom)
			SignalAction(i);
		}

	return false;
	}

bool CGSelectorArea::MoveCursor (EDirections iDir)

//	MoveCursor
//
//	Moves the cursor in the specified direction. Returns TRUE if the cursor
//	moved successfully.

	{
	switch (iDir)
		{
		case moveDown:
		case moveLeft:
		case moveRight:
		case moveUp:
			if (!FindRegionInDirection(iDir, &m_iCursor))
				return false;

			Invalidate();
			return true;

		case moveNext:
			if (m_iCursor + 1 >= m_Regions.GetCount())
				return false;

			m_iCursor++;
			Invalidate();
			return true;

		case movePrev:
			if (m_iCursor <= 0)
				return false;

			m_iCursor--;
			Invalidate();
			return true;

		//	Invalid

		default:
			ASSERT(false);
			return false;
		}
	}

void CGSelectorArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	int i;

	const int iCornerRadius = 4;
	const int xBackMargin = 3;
	const int yBackMargin = 3;
	const int iSelectionWidth = 2;

	//	All coordinates are relative to the center of the area.

	int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;

	//	Loop over all regions and paint

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		const SEntry &Entry = m_Regions[i];

		//	Convert the rect to destination coordinates

		RECT rcRegion;
		CalcRegionRect(Entry, xCenter, yCenter, &rcRegion);

		//	Paint the background of the entry

		WORD wBackColor = (m_iCursor == i ? m_VI.GetColor(colorAreaDialogInputFocus) : m_VI.GetColor(colorAreaDialog));
		DWORD dwBackOpacity = (m_iCursor == i ? 255 : 220);

		::DrawRoundedRectTrans(Dest, 
				rcRegion.left - xBackMargin, 
				rcRegion.top - yBackMargin, 
				RectWidth(rcRegion) + 2 * xBackMargin,
				RectHeight(rcRegion) + 2 * yBackMargin,
				iCornerRadius + xBackMargin, 
				wBackColor,
				dwBackOpacity);

		//	Paint the actual region

		switch (Entry.iType)
			{
			case typeEmptySlot:
				PaintEmptySlot(Dest, rcRegion, Entry);
				break;

			case typeInstalledItem:
				PaintInstalledItem(Dest, rcRegion, Entry);
				break;
			}

		//	Draw outline if we're selected

		if (m_iCursor == i)
			{
			::DrawRoundedRectOutline(Dest,
					rcRegion.left,
					rcRegion.top,
					RectWidth(rcRegion),
					RectHeight(rcRegion),
					iCornerRadius,
					iSelectionWidth,
					m_VI.GetColor(colorAreaDialogHighlight));
			}
		}
	}

void CGSelectorArea::PaintEmptySlot (CG16bitImage &Dest, const RECT &rcRect, const SEntry &Entry)

//	PaintEmptySlot
//
//	Paints the device slot

	{
	//	Paint the slot icon

	const CG16bitImage &SlotImage = m_VI.GetImage(imageSlotIcon);
	int xIcon = rcRect.left + (RectWidth(rcRect) - ITEM_ICON_WIDTH) / 2;
	int yIcon = rcRect.top + ITEM_ENTRY_PADDING_TOP;
	DrawBltTransformed(Dest,
			xIcon + (ITEM_ICON_WIDTH / 2),
			yIcon + (ITEM_ICON_HEIGHT / 2),
			(Metric)ITEM_ICON_WIDTH / (Metric)SlotImage.GetWidth(),
			(Metric)ITEM_ICON_HEIGHT / (Metric)SlotImage.GetHeight(),
			0.0,
			SlotImage,
			0,
			0,
			SlotImage.GetWidth(),
			SlotImage.GetHeight());

	//	Paint the name of the slot

	CString sName;
	switch (Entry.iSlotType)
		{
		case devPrimaryWeapon:
			sName = CONSTLIT("weapon slot");
			break;

		case devMissileWeapon:
			sName = CONSTLIT("launcher slot");
			break;

		case devShields:
			sName = CONSTLIT("shield generator slot");
			break;

		case devCargo:
			sName = CONSTLIT("cargo hold slot");
			break;

		case devReactor:
			sName = CONSTLIT("reactor slot");
			break;

		case devDrive:
			sName = CONSTLIT("drive slot");
			break;

		default:
			sName = CONSTLIT("device slot");
		}

	RECT rcText;
	rcText.left = rcRect.left + ITEM_ENTRY_PADDING_LEFT;
	rcText.right = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
	rcText.top = yIcon + ITEM_ICON_HEIGHT;
	rcText.bottom = rcRect.bottom;

	m_VI.GetFont(fontMedium).DrawText(Dest, 
			rcText,
			m_VI.GetColor(colorTextHighlight),
			sName,
			0,
			CG16bitFont::AlignCenter);
	}

void CGSelectorArea::PaintInstalledItem (CG16bitImage &Dest, const RECT &rcRect, const SEntry &Entry)

//	PaintInstalledItem
//
//	Paints the installed item.

	{
	const CItem &Item = Entry.pItemCtx->GetItem();
	if (Item.GetType() == NULL)
		return;

	CSpaceObject *pSource = Entry.pItemCtx->GetSource();
	CInstalledArmor *pArmor = Entry.pItemCtx->GetArmor();
	CInstalledDevice *pDevice = Entry.pItemCtx->GetDevice();

	//	Paint the item icon

	bool bGrayed = (pDevice && !pDevice->IsEnabled());
	int xIcon = rcRect.left + (RectWidth(rcRect) - ITEM_ICON_WIDTH) / 2;
	int yIcon = rcRect.top + ITEM_ENTRY_PADDING_TOP;
	DrawItemTypeIcon(Dest, xIcon, yIcon, Item.GetType(), ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT, bGrayed);

	//	Paint the name of the item below.

	RECT rcText;
	rcText.left = rcRect.left + ITEM_ENTRY_PADDING_LEFT;
	rcText.right = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
	rcText.top = yIcon + ITEM_ICON_HEIGHT;
	rcText.bottom = rcRect.bottom;

	m_VI.GetFont(fontMedium).DrawText(Dest, 
			rcText,
			m_VI.GetColor(colorTextHighlight),
			Item.GetNounPhrase(nounShort | nounNoModifiers),
			0,
			CG16bitFont::AlignCenter);

	//	If this is an armor segment, then paint HP, etc.

	if (pArmor)
		{
		int x = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
		int y = rcRect.top + ITEM_ENTRY_PADDING_TOP;

		//	HP

		CString sHP = strFromInt(pArmor->GetHitPoints());
		m_VI.GetFont(fontLarge).DrawText(Dest,
				x,
				y,
				m_VI.GetColor(colorTextHighlight),
				sHP,
				CG16bitFont::AlignRight);
		y += m_VI.GetFont(fontLarge).GetHeight();

		//	Damage

		int iMaxHP = pArmor->GetMaxHP(pSource);
		if (iMaxHP != pArmor->GetHitPoints() && iMaxHP > 0)
			{
			int iPercent = ((1000 * pArmor->GetHitPoints() / iMaxHP) + 5) / 10;
			CString sPercent = strPatternSubst(CONSTLIT("%d%%"), iPercent);

			m_VI.GetFont(fontMedium).DrawText(Dest,
					x,
					y,
					m_VI.GetColor(colorTextDockWarning),
					sPercent,
					CG16bitFont::AlignRight);
			y += m_VI.GetFont(fontMedium).GetHeight();
			}

		//	Modifiers

		if (pArmor->GetMods().IsNotEmpty())
			{
			CString sMods = Item.GetEnhancedDesc(pSource);
			if (!sMods.IsBlank())
				{
				bool bIsDisadvantage = *sMods.GetASCIIZPointer() == '-';
				WORD wBackColor = (bIsDisadvantage ? m_VI.GetColor(colorAreaDisadvantage) : m_VI.GetColor(colorAreaAdvantage));
				WORD wTextColor = (bIsDisadvantage ? m_VI.GetColor(colorTextDisadvantage) : m_VI.GetColor(colorTextAdvantage));

				PaintModifier(Dest, x, y, sMods, wTextColor, wBackColor, &y);
				}
			}
		}

	//	If this is not a device, then nothing to do

	else if (pDevice == NULL)
		{
		}

	//	If this is a shield generator, then paint shield stuff

	else if (pDevice->GetCategory() == itemcatShields)
		{
		int x = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
		int y = rcRect.top + ITEM_ENTRY_PADDING_TOP;

		//	HP

		if (pDevice->IsEnabled())
			{
			int iHP;
			int iMaxHP;
			pDevice->GetStatus(pSource, &iHP, &iMaxHP);

			CString sHP = strFromInt(iHP);
			m_VI.GetFont(fontLarge).DrawText(Dest,
					x,
					y,
					m_VI.GetColor(colorTextHighlight),
					sHP,
					CG16bitFont::AlignRight);
			y += m_VI.GetFont(fontLarge).GetHeight();

			//	Shield level

			if (iMaxHP != iHP && iMaxHP > 0)
				{
				int iPercent = ((1000 * iHP / iMaxHP) + 5) / 10;
				CString sPercent = strPatternSubst(CONSTLIT("%d%%"), iPercent);

				m_VI.GetFont(fontMedium).DrawText(Dest,
						x,
						y,
						m_VI.GetColor(colorTextShields),
						sPercent,
						CG16bitFont::AlignRight);
				y += m_VI.GetFont(fontMedium).GetHeight();
				}
			}
		else
			PaintModifier(Dest, x, y, CONSTLIT("disabled"), m_VI.GetColor(colorTextNormal), DEFAULT_TRANSPARENT_COLOR, &y);

		//	Damaged

		if (pDevice->IsDamaged())
			PaintModifier(Dest, x, y, CONSTLIT("damaged"), m_VI.GetColor(colorTextDisadvantage), m_VI.GetColor(colorAreaDisadvantage), &y);

		//	Modifiers

		if (pDevice->GetEnhancements() != NULL)
			{
			CString sMods = pDevice->GetEnhancedDesc(pSource, &Item);
			if (!sMods.IsBlank())
				{
				bool bIsDisadvantage = *sMods.GetASCIIZPointer() == '-';
				WORD wBackColor = (bIsDisadvantage ? m_VI.GetColor(colorAreaDisadvantage) : m_VI.GetColor(colorAreaAdvantage));
				WORD wTextColor = (bIsDisadvantage ? m_VI.GetColor(colorTextDisadvantage) : m_VI.GetColor(colorTextAdvantage));

				PaintModifier(Dest, x, y, sMods, wTextColor, wBackColor, &y);
				}
			}
		}
	}

void CGSelectorArea::PaintModifier (CG16bitImage &Dest, int x, int y, const CString &sText, WORD wColor, WORD wBackColor, int *rety)

//	PaintModifier
//
//	Paints a modifier block.

	{
	int cx = m_VI.GetFont(fontSmall).MeasureText(sText);
	if (wBackColor != DEFAULT_TRANSPARENT_COLOR)
		Dest.Fill(x - cx - 8,
				y,
				cx + 8,
				m_VI.GetFont(fontSmall).GetHeight(),
				wBackColor);

	m_VI.GetFont(fontSmall).DrawText(Dest,
			x - cx - 4,
			y,
			wColor,
			sText);

	if (rety)
		*rety = y + m_VI.GetFont(fontSmall).GetHeight() + 1;
	}

void CGSelectorArea::SetRegions (CSpaceObject *pSource, EConfigurations iConfig)

//	SetRegions
//
//	Sets the regions based on the given configuration.

	{
	CleanUp();

	m_pSource = pSource;

	switch (iConfig)
		{
		case configArmor:
			SetRegionsFromArmor(pSource);
			break;

		case configMiscDevices:
			SetRegionsFromMiscDevices(pSource);
			break;

		case configWeapons:
			SetRegionsFromWeapons(pSource);
			break;
		}

	Invalidate();
	}

void CGSelectorArea::SetRegionsFromArmor (CSpaceObject *pSource)

//	SetRegionsFromArmor
//
//	Generates regions showing armor and shields for the given ship.

	{
	int i;
	ASSERT(pSource);
	if (pSource == NULL)
		return;

	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return;

	CShipClass *pClass = pShip->GetClass();

	//	Compute some metrics.
	//
	//	We place the shield generator in the center and the armor segments in a
	//	circle around it.

	const RECT &rcRect = GetRect();
	int cxArea = RectWidth(rcRect);
	int cyArea = RectHeight(rcRect);

	const int iRadius = WIDE_COLUMN_SPACING;

	//	Now add all the armor segments

	for (i = 0; i < pShip->GetArmorSectionCount(); i++)
		{
		SEntry *pEntry = m_Regions.Insert();
		CInstalledArmor *pArmor = pShip->GetArmorSection(i);

		pEntry->iType = typeInstalledItem;
		pEntry->pItemCtx = new CItemCtx(pShip, pArmor);

		//	Position the armor segment in a circle (add 90 degrees because the
		//	ship image points up).

		CShipClass::HullSection *pSection = pClass->GetHullSection(i);
		int iCenterAngle = 90 + AngleMiddle(pSection->iStartAt, AngleMod(pSection->iStartAt + pSection->iSpan));

		int xCenter;
		int yCenter;
		IntPolarToVector(iCenterAngle, iRadius, &xCenter, &yCenter);

		pEntry->rcRect.left = xCenter - (ITEM_ENTRY_WIDTH / 2);
		pEntry->rcRect.top = -yCenter - (ITEM_ENTRY_HEIGHT / 2);
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}

	//	Add the shield generator last

	SEntry *pEntry = m_Regions.Insert();
	CInstalledDevice *pShields = pShip->GetNamedDevice(devShields);
	if (pShields)
		{
		pEntry->iType = typeInstalledItem;
		pEntry->pItemCtx = new CItemCtx(pShip, pShields);
		}
	else
		{
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devShields;
		}

	pEntry->rcRect.left = -ITEM_ENTRY_WIDTH / 2;
	pEntry->rcRect.top = -ITEM_ENTRY_HEIGHT / 2;
	pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
	pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
	}

void CGSelectorArea::SetRegionsFromMiscDevices (CSpaceObject *pSource)

//	SetRegionsFromMiscDevices
//
//	Generates regions showing misc devices (including reactor, drive, 
//	and cargo hold).

	{
	int i;
	ASSERT(pSource);
	if (pSource == NULL)
		return;

	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return;

	CShipClass *pClass = pShip->GetClass();

	//	Count the number of miscellaneous devices with 0
	//	slots (because we may need to bump them).

	int iSlottedDevices = 0;
	int iNonSlotDevices = 0;
	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty() || pDevice->GetSlotCategory() != itemcatMiscDevice)
			continue;

		if (pDevice->GetClass()->GetSlotsRequired() > 0)
			iSlottedDevices++;
		else
			iNonSlotDevices++;
		}

	//	We try to fit all other devices (and placeholders) before we add a
	//	non-slotted device.

	int iNonSlotDeviceSlotsAvail = Max(0, MISC_DEVICES_LAYOUT_COUNT - 4 - iSlottedDevices);

	//	Create a region for each device.

	bool bHasReactor = false;
	bool bHasDrive = false;
	bool bHasCargo = false;
	int iNextUnamedSlot = FIRST_UNNAMED_SLOT_INDEX;
	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty())
			continue;

		//	If this is a non-slot device, make sure we have room.

		if (pDevice->GetClass()->GetSlotsRequired() == 0)
			{
			if (iNonSlotDeviceSlotsAvail == 0)
				continue;

			iNonSlotDevices--;
			}

		//	Figure out the layout descriptor

		const SLayoutDesc *pLayout = NULL;
		switch (pDevice->GetSlotCategory())
			{
			case itemcatCargoHold:
				pLayout = &g_MiscDevicesLayout[CARGO_SLOT_INDEX];
				bHasCargo = true;
				break;

			case itemcatDrive:
				pLayout = &g_MiscDevicesLayout[DRIVE_SLOT_INDEX];
				bHasDrive = true;
				break;

			case itemcatMiscDevice:
				if (iNextUnamedSlot < MISC_DEVICES_LAYOUT_COUNT)
					pLayout = &g_MiscDevicesLayout[iNextUnamedSlot++];
				break;

			case itemcatReactor:
				pLayout = &g_MiscDevicesLayout[REACTOR_SLOT_INDEX];
				bHasReactor = true;
				break;
			}

		//	Create the region (but only if we have a layout position
		//	for it).

		if (pLayout)
			{
			SEntry *pEntry = m_Regions.Insert();
			pEntry->iType = typeInstalledItem;
			pEntry->pItemCtx = new CItemCtx(pShip, pDevice);

			pEntry->rcRect.left = pLayout->xLeft;
			pEntry->rcRect.top = pLayout->yTop;
			pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
			pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
			}
		}

	//	Add empty slots, if necessary

	if (!bHasReactor)
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[REACTOR_SLOT_INDEX];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devReactor;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}

	if (!bHasDrive)
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[DRIVE_SLOT_INDEX];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devDrive;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}

	if (!bHasCargo)
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[CARGO_SLOT_INDEX];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devCargo;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}

	//	Always add a misc device slot

	if (iNextUnamedSlot < MISC_DEVICES_LAYOUT_COUNT)
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[iNextUnamedSlot++];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devNone;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}
	}

void CGSelectorArea::SetRegionsFromWeapons (CSpaceObject *pSource)

//	SetRegionsFromWeapons
//
//	Creates regions based on installed weapons.

	{
	int i;
	ASSERT(pSource);
	if (pSource == NULL)
		return;

	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return;

	CShipClass *pClass = pShip->GetClass();

	//	Keep track of layouts that have already been used.

	TArray<bool> SlotStatus;
	SlotStatus.InsertEmpty(MISC_DEVICES_LAYOUT_COUNT);
	for (i = 0; i < MISC_DEVICES_LAYOUT_COUNT; i++)
		SlotStatus[i] = true;

	//	Create a region for each weapon.

	bool bHasLauncher = false;
	int iIndex;
	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty() 
				|| (pDevice->GetSlotCategory() != itemcatWeapon 
					&& pDevice->GetSlotCategory() != itemcatLauncher))
			continue;

		if (pDevice->GetCategory() == itemcatLauncher)
			bHasLauncher = true;

		//	If the device already has a position index, then use that (assuming
		//	it's free).

		iIndex = pDevice->GetSlotPosIndex();
		if (iIndex != -1 && !SlotStatus[iIndex])
			iIndex = -1;

		//	If we don't have an assigned slot, figure it out.

		if (iIndex == -1)
			{
			if (!FindLayoutForPos(pDevice->GetPosOffset(pShip), SlotStatus, &iIndex))
				continue;

			//	Remember so we stay in this location.

			pDevice->SetSlotPosIndex(iIndex);
			}

		//	Create the region

		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[iIndex];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeInstalledItem;
		pEntry->pItemCtx = new CItemCtx(pShip, pDevice);

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;

		//	Mark the layout as used

		SlotStatus[iIndex] = false;
		}

	//	If necessary, add a launcher slot

	if (!bHasLauncher
			&& FindLayoutForPos(CVector(), SlotStatus, &iIndex))
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[iIndex];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devMissileWeapon;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;

		SlotStatus[iIndex] = false;
		}

	//	Always add an empty slot

	if (FindLayoutForPos(CVector(), SlotStatus, &iIndex))
		{
		const SLayoutDesc *pLayout = &g_MiscDevicesLayout[iIndex];

		SEntry *pEntry = m_Regions.Insert();
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devPrimaryWeapon;

		pEntry->rcRect.left = pLayout->xLeft;
		pEntry->rcRect.top = pLayout->yTop;
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;

		SlotStatus[iIndex] = false;
		}
	}

void CGSelectorArea::Update (void)

//	Update
//
//	Update the area

	{
	}
