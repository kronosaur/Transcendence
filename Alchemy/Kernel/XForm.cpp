//	XForm.cpp
//
//	Implements XForm object

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"

CXForm::CXForm (void)
	{
	}

CXForm::CXForm (XFormType type)
	{
	ASSERT(type == xformIdentity);

	m_Xform[0][0] = 1.0;
	m_Xform[0][1] = 0.0;
	m_Xform[0][2] = 0.0;

	m_Xform[1][0] = 0.0;
	m_Xform[1][1] = 1.0;
	m_Xform[1][2] = 0.0;

	m_Xform[2][0] = 0.0;
	m_Xform[2][1] = 0.0;
	m_Xform[2][2] = 1.0;
	}

CXForm::CXForm (XFormType type, Metric rX, Metric rY)
	{
	switch (type)
		{
		case xformTranslate:
			m_Xform[0][0] = 1.0;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = 1.0;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = rX;
			m_Xform[2][1] = rY;
			m_Xform[2][2] = 1.0;
			break;

		case xformScale:
			m_Xform[0][0] = rX;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = rY;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;

		default:
			ASSERT(false);
		}
	}

CXForm::CXForm (XFormType type, const CVector &vVector)
	{
	switch (type)
		{
		case xformTranslate:
			m_Xform[0][0] = 1.0;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = 1.0;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = vVector.GetX();
			m_Xform[2][1] = vVector.GetY();
			m_Xform[2][2] = 1.0;
			break;

		case xformScale:
			m_Xform[0][0] = vVector.GetX();
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = vVector.GetY();
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;

		default:
			ASSERT(false);
		}
	}

CXForm::CXForm (XFormType type, int iAngle)
	{
	switch (type)
		{
		case xformRotate:
			{
			Metric rCos = cos(g_Pi * (Metric)iAngle / 180.0);
			Metric rSin = sin(g_Pi * (Metric)iAngle / 180.0);

			m_Xform[0][0] = rCos;
			m_Xform[0][1] = rSin;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = -rSin;
			m_Xform[1][1] = rCos;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CXForm::Transform (Metric x, Metric y, Metric *retx, Metric *rety) const
	{
	Metric xNew = x * m_Xform[0][0] + y * m_Xform[1][0] + m_Xform[2][0];
	Metric yNew = x * m_Xform[0][1] + y * m_Xform[1][1] + m_Xform[2][1];

	*retx = xNew;
	*rety = yNew;
	}

CVector CXForm::Transform (const CVector &vVector) const
	{
	return CVector(
			vVector.GetX() * m_Xform[0][0] + vVector.GetY() * m_Xform[1][0] + m_Xform[2][0],
			vVector.GetX() * m_Xform[0][1] + vVector.GetY() * m_Xform[1][1] + m_Xform[2][1]
			);
	}

const CXForm operator* (const CXForm &op1, const CXForm &op2)
	{
	CXForm Result;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			Result.m_Xform[i][j] = 
					  op1.m_Xform[i][0] * op2.m_Xform[0][j]
					+ op1.m_Xform[i][1] * op2.m_Xform[1][j]
					+ op1.m_Xform[i][2] * op2.m_Xform[2][j];

	return Result;
	}

