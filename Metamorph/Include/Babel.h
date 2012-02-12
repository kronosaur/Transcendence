//	Babel.h
//
//	Classes for universal data structures
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#ifndef INCL_BABEL
#define INCL_BABEL

enum DataElementTypes
	{
	dtNil,
	dtInteger,
	dtString,
	dtList,
	dtStructure,
	dtCode,
	};

//	CBabelData
//
//	A single class takes care of all datatypes. We avoid virtual classes in
//	this case to save us the extra allocation required.

class CBabelData
	{
	public:
		CBabelData (void);
		CBabelData (int iInteger);
		CBabelData (const CString &sString);
		CBabelData (DataElementTypes Type);
		CBabelData (const CBabelData &sData);
		CBabelData (DataElementTypes Type, const CString &sString);

		~CBabelData (void);

		CBabelData &operator= (const CBabelData &sData);

		//	Generic functions
		inline DataElementTypes GetType (void) const { return m_Type; }
		inline bool IsAtom (void) const { return m_Type != dtList && m_Type != dtStructure; }
		inline bool IsNil (void) const { return GetType() != dtNil; }

		//	Integer functions
		inline int GetInteger (void) const { return (int)m_dwData; }

		//	String functions
		inline const CString &GetString (void) const { return *(CString *)(&m_dwData); }

		//	List functions
		inline const CBabelData &operator [] (int iIndex) const { return GetElement(iIndex); }
		void AddElement (const CBabelData &Data);
		const CBabelData &GetElement (int iIndex) const;
		int GetElementCount (void) const;

		//	Structure functions
		void AddField (const CString &sFieldName, const CBabelData &Data);
		const CBabelData &GetField (const CString &sFieldName) const;

	private:
		void CleanUp (void);
		void CopyData (const CBabelData &sData);

		DataElementTypes m_Type;			//	Type of data
		DWORD m_dwData;						//	Variant data depending on type
	};

CString BabelQuotedString (const CString &sString);
CBabelData ReadBabel (IMemoryBlock &Input);
//void WriteBabel (const CBabelData &Data, const IWriteStream &Output);

#endif