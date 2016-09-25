//	CArtifactMessagePainter.cpp
//
//	CArtifactMessagePainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int DEFAULT_MESSAGE_LIFETIME =				150;
const int TTY_CHARACTER_DELAY =						2;

CArtifactMessagePainter::CArtifactMessagePainter (const CVisualPalette &VI) :
		m_VI(VI),
		m_iTick(0)

//	CArtifactMessagePainter constructor

	{
	}

void CArtifactMessagePainter::AddMessage (const CString &sText, EStyles iStyle, int x, int y, int iDelay)

//	AddMessage
//
//	Adds a message to paint

	{
	SEntry *pNewEntry = m_Entries.Insert();
	pNewEntry->iStyle = iStyle;
	pNewEntry->sText = sText;

	//	NOTE: Callers give the coordinate of the center of the message. We 
	//	convert this later to upper-left coordinate.

	pNewEntry->x = x;
	pNewEntry->y = y;

	//	If we're delaying the message, then set the state

	if (iDelay > 0)
		{
		pNewEntry->iState = stateDelay;
		pNewEntry->iTick = iDelay;
		}

	//	Otherwise, we initialize

	else
		UpdateNextState(*pNewEntry);
	}

void CArtifactMessagePainter::Paint (CG32bitImage &Dest) const

//	Paint
//
//	Paint all messages

	{
	int i;

	for (i = 0; i < m_Entries.GetCount(); i++)
		{
		switch (m_Entries[i].iState)
			{
			case stateBlinkIn:
				PaintBlinkIn(Dest, m_Entries[i]);
				break;

			case stateFadeOut:
				PaintFadeOut(Dest, m_Entries[i]);
				break;

			case stateNormal:
				PaintNormal(Dest, m_Entries[i]);
				break;

			case stateTTYIn:
				PaintTTYIn(Dest, m_Entries[i]);
				break;
			}
		}
	}

void CArtifactMessagePainter::PaintBlinkIn (CG32bitImage &Dest, SEntry &Entry) const

//	PaintBlinkIn
//
//	Blinks the message in

	{
	}

void CArtifactMessagePainter::PaintFadeOut (CG32bitImage &Dest, SEntry &Entry) const

//	PaintFadeOut
//
//	Fades out

	{
	}

void CArtifactMessagePainter::PaintNormal (CG32bitImage &Dest, SEntry &Entry) const

//	PaintNormal
//
//	Paints a steady state message

	{
	Entry.pFont->DrawText(Dest, Entry.x, Entry.y, Entry.rgbText, Entry.sText);
	}

void CArtifactMessagePainter::PaintTTYIn (CG32bitImage &Dest, SEntry &Entry) const

//	PaintTTYIn
//
//	Paints character by character

	{
	int xEnd;
	Entry.pFont->DrawText(Dest, Entry.x, Entry.y, Entry.rgbText, strSubString(Entry.sText, 0, Entry.iTick), 0, &xEnd);
	Dest.Fill(xEnd, Entry.y, 12, Entry.pFont->GetHeight(), Entry.rgbText);
	}

void CArtifactMessagePainter::Update (void)

//	Update
//
//	Update all

	{
	int i;

	m_iTick++;

	for (i = 0; i < m_Entries.GetCount(); i++)
		{
		UpdateNextState(m_Entries[i]);

		//	If we reach the delete state, then we're done and delete the entry

		if (m_Entries[i].iState == stateDelete)
			{
			m_Entries.Delete(i);
			i--;
			}
		}
	}

void CArtifactMessagePainter::UpdateNextState (SEntry &Entry) const

//	UpdateNextState
//
//	Increments the tick and switches to the next state, if necessary.

	{
	switch (Entry.iState)
		{
		//	If we're just starting, the first state depends on the style.

		case stateNone:
			{
			Entry.iTick = 0;
			Entry.pFont = &m_VI.GetFont(fontLarge);
			Entry.rgbText = CG32bitPixel(128, 255, 255);

			switch (Entry.iStyle)
				{
				case styleDanger:
					Entry.iState = stateBlinkIn;
					break;

				case styleInfo:
					Entry.iState = stateTTYIn;
					break;

				default:
					Entry.iState = stateNormal;
				}

			//	Now that we have a font, position the entry

			int cyHeight;
			int cxWidth = Entry.pFont->MeasureText(Entry.sText, &cyHeight);

			Entry.x = Entry.x - (cxWidth / 2);
			Entry.y = Entry.y - (cyHeight / 2);
			break;
			}

		case stateDelay:
			if (--Entry.iTick == 0)
				Entry.iState = stateNone;
			break;

		case stateTTYIn:
			//	If we've hit the end of the text, then flip to normal 

			if (Entry.iTick >= Entry.sText.GetLength())
				{
				Entry.iState = stateNormal;
				Entry.iTick = 0;
				}

			//	Otherwise, advance once character if it's time

			else if ((m_iTick % TTY_CHARACTER_DELAY) == 0)
				{
				Entry.iTick++;
				}

			break;

		case stateNormal:
			//	If we're done, then delete

			if (++Entry.iTick >= DEFAULT_MESSAGE_LIFETIME)
				Entry.iState = stateDelete;
			break;
		}
	}
