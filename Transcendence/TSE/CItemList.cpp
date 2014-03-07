//	CItemList.cpp
//
//	CItemList object

#include "PreComp.h"

CItemList::CItemList (void)

//	CItemList constructor

	{
	}

CItemList::CItemList (const CItemList &Src)

//	CItemList constructor

	{
	Copy(Src);
	}

CItemList::~CItemList (void)

//	CItemList destructor

	{
	DeleteAll();
	}

CItemList &CItemList::operator= (const CItemList &Src)

//	CItemList assign operator

	{
	DeleteAll();
	Copy(Src);
	return *this;
	}

void CItemList::AddItem (const CItem &Item)

//	AddItem
//
//	Adds an item to the list

	{
	m_List.Insert(new CItem(Item));
	}

void CItemList::Copy (const CItemList &Src)

//	Copy
//
//	Copy stuff. We assume that we are clean.

	{
	int i;

	if (Src.GetCount() > 0)
		{
		m_List.InsertEmpty(Src.GetCount());

		for (i = 0; i < m_List.GetCount(); i++)
			m_List[i] = new CItem(Src.GetItem(i));
		}
	}

void CItemList::DeleteAll (void)

//	DeleteAll
//
//	Delete all items

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	}

void CItemList::DeleteItem (int iIndex)

//	DeleteItem
//
//	Deletes the item at the index position

	{
	delete m_List[iIndex];
	m_List.Delete(iIndex);
	}

void CItemList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads the item list from a stream
//
//	DWORD		Number of items
//	CItem[number of items]

	{
	int i;
	DWORD dwLoad;

	ASSERT(m_List.GetCount() == 0);

	//	Read the number of items

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));

	//	(Previous versions also saved an allocation count)

	if (Ctx.dwVersion < 58)
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	//	Load the items

	if (dwCount > 0)
		{
		m_List.InsertEmpty(dwCount);

		for (i = 0; i < (int)dwCount; i++)
			{
			m_List[i] = new CItem;
			m_List[i]->ReadFromStream(Ctx);
			}
		}
	}

void CItemList::SortItems (void)

//	SortItems
//
//	Sorts items in order:
//
//	installed/not-installed
//	armor/weapon/device/other

	{
	if (GetCount() == 0)
		return;

	int i;
	CSymbolTable Sort(false, true);

	for (i = 0; i < GetCount(); i++)
		{
		CItem &Item = GetItem(i);
		CItemType *pType = Item.GetType();

		//	All installed items first

		CString sInstalled;
		if (Item.IsInstalled())
			sInstalled = CONSTLIT("0");
		else
			sInstalled = CONSTLIT("1");

		//	Next, sort on category

		CString sCat;
		switch (pType->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				sCat = CONSTLIT("0");
				break;

			case itemcatMissile:
				sCat = CONSTLIT("1");
				break;

			case itemcatShields:
				sCat = CONSTLIT("2");
				break;

			case itemcatReactor:
				sCat = CONSTLIT("3");
				break;

			case itemcatDrive:
				sCat = CONSTLIT("4");
				break;

			case itemcatCargoHold:
				sCat = CONSTLIT("5");
				break;

			case itemcatMiscDevice:
				sCat = CONSTLIT("6");
				break;

			case itemcatArmor:
				sCat = CONSTLIT("7");
				break;

			case itemcatFuel:
			case itemcatUseful:
				sCat = CONSTLIT("8");
				break;

			default:
				sCat = CONSTLIT("9");
			}

		//	Next, sort by install location

		if (Item.IsInstalled() && Item.GetType()->GetArmorClass())
			sCat.Append(strPatternSubst(CONSTLIT("%03d%08x"), Item.GetInstalled(), Item.GetType()->GetUNID()));
		else
			sCat.Append(CONSTLIT("99900000000"));

		//	Within category, sort by level (highest first)

		sCat.Append(strPatternSubst(CONSTLIT("%02d"), MAX_ITEM_LEVEL - Item.GetType()->GetApparentLevel()));

		//	Enhanced items before others

		if (Item.IsEnhanced())
			sCat.Append(CONSTLIT("0"));
		else if (Item.IsDamaged())
			sCat.Append(CONSTLIT("2"));
		else
			sCat.Append(CONSTLIT("1"));

		CString sName = pType->GetSortName();
		CString sSort = strPatternSubst(CONSTLIT("%s%s%s%d"), sInstalled, sCat, sName, ((DWORD)(int)&Item) % 0x10000);
		Sort.AddEntry(sSort, (CObject *)i);
		}

	//	Allocate a new list

	TArray<CItem *> NewList;
	NewList.InsertEmpty(GetCount());

	//	Move the items from the old list to the new list in the new order

	for (i = 0; i < GetCount(); i++)
		{
		int iOld = (int)Sort.GetValue(i);
		NewList[i] = m_List[iOld];
		}

	//	Swap

	m_List.TakeHandoff(NewList);
	}

void CItemList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes the item list to stream
//
//	DWORD		Number of items
//	CItem[number of items]

	{
	int i;

	DWORD dwSave = GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < GetCount(); i++)
		GetItem(i).WriteToStream(pStream);
	}
