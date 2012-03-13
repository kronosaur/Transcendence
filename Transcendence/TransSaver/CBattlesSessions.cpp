//	CBattlesSessions.cpp
//
//	CBattlesSession class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

const int MAX_TIME_WITH_ONE_SHIP =				(g_TicksPerSecond * 90);
const int MAX_TIME_TO_SHOW_LOGO =				(g_TicksPerSecond * 60);

const DWORD UNID_PLAYER_SOVEREIGN =				0x00001001;
const DWORD UNID_PIRATES_SOVEREIGN =			0x00001005;

#define ATTRIB_GENERIC_SHIP_CLASS				CONSTLIT("genericClass")
#define DATA_BATTLES_SESSION					CONSTLIT("battlesSession")

void CBattlesSession::CreateBattle (DWORD dwFlags, DWORD dwNewShipClass, DWORD dwSovereign, CSpaceObject *pShipDestroyed)

//	CreateBattle
//
//	Makes sure that there is a proper battle.
//
//	If dwNewShipClass is non-zero, then we create a new ship of the given class
//	and dwSovereign is the sovereign of the new ship.
//
//	If pShipDestroyed is non-zero, then we don't count that ship when computing
//	the battle.

	{
	int i, j;
	const int SOVEREIGN_COUNT = 2;

	//	Make sure each sovereign has a ship

	CSovereign *pSovereigns[SOVEREIGN_COUNT];
	pSovereigns[0] = g_pUniverse->FindSovereign(UNID_PLAYER_SOVEREIGN);
	pSovereigns[1] = g_pUniverse->FindSovereign(UNID_PIRATES_SOVEREIGN);

	//	Init ships

	CShip *pShips[SOVEREIGN_COUNT];
	for (i = 0; i < SOVEREIGN_COUNT; i++)
		pShips[i] = NULL;

	//	Sovereign of POV

	DWORD dwCurSovereign = (dwSovereign ? dwSovereign : (g_pUniverse->GetPOV() ? g_pUniverse->GetPOV()->GetSovereign()->GetUNID() : UNID_PLAYER_SOVEREIGN));

	//	Look for the surviving ships

	int iTotalShips = 0;
	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& pObj != pShipDestroyed
				&& !pObj->GetData(DATA_BATTLES_SESSION).IsBlank())
			{
			for (j = 0; j < SOVEREIGN_COUNT; j++)
				if (pObj->GetSovereign() == pSovereigns[j])
					{
					if (pShips[j] == NULL)
						pShips[j] = pObj->AsShip();
					break;
					}

			iTotalShips++;
			}
		}

	//	Make sure we create at least one ship per sovereign

	for (i = 0; i < SOVEREIGN_COUNT; i++)
		if (pShips[i] == NULL || (dwNewShipClass && dwCurSovereign == pSovereigns[i]->GetUNID()))
			CreateRandomShips((dwCurSovereign == pSovereigns[i]->GetUNID() ? dwNewShipClass : 0), 
					pSovereigns[i],
					dwFlags,
					&pShips[i]);

	//	Make sure every ship has an order to attack someone

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& pObj != pShipDestroyed)
			{
			CShip *pShip = pObj->AsShip();
			if (pShip)
				{
				IShipController *pController = pShip->GetController();

				CSpaceObject *pTarget;
				IShipController::OrderTypes iOrder = pController->GetCurrentOrderEx(&pTarget);
				if ((pShipDestroyed && pTarget == pShipDestroyed) 
						|| iOrder == IShipController::orderNone
						|| iOrder == IShipController::orderGate)
					{
					pController->CancelAllOrders();

					//	Figure out our sovereign

					int iIndex = 0;
					for (j = 0; j < SOVEREIGN_COUNT; j++)
						if (pShip->GetSovereign() == pSovereigns[j])
							{
							iIndex = j;
							break;
							}

					//	Attack a ship of an enemy sovereign

					pController->AddOrder(IShipController::orderDestroyTarget, pShips[(iIndex + 1) % SOVEREIGN_COUNT], 0);
					}
				}
			}
		}

	//	Chance the POV if necessary

	if (g_pUniverse->GetPOV() == pShipDestroyed 
			|| g_pUniverse->GetPOV()->IsDestroyed()
			|| g_pUniverse->GetPOV()->AsShip() == NULL
			|| dwNewShipClass != 0)
		{
		//	Pick a POV of the same sovereign

		for (i = 0; i < SOVEREIGN_COUNT; i++)
			if (pSovereigns[i]->GetUNID() == dwCurSovereign)
				{
				g_pUniverse->SetPOV(pShips[i]);
				break;
				}

		OnPOVSet(g_pUniverse->GetPOV());
		}

	m_iLastChange = g_pUniverse->GetTicks();
	}

void CBattlesSession::CreateLogoAnimation (IAnimatron **retpAnimatron)

//	CreateLogoAnimation
//
//	Shows the logo, version, and web site URL

	{
	int cxScreen = m_HI.GetScreen().GetWidth();
	int cyScreen = m_HI.GetScreen().GetHeight();
	const CVisualPalette &VI = m_HI.GetVisuals();

	int iDuration = 600;
	int iInterLineDelay = 1;
	int iDelay = 0;
	int x = cxScreen / 2;
	int y = cyScreen - cyScreen / 3;

	//	Fonts

	const CG16bitFont &LogoFont = VI.GetFont(fontLogoTitle);
	const CG16bitFont &TitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SmallFont = VI.GetFont(fontSmall);
	WORD wHighlightColor = VI.GetColor(colorTextHighlight);
	WORD wNormalColor = VI.GetColor(colorTextNormal);
	WORD wFadeColor = VI.GetColor(colorTextFade);

	//	Create sequencer to hold everything

	CAniSequencer *pSeq = new CAniSequencer;

	//	Show the logo

	CString sTitle = CONSTLIT("TRANSCENDENCE");
	IAnimatron *pText;
	CAniText::Create(sTitle,
			CVector((Metric)x, (Metric)y),
			&LogoFont,
			CG16bitFont::AlignCenter,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 30, 30);
	pSeq->AddTrack(pText, iDelay);

	CAniText::Create(CONSTLIT("®"),
			CVector((Metric)x + LogoFont.MeasureText(sTitle) / 2, (Metric)y + MediumFont.GetAscent() / 2),
			&MediumFont,
			0,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 30, 30);
	pSeq->AddTrack(pText, iDelay);

	y += LogoFont.GetHeight();
	iDelay += iInterLineDelay * 3;

	//	Version and copyright

	CString sText = strPatternSubst(CONSTLIT("%s %s"), m_Model.GetProductName(), m_Model.GetVersion());
	CAniText::Create(sText,
			CVector((Metric)x, (Metric)y),
			&MediumFont,
			CG16bitFont::AlignCenter,
			wNormalColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 30, 30);
	pSeq->AddTrack(pText, iDelay);

	y += MediumFont.GetHeight();
	iDelay += iInterLineDelay;

	CAniText::Create(m_Model.GetCopyright(),
			CVector((Metric)x, (Metric)y),
			&MediumFont,
			CG16bitFont::AlignCenter,
			wNormalColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 30, 30);
	pSeq->AddTrack(pText, iDelay);

	y += MediumFont.GetHeight();
	iDelay += iInterLineDelay;

	//	URL

	y += TitleFont.GetHeight();
	CAniText::Create(CONSTLIT("http://transcendence-game.com"),
			CVector((Metric)x, (Metric)y),
			&TitleFont,
			CG16bitFont::AlignCenter,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 30, 30);
	pSeq->AddTrack(pText, iDelay);

	y += TitleFont.GetHeight();
	iDelay += iInterLineDelay;

	//	Done

	*retpAnimatron = pSeq;
	}

void CBattlesSession::CreateRandomShips (DWORD dwClass, CSovereign *pSovereign, DWORD dwFlags, CShip **retpShip)

//	CreateRandomShips
//
//	Creates some random ships of the given class and sovereign.
//
//	If dwClass is 0, then we pick a random class.

	{
	int i;

	//	Figure out the class

	CShipClass *pShipClass;
	if (dwClass == 0
			|| (pShipClass = g_pUniverse->FindShipClass(dwClass)) == NULL)
		{
		do
			pShipClass = g_pUniverse->GetShipClass(mathRandom(0, g_pUniverse->GetShipClassCount()-1));
		while (pShipClass->IsPlayerShip()
				|| pShipClass->IsVirtual()
				|| !pShipClass->HasAttribute(ATTRIB_GENERIC_SHIP_CLASS));
		}

	//	Normally we create a single ship, but sometimes we create lots

	int iCount;
	if ((dwFlags & FLAG_FORCE_MULTIPLE_SHIPS)
			|| mathRandom(1, 100) <= 10)
		{
		int iScore = Max(1, Min(pShipClass->GetScore(), 5000));
		int iAveCount = Min(5000 / iScore, 100);
		iCount = mathRandom(Max(1, iAveCount / 2), iAveCount + (iAveCount / 4));
		}
	else
		iCount = 1;

	//	Create the ships

	for (i = 0; i < iCount; i++)
		{
		CShip *pShip;
		if (m_pSystem->CreateShip(pShipClass->GetUNID(),
				NULL,
				NULL,
				pSovereign,
				PolarToVector(mathRandom(0, 359), mathRandom(250, 2500) * g_KlicksPerPixel),
				NullVector,
				mathRandom(0, 359),
				NULL,
				NULL,
				&pShip) != NOERROR)
			return;

		//	Mark the ship

		pShip->SetData(DATA_BATTLES_SESSION, CONSTLIT("True"));

		if (retpShip && i == 0)
			*retpShip = pShip;
		}
	}

void CBattlesSession::CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron)

//	CreateShipDescAnimation
//
//	Creates animation describing the given ship

	{
	int i, j;

	int cxScreen = m_HI.GetScreen().GetWidth();
	int cyScreen = m_HI.GetScreen().GetHeight();
	const CVisualPalette &VI = m_HI.GetVisuals();

	int iDuration = 600;
	int iInterLineDelay = 1;
	int iDelay = 0;
	int x = (cxScreen / 2) + (cxScreen / 6);
	int y = cyScreen - cyScreen / 3;

	//	Fonts

	const CG16bitFont &TitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SmallFont = VI.GetFont(fontSmall);
	WORD wHighlightColor = VI.GetColor(colorTextHighlight);
	WORD wNormalColor = VI.GetColor(colorTextNormal);
	WORD wFadeColor = VI.GetColor(colorTextFade);

	//	Create sequencer to hold everything

	CAniSequencer *pSeq = new CAniSequencer;

	//	Show the ship class

	CString sClassName = strToLower(pShip->GetName());
	int cyClassName;
	int cxClassName = TitleFont.MeasureText(sClassName, &cyClassName);
	int cySectionSpacing = cyClassName / 6;

	IAnimatron *pText;
	CAniText::Create(sClassName,
			CVector((Metric)x, (Metric)y),
			&TitleFont,
			CG16bitFont::AlignCenter,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 0, 30);
	pSeq->AddTrack(pText, 0);

	y += cyClassName + cySectionSpacing;
	iDelay += iInterLineDelay * 3;

	//	Weapons label

	CAniText::Create(CONSTLIT("WEAPONS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + MediumFont.GetAscent() - SmallFont.GetAscent())),
			&SmallFont,
			CG16bitFont::AlignRight,
			wFadeColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	//	Collect duplicate weapons

	struct SWeaponDesc
		{
		CString sWeaponName;
		int iCount;
		};

	TArray<SWeaponDesc> WeaponList;

	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty())
			continue;

		if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
			{
			CString sName = pDevice->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize);

			//	Look for the weapon in the list

			bool bFound = false;
			for (j = 0; j < WeaponList.GetCount() && !bFound; j++)
				if (strEquals(WeaponList[j].sWeaponName, sName))
					{
					WeaponList[j].iCount++;
					bFound = true;
					}

			//	Add if necessary

			if (!bFound)
				{
				SWeaponDesc *pWeapon = WeaponList.Insert();
				pWeapon->sWeaponName = sName;
				pWeapon->iCount = 1;
				}
			}
		}

	//	Output weapon list

	if (WeaponList.GetCount() == 0)
		{
		CAniText::Create(CONSTLIT("None"),
				CVector((Metric)x + cyClassName / 4, (Metric)y),
				&MediumFont,
				0,
				wHighlightColor,
				&pText);
		pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
		pSeq->AddTrack(pText, iDelay);

		y += MediumFont.GetHeight();
		iDelay += iInterLineDelay;
		}
	else
		{
		for (i = 0; i < WeaponList.GetCount() && i < 6; i++)
			{
			CAniText::Create((WeaponList[i].iCount == 1 ? WeaponList[i].sWeaponName
						: strPatternSubst(CONSTLIT("%s (x%d)"), WeaponList[i].sWeaponName, WeaponList[i].iCount)),
					CVector((Metric)x + cyClassName / 4, (Metric)y),
					&MediumFont,
					0,
					wHighlightColor,
					&pText);
			pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
			pSeq->AddTrack(pText, iDelay);

			y += MediumFont.GetHeight();
			iDelay += iInterLineDelay;
			}
		}

	//	Shields

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("SHIELDS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + MediumFont.GetAscent() - SmallFont.GetAscent())),
			&SmallFont,
			CG16bitFont::AlignRight,
			wFadeColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	CInstalledDevice *pShields = pShip->GetNamedDevice(devShields);

	CAniText::Create((pShields ? pShields->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&MediumFont,
			0,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += MediumFont.GetHeight();
	iDelay += iInterLineDelay;

	//	Armor

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("ARMOR:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + MediumFont.GetAscent() - SmallFont.GetAscent())),
			&SmallFont,
			CG16bitFont::AlignRight,
			wFadeColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	int iCount = pShip->GetArmorSectionCount();
	CInstalledArmor *pArmor = (iCount > 0 ? pShip->GetArmorSection(0) : NULL);
	CString sArmor = (pArmor ? pArmor->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize | nounShort) : NULL_STR);

	CAniText::Create((pArmor ? strPatternSubst(CONSTLIT("%s (x%d)"), sArmor, iCount) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&MediumFont,
			0,
			wHighlightColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += MediumFont.GetHeight();
	iDelay += iInterLineDelay;

	//	Done

	*retpAnimatron = pSeq;
	}

void CBattlesSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	g_pUniverse->DestroySystem(m_pSystem);
	}

ALERROR CBattlesSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	m_iState = stateNormal;
	m_iLastChange = g_pUniverse->GetTicks();
	m_iLastShipDesc = g_pUniverse->GetTicks();

	//	Create an empty system

	if (error = g_pUniverse->CreateEmptyStarSystem(&m_pSystem))
		return error;

	g_pUniverse->SetCurrentSystem(m_pSystem);

	//	Make sure we have a battle.
	//	(We start with a large number of ships)

	CreateBattle(FLAG_FORCE_MULTIPLE_SHIPS);

	return NOERROR;
	}

void CBattlesSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint the frame

	{
	RECT rcViewport;
	rcViewport.left = 0;
	rcViewport.right = Screen.GetWidth();
	rcViewport.top = 0;
	rcViewport.bottom = Screen.GetHeight();

	//	Paint the universe

	g_pUniverse->PaintPOV(Screen, rcViewport, false);
	}

void CBattlesSession::OnPOVSet (CSpaceObject *pObj)

//	OnPOVSet
//
//	POV has changed

	{
	if (pObj == NULL)
		return;

	CShip *pShip = pObj->AsShip();
	if (pShip == NULL)
		return;

	ShowShipDescAnimation(pShip);
	}

void CBattlesSession::OnSize (int cxWidth, int cyHeight)

//	OnSize
//
//	Size changed

	{
	}

void CBattlesSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update the frame

	{
	int i;

	//	Update the universe

	g_pUniverse->Update(g_SecondsPerUpdate);

	//	If the same ship has been here for a while, then create a new ship

	if (g_pUniverse->GetTicks() - m_iLastChange > MAX_TIME_WITH_ONE_SHIP)
		{
		CShip *pShip = g_pUniverse->GetPOV()->AsShip();
		if (pShip)
			{
			pShip->Destroy(removedFromSystem, CDamageSource());
			CreateBattle();
			}
		}

	//	Slight HACK: If the current POV is not a ship, then create a new one

	else if (g_pUniverse->GetPOV()->GetCategory() != CSpaceObject::catShip)
		CreateBattle();

	//	Otherwise, if any ships were destroyed, then replenish

	else
		{
		for (i = 0; i < m_pSystem->GetDestroyedObjectCount(); i++)
			if (m_pSystem->GetDestroyedObject(i)->GetCategory() == CSpaceObject::catShip)
				{
				CreateBattle();

				//	If a ship was destroyed, reset the timer, since clearly
				//	exciting things are happening

				m_iLastChange = g_pUniverse->GetTicks();
				break;
				}
		}

	//	If we're showing the logo, see if we're done

	if (m_iState == stateShowingLogo && !m_HI.GetReanimator().IsRunning())
		ShowShipDescAnimation();

	//	If we've gone for a while with no description, show the logo

	else if (g_pUniverse->GetTicks() - m_iLastShipDesc > MAX_TIME_TO_SHOW_LOGO)
		{
		CReanimator &Reanimator = m_HI.GetReanimator();
		Reanimator.StopAll();

		IAnimatron *pAni;
		CreateLogoAnimation(&pAni);
		DWORD dwPerformance = Reanimator.AddPerformance(pAni);
		Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

		m_iLastShipDesc = g_pUniverse->GetTicks();
		m_iState = stateShowingLogo;
		}

	//	Invalidate

	HIInvalidate();
	}

void CBattlesSession::ShowShipDescAnimation (CShip *pShip)

//	ShowShipDescAnimation
//
//	Shows the ship description

	{
	if (pShip == NULL)
		{
		CSpaceObject *pPOV = g_pUniverse->GetPOV();
		if (pPOV == NULL)
			return;

		pShip = pPOV->AsShip();
		if (pShip == NULL)
			return;
		}

	CReanimator &Reanimator = m_HI.GetReanimator();
	Reanimator.StopAll();

	IAnimatron *pAni;
	CreateShipDescAnimation(pShip, &pAni);
	DWORD dwPerformance = Reanimator.AddPerformance(pAni);
	Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	m_iLastShipDesc = g_pUniverse->GetTicks();
	m_iState = stateNormal;
	}
