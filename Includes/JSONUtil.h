//	JSONUtil.h
//
//	Library to help parse JSON
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#ifndef INCL_JSONUTIL
#define INCL_JSONUTIL

#ifndef INCL_INTERNETS
#include "Internets.h"
#endif

class CJSONValue
	{
	public:
		enum Types
			{
			typeString,
			typeNumber,
			typeObject,
			typeArray,
			typeTrue,
			typeFalse,
			typeNull,
			};

		CJSONValue (void) : m_iType(typeNull), m_pValue(NULL) { }
		CJSONValue (Types iType);
		CJSONValue (const CJSONValue &Source);
		CJSONValue (const CString &sValue, bool bToUTF8 = false);
		CJSONValue (int iValue);
		CJSONValue (double rValue);
		~CJSONValue (void);

		CJSONValue &operator= (const CJSONValue &Source);

		CString AsCP1252 (void) const;
		inline double AsDouble (void) const { return (m_iType == typeNumber ? *(double *)m_pValue : 0.0); }
		inline int AsInt32 (void) const { return (m_iType == typeNumber ? (int)*(double *)m_pValue : 0); }
		inline CString AsString (void) const { return (m_iType == typeString ? CString::INTMakeString(m_pValue) : NULL_STR); }
		static ALERROR Deserialize (const CString &sBuffer, CJSONValue *retValue, CString *retsError);
		int GetCount (void) const;
		const CJSONValue &GetElement (int iIndex) const;
		const CJSONValue &GetElement (const CString &sKey) const;
		const CString &GetKey (int iIndex) const;
		Types GetType (void) const { return m_iType; }
		bool FindElement (const CString &sKey, CJSONValue *retValue = NULL) const;
		void Insert (const CJSONValue &Source);
		void Insert (const CString &sKey, const CJSONValue &Source);
		void InsertHandoff (CJSONValue &Source);
		void InsertHandoff (const CString &sKey, CJSONValue &Source);
		inline bool IsNotFalse (void) const { return ((m_iType != typeFalse) && (m_iType != typeNull)); }
		inline bool IsNull (void) const { return (m_iType == typeNull); }
		void Serialize (IWriteStream *pOutput) const;
		static void SerializeString (IWriteStream *pOutput, const CString &sText);
		void TakeHandoff (CJSONValue &Source);

	private:
		void CleanUp (void);
		void Copy (const CJSONValue &Source);

		Types m_iType;
		void *m_pValue;
	};

class CJSONMessage : public IMediaType
	{
	public:
		CJSONMessage (void) { }
		CJSONMessage (CJSONValue &Value);

		//	IMediaType
		virtual ALERROR DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer);
		virtual ALERROR EncodeToBuffer (IWriteStream *pOutput) const;
		virtual DWORD GetMediaLength (void) const;
		virtual CString GetMediaType (void) const { return CONSTLIT("application/json"); }

	private:
		void Serialize (void) const;

		CJSONValue m_Value;
		mutable CMemoryWriteStream m_Serialized;
	};

#endif
