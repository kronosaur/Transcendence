//	Utilities.h
//
//	Miscellaneous classes
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved

#pragma once

//	Keyboard Tracker ----------------------------------------------------------

class CKeyboardTracker
	{
	public:
		inline int GetKeyDownCount (void) { return m_KeysDown.GetCount(); }
		inline void OnKeyDown (int iVirtKey) { m_KeysDown.Insert(iVirtKey); }
		inline void OnKeyUp (int iVirtKey) { int iIndex; if (m_KeysDown.Find(iVirtKey, &iIndex)) m_KeysDown.Delete(iIndex); }

	private:
		TArray<int> m_KeysDown;
	};

