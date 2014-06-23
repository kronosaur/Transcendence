//	CSoundType.cpp
//
//	CSoundType class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define SEGMENTS_TAG						CONSTLIT("Segments")

#define COMPOSED_BY_ATTRIB					CONSTLIT("composedBy")
#define END_POS_ATTRIB						CONSTLIT("endPos")
#define FILENAME_ATTRIB						CONSTLIT("filename")
#define LOCATION_CRITERIA_ATTRIB			CONSTLIT("locationCriteria")
#define PERFORMED_BY_ATTRIB					CONSTLIT("performedBy")
#define PRIORITY_ATTRIB						CONSTLIT("priority")
#define START_POS_ATTRIB					CONSTLIT("startPos")
#define TITLE_ATTRIB						CONSTLIT("title")

bool CSoundType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns a data field

	{
	return false;
	}

int CSoundType::FindSegment (int iPos)

//	FindSegment
//
//	Returns the segment we're on. We always return a segment, even if we're out 
//	of range.

	{
	int i;

	for (i = 0; i < m_Segments.GetCount() - 1; i++)
		{
		if (iPos >= m_Segments[i].iStartPos
				&& (m_Segments[i].iEndPos == -1 || iPos < m_Segments[i].iEndPos))
			return i;
		}

	//	If we get this far then either we have no segments (and i = 0) or we hit
	//	the end of the array - 1, which means i is the last segment.

	return i;
	}

const CString &CSoundType::GetAlbum (void) const

//	GetAlbum
//
//	Returns the album name
	
	{
	CExtension *pExtension = GetExtension();
	return (pExtension ? pExtension->GetName() : NULL_STR);
	}

int CSoundType::GetNextFadePos (int iPos)

//	GetNextFadePos
//
//	Returns the end of the current segment (or -1 if the given segment continues
//	until the end of the track).

	{
	if (m_Segments.GetCount() == 0)
		return -1;

	return m_Segments[FindSegment(iPos)].iEndPos;
	}

CString CSoundType::GetFilespec (void) const

//	GetFilespec
//
//	Returns the filespec of the sound file.

	{
	return m_sFilespec;
	}

int CSoundType::GetNextPlayPos (void)

//	GetNextPlayPos
//
//	Returns the next position to play at.

	{
	if (m_Segments.GetCount() == 0)
		return 0;
	else
		return m_Segments[m_iNextSegment].iStartPos;
	}

ALERROR CSoundType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Initialize from XML

	{
	int i;
	ALERROR error;

	m_sFilename = pDesc->GetAttribute(FILENAME_ATTRIB);
	m_sTitle = pDesc->GetAttribute(TITLE_ATTRIB);
	m_sComposedBy = pDesc->GetAttribute(COMPOSED_BY_ATTRIB);
	m_sPerformedBy = pDesc->GetAttribute(PERFORMED_BY_ATTRIB);

	//	Set the actual location of the file

	if (!m_sFilename.IsBlank())
		{
		//	If this is in the Collection folder, then we expect it to be under
		//	the extension folder.

		if (Ctx.pExtension
				&& Ctx.pExtension->GetFolderType() == CExtension::folderCollection)
			m_sFilespec = g_pUniverse->GetExtensionCollection().GetExternalResourceFilespec(Ctx.pExtension, m_sFilename);

		//	Otherwise, it is part of the resource db

		else
			m_sFilespec = Ctx.pResDb->ResolveFilespec(Ctx.sFolder, m_sFilename);
		}

	//	Location criteria

	m_iPriority = pDesc->GetAttributeIntegerBounded(PRIORITY_ATTRIB, 0, -1, 0);

	if (error = m_LocationCriteria.Parse(pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB), 0, &Ctx.sError))
		return ComposeLoadError(Ctx, Ctx.sError);

	//	Load any segments

	CXMLElement *pSegments = pDesc->GetContentElementByTag(SEGMENTS_TAG);
	if (pSegments)
		{
		m_Segments.InsertEmpty(pSegments->GetContentElementCount());

		for (i = 0; i < pSegments->GetContentElementCount(); i++)
			{
			CXMLElement *pSegment = pSegments->GetContentElement(i);

			//	Get the start position

			int iStartPos = pSegment->GetAttributeInteger(START_POS_ATTRIB);

			//	Get the end position. If this is the last segment or if the end position
			//	is omitted, then we run to the end.

			int iEndPos;
			if ((i == pSegments->GetContentElementCount() - 1)
					|| !pSegment->FindAttributeInteger(END_POS_ATTRIB, &iEndPos))
				iEndPos = -1;

			//	Check for errors

			if (iStartPos < 0 || (iEndPos != -1 && iEndPos < iStartPos))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Invalid segment positions: startPos=%d endPos=%d"), iStartPos, iEndPos));

			if (i > 0 && iStartPos < m_Segments[i - 1].iStartPos)
				return ComposeLoadError(Ctx, CONSTLIT("Segments must be in order of increasing startPos."));

			//	Initialize

			m_Segments[i].iStartPos = iStartPos;
			m_Segments[i].iEndPos = iEndPos;
			}

		//	If the first segment does not start at 0, then add an initial segment

		if (m_Segments.GetCount() > 0 && m_Segments[0].iStartPos != 0)
			{
			int iEndPos = m_Segments[0].iStartPos;
			SSegmentDesc *pFirst = m_Segments.InsertAt(0);
			pFirst->iStartPos = 0;
			pFirst->iEndPos = iEndPos;
			}
		}

	return NOERROR;
	}

void CSoundType::SetLastPlayPos (int iPos)

//	SetLastPlayPos
//
//	Sets the position at which we stopped playing. We remember the segment so 
//	that next time we start at the following segment.

	{
	m_iNextSegment = ((FindSegment(iPos) + 1) % GetSegmentCount());
	}
