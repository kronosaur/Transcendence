#pragma once

#include "resource.h"

struct SCreateS1
	{
	HINSTANCE hInst;
	char *pszCommandLine;
	bool bWindowed;
	};

const int GAME_STAT_POSITION_NEXT = -1;
const int GAME_STAT_POSITION_PREV = -2;
const int GAME_STAT_POSITION_NEXT_PAGE = -3;
const int GAME_STAT_POSITION_PREV_PAGE = -4;
const int GAME_STAT_POSITION_HOME = -5;
const int GAME_STAT_POSITION_END = -6;

class CMenuSession : public IHISession
	{
	public:
		CMenuSession (CHumanInterface &HI) : IHISession(HI) { }

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnInit (void);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags);
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (void);

	private:
		void CreateMenuAnimation (const RECT &rcRect, IAnimatron **retpAni);
		void SelectMenuItem (IAnimatron *pAni, int iStatPos, int cxWidth, int iDuration);
		void SetMenu (void);
		void SetSelection (int iPos);

		RECT m_rcWidescreen;
		int m_iCount;
		int m_iPos;

		CG16bitFont m_SuperTitleFont;
	};

#define CMD_LEFT						CONSTLIT("cmdLeft")
#define CMD_RIGHT						CONSTLIT("cmdRight")
#define CMD_UP							CONSTLIT("cmdUp")
#define CMD_DOWN						CONSTLIT("cmdDown")
#define CMD_ENTER						CONSTLIT("cmdEnter")
#define CMD_ESCAPE						CONSTLIT("cmdEscape")

class CS1Controller : public IHICommand
	{
	public:
		CS1Controller (CHumanInterface &HI) : IHICommand(HI),
				m_pGuide(NULL),
				m_pPos(NULL)
				{ }

		bool CmdEscape (void);
		void CmdSelectMenuItem (int iIndex);
		int GetMenuCount (void);
		CString GetMenuTitle (void);
		CString GetMenuItemLabel (int iIndex);

		//	IHICommand virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (void);

	private:
		ALERROR LoadGuide (void);

		CXMLElement *m_pGuide;
		CXMLElement *m_pPos;
	};

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

extern CHumanInterface *g_pHI;
extern CS1Controller *g_pCtrl;
