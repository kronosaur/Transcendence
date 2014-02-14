//	CSoundType.cpp
//
//	CSoundType class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define SEGMENTS_TAG						CONSTLIT("Segments")

#define END_POS_ATTRIB						CONSTLIT("endPos")
#define FILENAME_ATTRIB						CONSTLIT("filename")
#define LOCATION_CRITERIA_ATTRIB			CONSTLIT("locationCriteria")
#define PRIORITY_ATTRIB						CONSTLIT("priority")
#define START_POS_ATTRIB					CONSTLIT("startPos")

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

	CString sFilename = pDesc->GetAttribute(FILENAME_ATTRIB);
	if (!sFilename.IsBlank())
		m_sFilespec = Ctx.pResDb->ResolveFilespec(Ctx.sFolder, pDesc->GetAttribute(FILENAME_ATTRIB));

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
				return ComposeLoadError(Ctx, CONSTLIT("Invalid segment positions."));

			if (i > 0 && iStartPos < m_Segments[i - 1].iStartPos)
				return ComposeLoadError(Ctx, CONSTLIT("Segments must be in order of increasing startPos."));

			//	Initialize

			m_Segments[i].iStartPos = iStartPos;
			m_Segments[i].iEndPos = iEndPos;
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
