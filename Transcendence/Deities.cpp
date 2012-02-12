//	Deities.cpp
//
//	Implements relationship with deities

#include "PreComp.h"
#include "Transcendence.h"

#define INITIAL_INVOKE_TIME						1800
#define INVOKE_INTERVAL							1800

int GetDeity (CPower *pPower)
	{
	if (pPower->HasModifier(CONSTLIT("Domina")))
		return deityDomina;
	else if (pPower->HasModifier(CONSTLIT("Oracus")))
		return deityOracus;
	else
		{
		ASSERT(false);
		return -1;
		}
	}

void CPlayerShipController::AddBasicPowers (void)

//	AddBasicPowers
//
//	Adds all basic (intrinsic) powers

	{
	for (int i = 0; i < g_pUniverse->GetPowerCount(); i++)
		{
		CPower *pPower = g_pUniverse->GetPower(i);

		if (!pPower->HasModifier(CONSTLIT("Advanced")))
			m_Deity[::GetDeity(pPower)].KnownPowers.AppendElement((int)pPower, NULL);
		}

	UpdatePowerTable();
	}

void CPlayerShipController::AddOffering (const CItem &Item)

//	AddOffering
//
//	Offers items to deity

	{
	}

void CPlayerShipController::AddPower (CPower *pPower)

//	AddPower
//
//	Adds a power

	{
	}

int CPlayerShipController::GetDeity (void)

//	GetDeity
//
//	Get the deity that we are associated with

	{
	if (m_Deity[deityOracus].iRel > 0)
		return deityOracus;
	else
		return deityDomina;
	}

void CPlayerShipController::IncRelationship (int iDeity, int iChange)

//	IncRelationship
//
//	Increases relationship counter

	{
	m_Deity[iDeity].iRel += iChange;
	}

void CPlayerShipController::InitDeities (void)

//	InitDeities
//
//	Initializes deities data structure

	{
	for (int i = deityFirst; i < deityCount; i++)
		{
		m_Deity[i].iRel = 0;
		m_Deity[i].iNextInvoke = g_pUniverse->GetTicks() + INITIAL_INVOKE_TIME;
		}

	AddBasicPowers();
	}

void CPlayerShipController::Invoke (const CString &sKey)

//	Invoke
//
//	Invoke a power

	{
	ALERROR error;
	CPower *pPower;

	if (error = m_AvailPowers.Lookup(sKey, (CObject **)&pPower))
		return;

	InvokePower(pPower);
	}

void CPlayerShipController::InvokePower (CPower *pPower)

//	InvokePower
//
//	Invokes the given power

	{
	int iDeity = GetDeity();

	//	See if we can invoke powers at this point.

	if (m_Deity[iDeity].iNextInvoke > g_pUniverse->GetTicks() || m_Deity[iDeity].iRel < 0)
		{
		if (iDeity == deityDomina)
			{
			if (m_Deity[iDeity].iRel < 0)
				{
				m_pTrans->DisplayMessage(CONSTLIT("Domina: \"I cannot help those who cannot help themselves\""));
				m_Deity[iDeity].iRel -= 100;
				}
			else if (m_Deity[iDeity].iRel == 0)
				{
				m_pTrans->DisplayMessage(CONSTLIT("Domina: \"Allowing you to struggle is the best help I can offer\""));
				m_Deity[iDeity].iRel -= 25;
				}
			else
				{
				m_pTrans->DisplayMessage(CONSTLIT("Domina: \"Do not rely on my powers, child\""));
				m_Deity[iDeity].iRel -= 20;
				}
			}
		else if (iDeity == deityOracus)
			m_pTrans->DisplayMessage(CONSTLIT("Oracus ignores your invocation"));
		return;
		}

	//	Do we have enough to invoke?

	if (pPower->GetInvokeCost() > m_Deity[iDeity].iRel)
		return;

	//	Otherwise, it is OK to invoke

	CString sError;
	pPower->Invoke(m_pShip, m_pTarget, &sError);
	if (!sError.IsBlank())
		m_pTrans->DisplayMessage(sError);

	//	Consume relationship

	int iPower = pPower->GetInvokeCost();
	m_Deity[iDeity].iRel -= iPower;
	if (iPower == 0)
		m_Deity[iDeity].iNextInvoke = g_pUniverse->GetTicks() + INVOKE_INTERVAL;
	else
		{
		int iInterval = Min(INVOKE_INTERVAL * 5, (INVOKE_INTERVAL * iPower / 100));
		m_Deity[iDeity].iNextInvoke = g_pUniverse->GetTicks() + iInterval;
		}
	}

bool CPlayerShipController::IsValidInvokeKey (const CString &sKey)

//	IsValidInvokeKey
//
//	Returns TRUE if the key is valid for a power

	{
	return (m_AvailPowers.Lookup(sKey, NULL) == NOERROR);
	}

void CPlayerShipController::UpdatePowerTable (void)

//	UpdatePowerTable
//
//	Updates the table of available powers

	{
	//	First we figure out what deity we have chosen

	int iDeity = GetDeity();

	//	Add all known powers that we have enough energy to invoke

	m_AvailPowers.RemoveAll();
	for (int i = 0; i < m_Deity[iDeity].KnownPowers.GetCount(); i++)
		{
		CPower *pPower = (CPower *)m_Deity[iDeity].KnownPowers.GetElement(i);

		//	We can't invoke powers that we don't have energy for

		if (pPower->GetInvokeCost() > 0 
				&& pPower->GetInvokeCost() > m_Deity[iDeity].iRel)
			continue;

		//	We can't invoke automatic powers

		if (pPower->HasModifier(CONSTLIT("Automatic")))
			continue;

		//	Add this power to our list; index it by access key

		m_AvailPowers.AddEntry(pPower->GetInvokeKey(), pPower);
		}
	}

