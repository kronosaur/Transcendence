//	Euclid.h
//
//	Basic geometry definitions

#ifndef INCL_EUCLID
#define INCL_EUCLID

#ifndef _INC_MATH
#include <math.h>
#endif

typedef double Metric;

//	Angles

bool AngleInArc (int iAngle, int iMinAngle, int iMaxAngle);
inline int AngleMod (int iAngle) { if (iAngle >= 0) return (iAngle % 360); else return 360 - (-iAngle % 360); }
inline int AngleMiddle (int iLowAngle, int iHighAngle)
//	Returns the angle in between the two given angles. If iLowAngle > iHighAngle then
//	we wrap around through 0
	{
	if (iLowAngle > iHighAngle)
		return (iLowAngle + ((iHighAngle + 360 - iLowAngle) / 2)) % 360;
	else
		return iLowAngle + ((iHighAngle - iLowAngle) / 2);
	}

inline Metric AngleToRadians (int iAngle) { return iAngle * g_Pi / 180.0; }
inline int AngleToDegrees (Metric rAngle) { return AngleMod((int)(rAngle * 180.0 / g_Pi)); }

//	2d vector class

class CVector
	{
	public:
		CVector (void) : x(0.0), y(0.0) { }
		CVector (Metric ix, Metric iy) : x(ix), y(iy) { }

		inline bool operator == (const CVector &vA) const { return (x == vA.x && y == vA.y); }

		bool Clip (Metric rLength);
		inline Metric Dot (const CVector &vA) const { return x * vA.x + y * vA.y; }
		inline const Metric &GetX (void) const { return x; }
		inline const Metric &GetY (void) const { return y; }
		inline bool InBox (const CVector &vUR, const CVector &vLL) const { return (x >= vLL.x && x < vUR.x	&& y >= vLL.y && y < vUR.y); }
		inline bool IsNull (void) const { return (x == 0.0 && y == 0.0); }
		inline Metric Length (void) const { return sqrt(x * x + y * y); }
		inline Metric Length2 (void) const { return (x * x + y * y); }
		inline Metric Longest (void) const
			{
			Metric ax = (x < 0.0 ? -x : x);
			Metric ay = (y < 0.0 ? -y : y);
			return (ax > ay ? ax : ay);
			}
		inline CVector Normal (void) const 
			{
			Metric rLength = Length();
			if (rLength == 0.0)
				return CVector();
			else
				return CVector(x / rLength, y / rLength);
			}
		inline CVector Normal (Metric *retrLength) const
			{
			*retrLength = Length();
			if (*retrLength == 0.0)
				return CVector();
			else
				return CVector(x / *retrLength, y / *retrLength);
			}
		void GenerateOrthogonals (const CVector &vNormal, Metric *retvPara, Metric *retvPerp) const;
		inline CVector Perpendicular (void) const { return CVector(-y, x); }
		inline CVector Reflect (void) const { return CVector(-x, -y); }
		CVector Rotate (int iAngle) const;
		inline void SetX (Metric NewX) { x = NewX; }
		inline void SetY (Metric NewY) { y = NewY; }

	private:
		Metric x;
		Metric y;
	};

extern const CVector NullVector;
extern Metric g_Cosine[360];
extern Metric g_Sine[360];

//	Vector-Vector operations
inline const CVector operator+ (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() + op2.GetX(), op1.GetY() + op2.GetY()); }
inline const CVector operator- (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() - op2.GetX(), op1.GetY() - op2.GetY()); }
inline const CVector operator* (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() * op2.GetX(), op1.GetY() * op2.GetY()); }
inline const CVector operator- (const CVector &op) { return CVector(-op.GetX(), -op.GetY()); }

//	Vector-scalar operations
inline const CVector operator* (const CVector &op1, const Metric op2) { return CVector(op1.GetX() * op2, op1.GetY() * op2); }
inline const CVector operator* (const Metric op2, const CVector &op1) { return CVector(op1.GetX() * op2, op1.GetY() * op2); }
inline const CVector operator/ (const CVector &op1, const Metric op2) { return CVector(op1.GetX() / op2, op1.GetY() / op2); }

//	Transform class

enum XFormType
	{
	xformIdentity,

	xformTranslate,
	xformScale,
	xformRotate,
	};

class CXForm
	{
	public:
		CXForm (void);
		CXForm (XFormType type);
		CXForm (XFormType type, Metric rX, Metric rY);
		CXForm (XFormType type, const CVector &vVector);
		CXForm (XFormType type, int iAngle);

		void Transform (Metric x, Metric y, Metric *retx, Metric *rety) const;
		CVector Transform (const CVector &vVector) const;

	private:
		Metric m_Xform[3][3];

	friend const CXForm operator* (const CXForm &op1, const CXForm &op2);
	};

const CXForm operator* (const CXForm &op1, const CXForm &op2);

//	Shapes ---------------------------------------------------------------------

void CreateArcPolygon (Metric rInnerRadius, Metric rOuterRadius, int iArc, TArray<CVector> *retPoints);

//	Graphs ---------------------------------------------------------------------

class CIntGraph
	{
	public:
		CIntGraph (void);

		void AddGraph (CIntGraph &Source);
		void AddNode (int x, int y, DWORD *retdwID = NULL);
		void Connect (DWORD dwFromID, DWORD dwToID);
		void DeleteAll (void);
		bool FindNearestNode (int x, int y, DWORD *retdwID);
		void GenerateRandomConnections (DWORD dwStartNode, int iMinConnections, int iMaxConnections);
		int GetNodeCount (void);
		int GetNodeForwardConnections (DWORD dwID, TArray<int> *retConnections);
		DWORD GetNodeID (int iIndex);
		int GetNodeIndex (DWORD dwID);
		void GetNodePos (DWORD dwID, int *retx, int *rety);
		bool IsCrossingConnection (DWORD dwFromID, DWORD dwToID);

	private:
		struct SNode
			{
			int x;							//	Coordinates
			int y;

			int iFirstForward;				//	First forward connection (-1 if no connections; -2 if this node is free)
			int iFirstBackward;				//	First backward connection (-1 if none; if free, index of next free block)
			};

		struct SConnection
			{
			int iFrom;						//	Node ID (-1 if free)
			int iTo;						//	Node ID

			int iNext;						//	Next connection
			};

		int AllocConnection (void);
		int AllocNode (void);
		void CreateNodeIndex (void);
		void FreeConnection (int iConnection);
		void FreeNode (int iNode);
		inline SConnection *GetConnection (int iConnection) { return &m_Connections[iConnection]; }
		inline SConnection *GetForwardConnection (SNode *pNode) { return (pNode->iFirstForward >= 0 ? GetConnection(pNode->iFirstForward) : NULL); }
		inline SConnection *GetNextConnection (SConnection *pConnection) { return (pConnection->iNext >= 0 ? GetConnection(pConnection->iNext) : NULL); }
		inline int GetNextFreeNode (SNode *pNode) { return (pNode->iFirstBackward); }
		inline SNode *GetNode (int iNode) { return &m_Nodes[iNode]; }
		inline void MakeNodeFree (SNode *pNode, int iNextFree) { pNode->iFirstForward = -2; pNode->iFirstBackward = iNextFree; }
		inline bool NodeIsFree (SNode *pNode) { return (pNode->iFirstForward == -2); }

		TArray<SNode> m_Nodes;
		TArray<SConnection> m_Connections;
		int m_iFirstFreeNode;				//	-1 if none
		int m_iFirstFreeConnection;			//	-1 if none

		bool m_bNodeIndexValid;				//	TRUE if node index is valid
		TArray<int> m_NodeIndex;			//	Index of valid nodes
	};

//	Numbers

class CIntegerIP
	{
	public:
		CIntegerIP (void) : m_iCount(0), m_pNumber(NULL) { }
		CIntegerIP (const CIntegerIP &Src);
		CIntegerIP (int iCount, BYTE *pNumber = NULL);
		~CIntegerIP (void);

		CIntegerIP &operator= (const CIntegerIP &Src);
		bool operator== (const CIntegerIP &Src) const;

		CString AsBase64 (void) const;
		inline BYTE *GetBytes (void) const { return m_pNumber; }
		inline int GetLength (void) const { return m_iCount; }
		inline bool IsEmpty (void) const { return (m_pNumber == NULL); }
		void TakeHandoff (CIntegerIP &Src);

	private:
		void CleanUp (void);
		void Copy (const CIntegerIP &Src);
	
		int m_iCount;						//	Number of bytes
		BYTE *m_pNumber;					//	Array of bytes in big-endian order
	};

//	Statistics

template <class VALUE> class TNumberSeries
	{
	public:
		struct SHistogramPoint
			{
			int iValue;
			int iCount;
			double rPercent;
			};

#ifdef DEBUG
		void DumpSeries (void) const
			{
			int i;

			for (i = 0; i < m_Series.GetCount(); i++)
				printf("%d ", (int)m_Series[i]);

			printf("\n");
			}
#endif

		void CalcHistogram (TArray<SHistogramPoint> *retHistogram) const
			{
			int i;

			retHistogram->DeleteAll();

			if (m_Series.GetCount() == 0)
				return;

			TSortMap<int, int> Counts;
			for (i = 0; i < m_Series.GetCount(); i++)
				{
				bool bNew;
				int *pCount = Counts.SetAt((int)m_Series[i], &bNew);
				if (bNew)
					*pCount = 1;
				else
					*pCount += 1;
				}

			for (i = 0; i < Counts.GetCount(); i++)
				{
				SHistogramPoint *pPoint = retHistogram->Insert();
				pPoint->iValue = Counts.GetKey(i);
				pPoint->iCount = Counts[i];
				pPoint->rPercent = pPoint->iCount / (double)m_Series.GetCount();
				}
			}

		inline VALUE GetMean (void) const { return (m_Series.GetCount() > 0 ? (m_Total / m_Series.GetCount()) : 0); }
		inline VALUE GetMedian (void) const
			{
			int iCount = m_Series.GetCount();

			if (iCount == 0)
				return 0;
			else if ((iCount % 2) == 1)
				return m_Series[iCount / 2];
			else
				{
				VALUE Low = m_Series[(iCount / 2) - 1];
				VALUE Hi = m_Series[(iCount / 2)];
				return (Low + Hi) / 2;
				}
			}

		inline VALUE GetMax (void) const { return (m_Series.GetCount() > 0 ? m_Max : 0); }
		inline VALUE GetMin (void) const { return (m_Series.GetCount() > 0 ? m_Min : 0); }

		void Insert (VALUE Value)
			{
			//	Update min, max, and total

			if (m_Series.GetCount() == 0)
				{
				m_Total = Value;
				m_Min = Value;
				m_Max = Value;
				}
			else
				{
				if (Value > m_Max)
					m_Max = Value;
				else if (Value < m_Min)
					m_Min = Value;
				
				m_Total += Value;
				}

			//	Insert

			m_Series.InsertSorted(Value);
			}

	private:
		TArray<VALUE> m_Series;
		VALUE m_Min;
		VALUE m_Max;
		VALUE m_Total;
	};

//	Functions

void EuclidInit (void);

void IntPolarToVector (int iAngle, Metric rRadius, int *retx, int *rety);
inline void IntPolarToVector (int iAngle, int iRadius, int *retx, int *rety) { IntPolarToVector(iAngle, (Metric)iRadius, retx, rety); }
bool IntSegmentsIntersect (int A1x, int A1y, int A2x, int A2y, int B1x, int B1y, int B2x, int B2y);
int IntVectorToPolar (int x, int y, int *retiRadius = NULL);

CVector PolarToVector (int iAngle, Metric rRadius);
CVector PolarToVectorRadians (Metric rRadians, Metric rRadius);
int VectorToPolar (const CVector &vP, Metric *retrRadius = NULL);
Metric VectorToPolarRadians (const CVector &vP, Metric *retrRadius = NULL);

#endif
