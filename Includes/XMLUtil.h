//	XMLUtil.h
//
//	Library to help parse XML files
//
//	Supported Format:
//
//		document :: <?XML version="1.0"?> {element}
//		
//		element :: {empty-element} | {open-tag} {content}* {close-tag}
//		empty-element :: < TAG {attribute}* />
//		open-tag :: < TAG {attribute}* >
//		close-tag :: </ TAG >
//		attribute :: ATTRIBUTE = " VALUE "
//		content :: {element} | {text} | {cdata} | {comment}
//		text :: any character except & and <
//		cdata :: <![CDATA[ any-text ]]>
//		comment :: <!-- any-text -->

#ifndef INCL_XMLUTIL
#define INCL_XMLUTIL

class CExternalEntityTable;
class CXMLElement;

class IXMLParserController
	{
	public:
		virtual ~IXMLParserController (void) { }

		virtual ALERROR OnOpenTag (CXMLElement *pElement, CString *retsError) { return NOERROR; }
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL) = 0;
	};

class CXMLElement : public CObject
	{
	public:
		CXMLElement (void);
		CXMLElement (const CXMLElement &Obj);
		CXMLElement (const CString &sTag, CXMLElement *pParent);
		~CXMLElement (void) { CleanUp(); }

		CXMLElement &operator= (const CXMLElement &Obj);

		static ALERROR ParseXML (IReadBlock *pStream, 
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static ALERROR ParseXML (IReadBlock *pStream, 
								 IXMLParserController *pController,
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static ALERROR ParseSingleElement (IReadBlock *pStream, 
										   IXMLParserController *pController,
										   CXMLElement **retpElement, 
										   CString *retsError);
		static ALERROR ParseEntityTable (IReadBlock *pStream, CExternalEntityTable *retEntityTable, CString *retsError);
		static ALERROR ParseRootElement (IReadBlock *pStream, CXMLElement **retpRoot, CExternalEntityTable *retEntityTable, CString *retsError);
		static ALERROR ParseRootTag (IReadBlock *pStream, CString *retsTag);

		ALERROR AddAttribute (const CString &sAttribute, const CString &sValue);
		ALERROR AppendContent (const CString &sContent, int iIndex = -1);
		ALERROR AppendSubElement (CXMLElement *pElement, int iIndex = -1);
		bool AttributeExists (const CString &sName);
		CString ConvertToString (void);
		ALERROR DeleteSubElement (int iIndex);
		bool FindAttribute (const CString &sName, CString *retsValue = NULL);
		bool FindAttributeBool (const CString &sName, bool *retbValue = NULL);
		bool FindAttributeDouble (const CString &sName, double *retrValue = NULL);
		bool FindAttributeInteger (const CString &sName, int *retiValue = NULL);
		CString GetAttribute (const CString &sName);
		inline CString GetAttribute (int iIndex) { return *(CString *)m_Attributes.GetValue(iIndex); }
		bool GetAttributeBool (const CString &sName);
		inline int GetAttributeCount (void) { return m_Attributes.GetCount(); }
		double GetAttributeDouble (const CString &sName);
		double GetAttributeDoubleBounded (const CString &sName, double rMin, double rMax = -1.0, double rNull = 0.0);
		int GetAttributeInteger (const CString &sName);
		int GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax = -1, int iNull = 0);
		ALERROR GetAttributeIntegerList (const CString &sName, CIntArray *pList);
		float GetAttributeFloat (const CString &sName);
		inline CString GetAttributeName (int iIndex) { return m_Attributes.GetKey(iIndex); }
		inline int GetContentElementCount (void) const { return m_ContentElements.GetCount(); }
		inline CXMLElement *GetContentElement (int iOrdinal) const { return m_ContentElements[iOrdinal]; }
		CXMLElement *GetContentElementByTag (const CString &sTag) const;
		inline const CString &GetContentText (int iOrdinal) { return ((iOrdinal >= 0 && iOrdinal < m_ContentText.GetCount()) ? m_ContentText[iOrdinal] : NULL_STR); }
		inline CXMLElement *GetParentElement (void) const { return m_pParent; }
		inline const CString &GetTag (void) const { return m_sTag; }
		void MergeFrom (CXMLElement *pElement);
		CXMLElement *OrphanCopy (void);
		ALERROR SetAttribute (const CString &sName, const CString &sValue);
		ALERROR SetContentText (const CString &sContent, int iIndex = -1);
		ALERROR WriteToStream (IWriteStream *pStream);

		static CString MakeAttribute (const CString &sText) { return strToXMLText(sText); }
		static bool IsBoolTrueValue (const CString &sValue) { return (strEquals(sValue, CONSTLIT("true")) || strEquals(sValue, CONSTLIT("1"))); }

	private:
		void CleanUp (void);

		CString m_sTag;							//	Element tag
		CXMLElement *m_pParent;					//	Parent of this element
		CSymbolTable m_Attributes;				//	Table of CStrings
		TArray<CXMLElement *> m_ContentElements;//	Array of sub elements
		TArray<CString> m_ContentText;			//	Interleaved content
	};

class CExternalEntityTable : public IXMLParserController
	{
	public:
		CExternalEntityTable (void);

		void AddTable (CSymbolTable &Table);
		inline int GetCount (void) { return m_Entities.GetCount(); }
		void GetEntity (int iIndex, CString *retsEntity, CString *retsValue);

		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);
		virtual void SetParent (IXMLParserController *pParent) { m_pParent = pParent; }

	private:
		TSortMap<CString, CString> m_Entities;
		IXMLParserController *m_pParent;
	};

class CEntityResolverList : public IXMLParserController
	{
	public:
		CEntityResolverList (void) { }

		inline void AddResolver (IXMLParserController *pResolver) { m_Resolvers.Insert(pResolver); }

		//	IXMLParserController virtuals
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		TArray<IXMLParserController *> m_Resolvers;
	};

//	Some utilities

ALERROR CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement);
ALERROR ParseAttributeIntegerList (const CString &sValue, CIntArray *pList);

#endif