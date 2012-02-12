//	TerraFirma.h
//
//	General definitions

#ifndef INCL_TERRAFIRMA
#define INCL_TERRAFIRMA

//	Application objects

#define OBJID_CLWINDOW					MakeOBJCLASSID(1)
#define OBJID_CLAREA					MakeOBJCLASSID(2)
#define OBJID_CLIMAGEAREA				MakeOBJCLASSID(3)

//	These are InterfaceID for methods that TerraFirma calls

#define METHOD_INODELISTSHOWNODE						6
#define METHOD_IGetRawImage								7
#define METHOD_IGetImageRect							8

class CLWindowManager;
class CTerraFirma;
class CLibraryFile;
class CNodeEditor;

class CGenericWnd
	{
	public:
		CGenericWnd (void);
		virtual ~CGenericWnd (void);

		ALERROR Create (CNodeEditor *pEditor, HWND hParent, RECT *pRect);
		void Destroy (void);
		inline void SetFocus (void) { ::SetFocus(m_hWnd); }

	private:
		HWND m_hWnd;
		CNodeEditor *m_pEditor;

	friend LONG APIENTRY GenericWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	};

class CCommandLineWnd
	{
	public:
		CCommandLineWnd (void);
		virtual ~CCommandLineWnd (void);

		ALERROR Create (CTerraFirma *pTF, HWND hParent, RECT rcRect);
		void Destroy (void);
		inline BOOL IsOpen (void) { return (m_hWnd != NULL); }
		inline void SetFocus (void) { ::SetFocus(m_hWnd); }

		CString GetData (void);
		void LoadBuffer (CString sMapEntry);
		void SaveBuffer (CString sMapEntry);
		void SetData (CString sData);

	private:
		friend LONG APIENTRY CommandLineWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

		void ExecuteCommand (void);
		CString GetCommand (void);
		void OutputResult (CString sResult);

		LONG WMChar (WPARAM wParam, LPARAM lParam);
		LONG WMKeyDown (WPARAM wParam, LPARAM lParam);

		CTerraFirma *m_pTF;
		HWND m_hWnd;
	};

class CLibraryFile
	{
	public:
		CLibraryFile (CCodeChain *pCC, CString sFilename);
		virtual ~CLibraryFile (void);

		ALERROR Close (void);
		ALERROR Create (void);
		ALERROR Flush (void);
		ALERROR Open (void);

		ALERROR CreateEntry (ICCItem *pItem, int *retiUNID);
		ALERROR DeleteEntry (int iUNID);
		ALERROR ReadEntry (int iUNID, ICCItem **retpItem);
		ALERROR WriteEntry (int iUNID, ICCItem *pItem);

		inline ICCItem *GetMap (void) { return m_pMap; }
		ICCItem *LoadMapEntry (CString sKey);
		ICCItem *SaveMapEntry (CString sKey, ICCItem *pValue);

	private:
		CCodeChain *m_pCC;							//	CodeChain interpreter
		CDataFile m_Db;								//	Data file
		ICCItem *m_pMap;							//	Main symbol table
	};

#define NODEEDITOR_TYPE_COMMANDSHELL			0	//	Command-line shell
#define NODEEDITOR_TYPE_LINE					1	//	Text lines
#define NODEEDITOR_TYPE_NODELIST				2	//	Directory of nodes
#define NODEEDITOR_TYPE_IMAGE					3	//	Image editor

class CNodeEditor
	{
	public:
		CNodeEditor (void);
		virtual ~CNodeEditor (void);

		//	Creation and destruction

		ALERROR Create (CTerraFirma *pTF, HWND hParent, RECT rcRect);
		void Destroy (void);

		//	Virtual methods

		virtual DWORD GetEditorID (void) = 0;
		virtual ICCItem *GetNodeData (void) = 0;
		virtual LONG HandleControlNotification (int iNotifyCode) { return 0; }
		virtual ICCItem *SetNodeData (ICCItem *pNodeData) = 0;

		//	Virtual methods to handle generic window notifications

		virtual LONG Paint (HDC hDC, RECT *pUpdateRect) { return 0; }

		//	Miscellaneous

		CTerraFirma *GetTF (void);

	protected:
		CCodeChain *GetCC (void);
		CLibraryFile *GetLibrary (void);

		//	Virtual methods

		virtual void CleanUp (void) { }
		virtual ALERROR Initialize (HWND hParent, RECT rcRect) { return NOERROR; }
		virtual BOOL IsModified (void) { return FALSE; }
		virtual void SetFocus (void) { }

	private:
		CTerraFirma *m_pTF;
		HWND m_hParent;
		RECT m_rcRect;
	};

class CCommandShellEditor : public CNodeEditor
	{
	public:
		CCommandShellEditor (void);
		virtual ~CCommandShellEditor (void);

		//	Virtual methods

		virtual DWORD GetEditorID (void) { return NODEEDITOR_TYPE_COMMANDSHELL; }
		virtual ICCItem *GetNodeData (void);
		virtual ICCItem *SetNodeData (ICCItem *pNodeData);

	protected:

		//	Virtual methods

		virtual void CleanUp (void);
		virtual ALERROR Initialize (HWND hParent, RECT rcRect);

	private:
		CCommandLineWnd m_CommandLineWnd;			//	Command line window
	};

class CNodeListEditor : public CNodeEditor
	{
	public:
		CNodeListEditor (void);
		virtual ~CNodeListEditor (void);

		//	Virtual methods

		virtual DWORD GetEditorID (void) { return NODEEDITOR_TYPE_NODELIST; }
		virtual ICCItem *GetNodeData (void);
		virtual LONG HandleControlNotification (int iNotifyCode);
		virtual ICCItem *SetNodeData (ICCItem *pNodeData);

	protected:
		//	Virtual methods

		virtual void CleanUp (void);
		virtual ALERROR Initialize (HWND hParent, RECT rcRect);

	private:
		ICCItem *ComputeNodeList (ICCItem *pNodeData);
		void RefreshListbox (ICCItem *pNodeList);

		HWND m_hListbox;
		ICCItem *m_pNodeData;
	};

class CImageEditor : public CNodeEditor
	{
	public:
		CImageEditor (void);
		virtual ~CImageEditor (void);

		//	Virtual methods

		virtual DWORD GetEditorID (void) { return NODEEDITOR_TYPE_IMAGE; }
		virtual ICCItem *GetNodeData (void);
		virtual ICCItem *SetNodeData (ICCItem *pNodeData);

		//	Virtual methods to handle generic window notifications

		virtual LONG Paint (HDC hDC, RECT *pUpdateRect);

	protected:
		//	Virtual methods

		virtual void CleanUp (void);
		virtual ALERROR Initialize (HWND hParent, RECT rcRect);

	private:
		CGenericWnd m_Window;
		ICCItem *m_pNodeData;

		RECT m_rcImage;							//	Size of the image
	};

class CTerraFirma
	{
	public:
		CTerraFirma (HWND hWnd);
		~CTerraFirma (void);

		//	CodeChain access

		ICCItem *EditorNotification (int iNotification, ICCItem *pParam1, ICCItem *pParam2, ICCItem *pParam3);
		CString ExecuteCommand (CString sCommand);
		ICCItem *ObjMethod (ICCItem *pObj, int iMethod, ICCItem *pParam1, ICCItem *pParam2, ICCItem *pParam3);

		//	Library methods

		ALERROR CloseLibrary (void);
		ALERROR CreateLibrary (CString sFilename);
		ALERROR OpenLibrary (CString sFilename);

		ICCItem *EditData (DWORD dwEditorID, ICCItem *pData);
		inline CNodeEditor *GetNodeEditor (void) { return m_pEditor; }
		ICCItem *GetNodeTable (void);

		//	Miscellaneous

		ALERROR Boot (CString sLibrary);
		void CalcMetrics (int cxWidth, int cyHeight);
		ALERROR DisplayError (ALERROR error);
		ALERROR ExecuteBootCode (void);
		ALERROR ExecuteShutdownCode (void);
		inline CCodeChain *GetCC (void) { return &m_CC; }
		inline HWND GetHWND (void) { return m_hWnd; }
		inline HINSTANCE GetInstance (void) { return (HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE); }
		inline CLibraryFile *GetLibrary (void) { return m_pLibrary; }
		inline CLWindowManager *GetWM (void) { return m_pWM; }
		LONG HandleEditorNotification (int iNotifyCode);
		inline void Quit (void) { PostMessage(m_hWnd, WM_CLOSE, 0, 0); }
		ALERROR RegisterExtensions (void);
		ALERROR RegisterWindowsExtensions (void);
		ALERROR Shutdown (void);

	private:
		void CloseEmergencyShell (void);
		ALERROR OpenEmergencyShell (void);

		HWND m_hWnd;								//	Main Window
		CCodeChain m_CC;							//	CodeChain interpreter
		CLibraryFile *m_pLibrary;					//	Currently open library

		CCommandLineWnd m_CommandLineWnd;			//	Command line window
		CNodeEditor *m_pEditor;						//	Current editor
		CLWindowManager *m_pWM;						//	Window Manager

		//	Metrics

		RECT m_rcNodeEditor;						//	Rect for NodeEditor
	};

extern char g_szAppName[];

#endif
