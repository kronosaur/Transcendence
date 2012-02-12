//	Core.h
//
//	Header file for Library Link back-end

#ifndef INCL_CORE
#define INCL_CORE

typedef DWORD ENTRYUNID;
typedef SYSTEMTIME TIMEDATE;

//	Hard-coded UNIDs

const DWORD UndefinedUNID =			0xFFFFFFFF;
const DWORD CategoriesUNID	=		0xFFFFFFFE;
const DWORD AllEntriesUNID =		0xFFFFFFFD;
const DWORD EphemeralUNID =			0xFFFFFFFC;

//	System categories

const WCHAR UNIDTableCategory[] = L"$UNIDTable";

//	Forward classes

class CCore;

class CField : public CObject
	{
	public:
		enum DataTypes
			{
			Date = 0,
			Number,
			Text,
			PersonName,
			Code
			};

		CField (void) : CObject(NULL) { }
		virtual ~CField (void) { }

		inline CString GetValue (int iIndex) { return m_Value.GetStringValue(iIndex); }
		inline int GetValueCount (void) { return m_Value.GetCount(); }
		inline CString GetName (void) { return m_sFieldName; }
		inline DataTypes GetType (void) { return m_iDataType; }
		inline BOOL IsSystemField (void) { return *m_sFieldName.GetPointer() == '$'; }

		inline void SetName (CString sName) { m_sFieldName = sName; }
		inline void SetType (DataTypes iType) { m_iDataType = iType; }
		inline void SetValues (CStringArray &Values) { m_Value = Values; }
		inline void SetSingleValue (CString sValue) { m_Value.RemoveAll(); m_Value.AppendString(sValue, NULL); }

	private:
		DataTypes m_iDataType;						//	DataType for content of element
		CString m_sFieldName;						//	Name of field
		CStringArray m_Value;						//	Actual data
	};

class CEntry
	{
	public:
		CEntry (CCore *pCore);
		virtual ~CEntry (void);

		static ALERROR LoadFromXML (CCore *pCore, CString sData, CEntry **retpEntry, CString *retsError);
		static ALERROR SaveToXML (CEntry *pEntry, CString *retsData);

		inline ALERROR AppendField (CField *pField) { return m_FieldList.AppendObject(pField, NULL); }
		ALERROR AddCategory (CString sCategory);
		ALERROR CompleteLoad (void);
		BOOL FindField (CString sName, CField **retpField);
		inline CString GetBody (void) { return m_sBody; }
		inline CStringArray &GetCategories (void) { return m_Categories; }
		inline int GetCategoryCount (void) { return m_Categories.GetCount(); }
		inline CString GetCategory (int iIndex) { return m_Categories.GetStringValue(iIndex); }
		inline ALERROR GetCode (CString sCodeEntry, ICCItem **retpCode) { return m_CodeList.Lookup(sCodeEntry, (CObject **)retpCode); }
		inline CCore *GetCore (void) { return m_pCore; }
		inline int GetFieldCount (void) { return m_FieldList.GetCount(); }
		inline CField *GetField (int iIndex) { return (CField *)m_FieldList.GetObject(iIndex); }
		inline CString GetRelatedLinks (void) { return m_sRelatedLinks; }
		inline CString GetTitle (void) { return m_sTitle; }
		inline ENTRYUNID GetUNID (void) { return m_dwUNID; }
		BOOL InCategory (CString sCategory);
		inline BOOL IsReadOnly (void) { return m_fEphemeral; }
		inline void SetCategories (CStringArray &Values) { m_Categories = Values; }
		inline void SetCreatedBy (CString sCreatedBy) { m_sCreatedBy = sCreatedBy; }
		inline void SetCreatedOn (TIMEDATE &td) { m_tdCreatedOn = td; }
		void SetCreation (void);
		inline void SetBody (CString sBody) { m_sBody = sBody; }
		inline void SetEphemeral (void) { m_fEphemeral = TRUE; }
		void SetModification (void);
		inline void SetModifiedOn (TIMEDATE &td) { m_tdModifiedOn = td; }
		inline void SetTemplate (CString sTemplate) { m_sTemplate = sTemplate; }
		inline void SetTitle (CString sTitle) { m_sTitle = sTitle; }
		inline void SetUNID (ENTRYUNID dwUNID) { m_dwUNID = dwUNID; }

	private:
		ALERROR ComputeRelatedLinks (CStringArray *pLinks);
		ALERROR CreateRelatedLinksSection (void);
		ALERROR LinkCode (void);

		CCore *m_pCore;								//	Pointer to core

		ENTRYUNID m_dwUNID;							//	Unique ID
		CString m_sTitle;							//	Entry title
		CString m_sBody;							//	Body
		CStringArray m_Categories;					//	List of categories
		CString m_sCreatedBy;						//	Author
		TIMEDATE m_tdCreatedOn;						//	Creation date
		TIMEDATE m_tdModifiedOn;					//	Modification date
		CString m_sTemplate;						//	Template

		CObjectArray m_FieldList;					//	Array of CField
		CSymbolTable m_CodeList;					//	Code
		CString m_sRelatedLinks;					//	Related links section

		DWORD m_fEphemeral:1;						//	Entry is not persistent
	};

class CCategory : public CObject
	{
	public:
		CCategory (void);
		CCategory (CCore *pCore, CString sCategory);
		virtual ~CCategory (void);

		static ALERROR LoadFromEntry (CEntry *pEntry, CCategory **retpCategory);

		inline CString GetCategory (void) { return m_sCategory; }
		inline CString GetIndexURL (void) { return m_sIndexURL; }

	private:
		CCore *m_pCore;
		CString m_sCategory;						//	Category
		CString m_sIndexURL;						//	Entry to display as category index
	};

typedef struct
	{
	CCore *pCore;
	} COREEXECCTX;

class CCore
	{
	public:
		CCore (void);
		~CCore (void);

		ALERROR Close (void);
		static ALERROR Create (CString sFilename);
		ALERROR Flush (void);
		ALERROR Open (CString sFilename);

		ALERROR CreateNewEntry (CEntry *pEntry);
		CString ExecuteCommand (CString sCommand);
		ALERROR GetCategoryLinks (CString sCategory, CSymbolTable *pLinks);
		inline CCodeChain *GetCC (void) { return m_pCC; }
		CString GetEntryTitle (ENTRYUNID dwUNID);
		inline BOOL IsSystemCategory (CString sCategory) { return *sCategory.GetPointer() == '$'; }
		ALERROR LoadEntry (CString sURL, CEntry **retpEntry);
		ALERROR LoadEntryByUNID (ENTRYUNID dwUNID, CEntry **retpEntry);
		CString MakeURLFromUNID (ENTRYUNID dwUNID);
		ALERROR SaveEntry (CEntry *pEntry);

	private:
		typedef struct
			{
			DWORD dwSignature;							//	Always 'CORE'
			DWORD dwVersion;							//	Version

			DWORD dwUNID2TitleIndex;					//	UNID for UNID2Title index
			DWORD dwTitle2UNIDIndex;					//	UNID for Title2UNID index
			DWORD dwCategoryIndex;						//	UNID for Category index

			DWORD dwUnused[61];							//	Spares
			} COREHEADER;

		ALERROR CreateGeneratedEntry (ENTRYUNID dwUNID, CEntry **retpEntry);
		static CString Deuniquify (CString sText, ENTRYUNID *retdwUNID);
		ALERROR LoadCategory (CString sData, CEntry **retpEntry);
		ALERROR LoadCategoryList (CEntry **retpEntry);
		ALERROR LoadIndexEntry (CEntry **retpEntry);
		ALERROR LoadHome (CEntry **retpEntry);
		ALERROR ParseCoreURL (CString sURL, CString *retsTopLevel, CString *retsData);
		ALERROR RegisterPrimitives (void);
		ICCItem *RunEntryCode (CEntry *pEntry, CString sCode, ICCItem *pArg1, ICCItem *pArg2, ICCItem *pArg3);
		ALERROR SaveHeaderObject (CObject *pObject, DWORD *iodwUNID);
		static CString Uniquify (CString sText, ENTRYUNID dwUNID);
		ALERROR UpdateCategoryIndex (CEntry *pEntry, BOOL bRemoveOldCategories);
		ALERROR UpdateCategoryList (void);
		ALERROR UpdateTitleIndex (CEntry *pEntry);

		CDataFile *m_pDb;							//	Actual file access
		COREHEADER m_Header;						//	Header info

		CIDTable *m_pUNID2Title;					//	Maps an entry UNID to a title
		CSymbolTable *m_pTitle2UNID;				//	Maps a title to an UNID
		CSymbolTable *m_pCategoryIndex;				//	Index of categories
		CSymbolTable *m_pCategoryList;				//	List of custom categories

		CCodeChain *m_pCC;							//	CodeChain processor

		DWORD m_fHeaderModified:1;					//	TRUE if header modified
		DWORD m_fTitleIndexModified:1;				//	TRUE if either UNID2Title or Title2UNID changed
		DWORD m_fCategoryIndexModified:1;			//	TRUE if category index needs to be saved
	};

#endif