//	IAnimatron.cpp
//
//	IAnimatron class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

#define ID_SCROLL_ANIMATION						CONSTLIT("idAniScroll")

#define PROP_MAX_SCROLL_POS						CONSTLIT("maxScrollPos")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCROLL_POS							CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

void IAnimatron::AnimateLinearFade (int iDuration, int iFadeIn, int iFadeOut, DWORD dwMaxOpacity)

//	AnimateLinearFade
//
//	Utility function to set animatron to fade in opacity

	{
	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(iDuration, iFadeIn, iFadeOut, dwMaxOpacity);
	AnimateProperty(CONSTLIT("opacity"), pFader);
	}

void IAnimatron::AnimateLinearRotation (int iStartAngle, Metric rRate, int iDuration)

//	AnimateLinearRotation
//
//	Rotates the animatron

	{
	CLinearRotation *pRotation = new CLinearRotation;
	pRotation->SetParams(iStartAngle, rRate, iDuration);
	AnimateProperty(CONSTLIT("rotation"), pRotation);
	}

void IAnimatron::AnimatePropertyLinear (const CString &sID, const CString &sName, const CAniProperty &Start, const CAniProperty &End, int iDuration, bool bStartNow)

//	AnimatePropertyLinear
//
//	Animates the given property

	{
	CLinearAnimator *pAnimator = new CLinearAnimator;
	pAnimator->SetParams(Start, End, iDuration);
	AnimateProperty(sName, pAnimator, 0, sID, bStartNow);
	}

void IAnimatron::Scroll (EScrollTypes iScroll, int iScrollDist)

//	Scroll
//
//	Scrolls the appropriate distance

	{
	int iScrollPos = (int)GetPropertyMetric(PROP_SCROLL_POS);
	int iMaxScrollPos = (int)GetPropertyMetric(PROP_MAX_SCROLL_POS);

	int iNewPos;
	switch (iScroll)
		{
		case scrollToEnd:
			iNewPos = iMaxScrollPos;
			break;

		case scrollToHome:
			iNewPos = 0;
			break;

		case scrollRelative:
			iNewPos = Max(0, Min(iMaxScrollPos, iScrollPos + iScrollDist));
			break;

		default:
			iNewPos = iScrollPos;
		}

	if (iNewPos != iScrollPos)
		ScrollTo(iScrollPos, iNewPos);
	}

void IAnimatron::ScrollTo (int iOriginalPos, int iPos)

//	ScrollTo
//
//	Scroll to the given position

	{
	RemoveAnimation(ID_SCROLL_ANIMATION);

	CLinearAnimator *pAni = new CLinearAnimator;
	pAni->SetParams(CAniProperty((Metric)iOriginalPos), CAniProperty((Metric)iPos), 5);

	AnimateProperty(PROP_SCROLL_POS, pAni, 0, ID_SCROLL_ANIMATION, true);
	}
