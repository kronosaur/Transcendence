//	IDockScreenDisplay.cpp
//
//	IDockScreenDisplay class

#include "PreComp.h"
#include "Transcendence.h"

void IDockScreenDisplay::OnShowPane (bool bNoListNavigation) 

//	OnShowPane
//
//	Pane has been shown. This is normally overridden by subclasses, but by
//	default we clear the armor selection.
	
	{
	g_pTrans->SelectArmor(-1); 
	}
