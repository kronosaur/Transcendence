//	CAniShape.cpp
//
//	CAniShape class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

#define PROP_FILL_TYPE						CONSTLIT("fillType")
#define PROP_LINE_TYPE						CONSTLIT("lineType")

#define FILL_TYPE_NONE						CONSTLIT("none")
#define FILL_TYPE_SOLID						CONSTLIT("solid")

#define LINE_TYPE_NONE						CONSTLIT("none")
#define LINE_TYPE_SOLID						CONSTLIT("solid")

CAniShape::CAniShape (void) : 
		m_pFill(NULL), 
		m_pLine(NULL)

//	CAniShape constructor

	{
	}

CAniShape::~CAniShape (void)

//	CAniShape destructor

	{
	if (m_pFill)
		delete m_pFill;

	if (m_pLine)
		delete m_pLine;
	}

IAniFillMethod *CAniShape::GetFillMethod (void)

//	GetFillMethod
//
//	Returns the fill method for the shape based on properties

	{
	if (m_pFill == NULL)
		{
		CString sFill = GetPropertyString(PROP_FILL_TYPE);
		if (strEquals(sFill, FILL_TYPE_NONE))
			SetFillMethod(new CAniNullFill);
		else if (strEquals(sFill, FILL_TYPE_SOLID))
			SetFillMethod(new CAniSolidFill);
		else
			SetFillMethod(new CAniSolidFill);
		}

	return m_pFill;
	}

IAniLineMethod *CAniShape::GetLineMethod (void)

//	GetLineMethod
//
//	Returns the line method for the shape based on properties

	{
	if (m_pLine == NULL)
		{
		CString sLine = GetPropertyString(PROP_LINE_TYPE);
		if (strEquals(sLine, LINE_TYPE_SOLID))
			SetLineMethod(new CAniSolidLine);
		else
			SetLineMethod(new CAniNullLine);
		}

	return m_pLine;
	}

void CAniShape::SetFillMethod (IAniFillMethod *pFill)

//	SetFillMethod
//
//	Sets the fill method for the shape

	{
	if (m_pFill)
		delete m_pFill;

	m_pFill = pFill;
	m_pFill->InitDefaults(m_Properties);
	}

void CAniShape::SetLineMethod (IAniLineMethod *pLine)

//	SetLineMethod
//
//	Sets the line drawing method for the shape

	{
	if (m_pLine)
		delete m_pLine;

	m_pLine = pLine;
	m_pLine->InitDefaults(m_Properties);
	}
