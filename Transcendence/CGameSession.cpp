//	CGameSession.cpp
//
//	CGameSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CGameSession::PaintInfoText (CG32bitImage &Dest, const CString &sTitle, const TArray<CString> &Body, bool bAboveTargeting)

//	PaintInfoText
//
//	Paints info text on the screen

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &TitleFont = VI.GetFont(fontHeader);
	const CG16bitFont &BodyFont = VI.GetFont(fontMedium);

	int x = m_rcScreen.left + 2 * TitleFont.GetAverageWidth();
	int cy = TitleFont.GetHeight() + Body.GetCount() * BodyFont.GetHeight();
	int cySpacing = BodyFont.GetHeight() / 2;
	int y = (bAboveTargeting ? g_pTrans->m_TargetDisplay.GetRect().top : g_pTrans->m_TargetDisplay.GetRect().bottom) - (cy + cySpacing);

	Dest.DrawText(x,
			y,
			TitleFont,
			VI.GetColor(colorTextHighlight),
			sTitle);
	y += TitleFont.GetHeight();

	//	Paint the body

	for (i = 0; i < Body.GetCount(); i++)
		{
		Dest.DrawText(x,
				y,
				BodyFont,
				VI.GetColor(colorTextDialogLabel),
				Body[i]);
		y += BodyFont.GetHeight();
		}
	}

void CGameSession::PaintSoundtrackTitles (CG32bitImage &Dest)

//	PaintSoundtrackTitles
//
//	Paints the info about the currently playing soundtrack.

	{
	int iPos;
	CSoundType *pTrack = m_Soundtrack.GetCurrentTrack(&iPos);
	if (pTrack == NULL)
		return;

	int iSegment = pTrack->FindSegment(iPos);

	//	Time

	int iMilliseconds = iPos % 1000;
	int iSeconds = (iPos / 1000) % 60;
	int iMinutes = (iPos / 60000);

	//	Add all the components

	TArray<CString> Desc;
	Desc.InsertEmpty(3);
	Desc[0] = pTrack->GetPerformedBy();
	Desc[1] = pTrack->GetFilename();
	Desc[2] = strPatternSubst(CONSTLIT("Segment %d of %d [%02d:%02d.%03d]"), iSegment + 1, pTrack->GetSegmentCount(), iMinutes, iSeconds, iMilliseconds);

	//	Paint

	PaintInfoText(Dest, pTrack->GetTitle(), Desc, true);
	}
