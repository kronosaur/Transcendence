//	CNameGenerator.cpp
//
//	CNameGenerator class

#include "PreComp.h"

CNameGenerator::CNameGenerator (void)

//	CNameGenerator constructor

	{
	}

CNameGenerator::~CNameGenerator (void)

//	CNameGenerator destructor

	{
	}

ALERROR CNameGenerator::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	return NOERROR;
	}

ALERROR CNameGenerator::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML element

	{
	return NOERROR;
	}
