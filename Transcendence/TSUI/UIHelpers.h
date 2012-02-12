//	UIHelpers.h
//
//	Classes and methods for generating appropriate UI.
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CItemDataAnimatron : public IAnimatron
	{
	public:
		struct SDamageAdjCell
			{
			DamageTypes iDamage;
			CString sText;
			};

		CItemDataAnimatron (const CVisualPalette &VI, const CItem &Item);

		inline bool IsEmpty (void) const { return (m_DamageAdj.GetCount() == 0); }

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

		static void CalcDamageAdj (const CItem &Item, TArray<SDamageAdjCell> *retDamageAdj);
		static int MeasureDamageAdj (const CG16bitFont &Font, const TArray<SDamageAdjCell> &DamageAdj);

	private:
		const CVisualPalette &m_VI;
		CItem m_Item;

		bool m_bIsWeapon;
		TArray<SDamageAdjCell> m_DamageAdj;
	};

class CListSaveFilesTask : public IHITask
	{
	public:
		CListSaveFilesTask (CHumanInterface &HI, const CString &sFolder, int cxWidth);
		~CListSaveFilesTask (void);

		inline IAnimatron *GetListHandoff (void) { IAnimatron *pResult = m_pList; m_pList = NULL; return pResult; }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		void CreateFileEntry (CGameFile &GameFile, const CTimeDate &ModifiedTime, int yStart, IAnimatron **retpEntry, int *retcyHeight);

		CString m_sFolder;
		int m_cxWidth;

		CAniListBox *m_pList;
	};

class CReadProfileTask : public IHITask
	{
	public:
		CReadProfileTask (CHumanInterface &HI, CCloudService &Service, int cxWidth);
		~CReadProfileTask (void);

		inline IAnimatron *GetListHandoff (void) { IAnimatron *pResult = m_pList; m_pList = NULL; return pResult; }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		void CreateAdventureRecordDisplay (CAdventureRecord &Record, int yStart, IAnimatron **retpAni);

		CCloudService &m_Service;
		int m_cxWidth;

		CUserProfile m_Profile;

		CAniVScroller *m_pList;
	};

class CUIHelper
	{
	public:
		enum EOptions
			{
			//	CreateClassInfo???
			OPTION_ITEM_RIGHT_ALIGN =		0x00000001,

			//	CreateSessionTitle
			OPTION_SESSION_OK_BUTTON =		0x00000001,
			};

		CUIHelper (CHumanInterface &HI) : m_HI(HI) { }

		void CreateClassInfoArmor (CShipClass *pClass, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoCargo (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoDeviceSlots (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoDrive (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoItem (const CItem &Item, int x, int y, int cxWidth, DWORD dwOptions, const CString &sExtraDesc, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoReactor (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateSessionTitle (IHISession *pSession, CCloudService &Service, const CString &sTitle, DWORD dwOptions, IAnimatron **retpControl) const;

	private:
		void CreateClassInfoSpecialItem (CItemType *pItemIcon, const CString &sText, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;

		CHumanInterface &m_HI;
	};