//	CRegKey.cpp
//
//	CRegKey class
//	Copyright (c) 2003 by NeuroHack, Inc. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

CRegKey::CRegKey (void) : m_hKey(NULL)

//	CRegKey constructor

	{
	}

CRegKey::~CRegKey (void)

//	CRegKey destructor

	{
	CleanUp();
	}

void CRegKey::CleanUp (void)

//	CleanUp
//
//	Free registry key

	{
	if (m_hKey)
		{
		::RegCloseKey(m_hKey);
		m_hKey = NULL;
		}
	}

bool CRegKey::FindStringValue (const CString &sValue, CString *retsData)

//	FindStringValue
//
//	Find a string value

	{
	DWORD dwType;
	DWORD dwSize;

	if (::RegQueryValueEx(m_hKey,
			sValue.GetASCIIZPointer(),
			NULL,
			&dwType,
			NULL,
			&dwSize) != ERROR_SUCCESS)
		return false;

	if (dwType != REG_SZ)
		return false;

	char *pData = retsData->GetWritePointer(dwSize-1);
	if (::RegQueryValueEx(m_hKey,
			sValue.GetASCIIZPointer(),
			NULL,
			NULL,
			(BYTE *)pData,
			&dwSize) != ERROR_SUCCESS)
		return false;

	return true;
	}

ALERROR CRegKey::OpenUserAppKey (const CString &sCompany, 
							     const CString &sAppName,
							     CRegKey *retKey)

//	OpenUserAppKey
//
//	Opens the key for the application's user data

	{
	//	Open the Software key

	HKEY hSoftware;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software",
			0,
			KEY_READ | KEY_WRITE,
			&hSoftware) != ERROR_SUCCESS)
		return ERR_FAIL;

	//	Open/create a key for the company

	HKEY hCompany;
	if (::RegCreateKeyEx(hSoftware,
			sCompany.GetASCIIZPointer(),
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_READ | KEY_WRITE,
			NULL,
			&hCompany,
			NULL) != ERROR_SUCCESS)
		{
		::RegCloseKey(hSoftware);
		return ERR_FAIL;
		}

	//	Open/create a key for the app

	HKEY hApp;
	if (::RegCreateKeyEx(hCompany,
			sAppName.GetASCIIZPointer(),
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_READ | KEY_WRITE,
			NULL,
			&hApp,
			NULL) != ERROR_SUCCESS)
		{
		::RegCloseKey(hCompany);
		::RegCloseKey(hSoftware);
		return ERR_FAIL;
		}

	//	Done

	::RegCloseKey(hCompany);
	::RegCloseKey(hSoftware);

	retKey->CleanUp();
	retKey->m_hKey = hApp;

	return NOERROR;
	}

void CRegKey::SetStringValue (const CString &sValue, const CString &sData)

//	SetStringValue
//
//	Sets a string value under this key

	{
	::RegSetValueEx(m_hKey,
			sValue.GetASCIIZPointer(),
			0,
			REG_SZ,
			(BYTE *)sData.GetASCIIZPointer(),
			sData.GetLength() + 1);
	}

