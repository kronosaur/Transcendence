//	Utilities.h
//
//	Utility classes for TransData
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CImageGrid
	{
	public:
		void Create (int iCount, int cxCellWidth, int cyCellHeight);
		void GetCellCenter (int iIndex, int *retx, int *rety);
		inline CG32bitImage &GetImage (void) { return m_Image; }

	private:
		CG32bitImage m_Image;
		int m_iCount;
		int m_iCellColumns;
		int m_iCellRows;
		int m_cxCellWidth;
		int m_cyCellHeight;
	};

class CItemTypeTable
	{
	public:
		CItemTypeTable (void) : m_bAll(true) { }

		bool Filter (const CString &sCriteria);
		int GetCount (void) const;
		CItemType *GetItemType (int iIndex) const;
		inline bool IsAll (void) const { return m_bAll;  }
		void Sort (void);

	private:
		int GetItemFreqIndex (CItemType *pType) const;
		int GetItemTypeIndex (CItemType *pType) const;

		TArray<CItemType *> m_Table;
		bool m_bAll;
	};

class COutputChart
	{
	public:
		CG32bitImage &GetOutputImage (int *retxOrigin = NULL, int *retyOrigin = NULL);
		CG32bitPixel GetStyleColor (const CString &sStyle);
		const CG16bitFont &GetStyleFont (const CString &sStyle);
		bool Output (void);
		void SetContentSize (int cxWidth, int cyHeight);
		void SetOutputFilespec (const CString &sFilespec);
		void SetStyleFont (const CString &sStyle, const CString &sFontDesc);
		void SetStyleColor (const CString &sStyle, CG32bitPixel rgbColor);

	private:
		struct SStyleDesc
			{
			SStyleDesc (void) :
					rgbColor(0)
				{ }

			CG16bitFont Font;
			CG32bitPixel rgbColor;
			};

		CG32bitImage m_Image;
		TSortMap<CString, SStyleDesc> m_Styles;
		CString m_sFilespec;
	};

class CSimViewer
	{
	public:
		CSimViewer (void) : m_hWnd(NULL) { }
		~CSimViewer (void) { Destroy(); }

		void Create (void);
		void Destroy (void);
		inline bool IsEmpty (void) { return (m_hWnd == NULL); }
		void PaintViewport (CUniverse &Universe);

		inline void WMDestroy (void) { m_hWnd = NULL; }

	private:
		void ProcessMessages (void);
		static LONG APIENTRY WndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

		HWND m_hWnd;
		CG32bitImage m_Buffer;
		DWORD m_dwLastUpdate;
	};

//	Functions

bool IsMainCommandParam (const CString &sAttrib);
