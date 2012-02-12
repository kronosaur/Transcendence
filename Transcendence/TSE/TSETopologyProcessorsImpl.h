//	TSETopologyProcessorsImpl.h
//
//	Transcendence ITopologyProcessor classes
//
//	ADDING A NEW PROCESSOR
//
//	1.	Define a new class descended from ITopologyProcessor
//	2.	Implement OnInitFromXML
//	3.	Implement OnProcess
//	4.	Add an entry to ITopologyProcessor::CreateFromXML

#ifndef INCL_TSE_TOPOLOGY_PROCESSORS
#define INCL_TSE_TOPOLOGY_PROCESSORS

class CApplySystemProc : public ITopologyProcessor
	{
	public:
		virtual ~CApplySystemProc (void);

	protected:
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		CXMLElement *m_pSystemDesc;
	};

class CConquerNodesProc : public ITopologyProcessor
	{
	protected:
		virtual ~CConquerNodesProc (void);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		struct SNodeWeight
			{
			CTopologyNode::SCriteria Criteria;
			int iWeight;
			int iSuccessChance;
			};

		struct SConqueror
			{
			ITopologyProcessor *pProc;			//	Processor to apply to partition

			int iSeedChance;					//	Chance that a new seed will start this round
			int iMaxSeeds;						//	Do not allow more than this number of seeds to start
			int iExpandChance;					//	Chance that we expand this round
			int iMaxNodes;						//	Max number of total nodes
			TArray<SNodeWeight> Seed;			//	Probability of seeding a given node
			TArray<SNodeWeight> Expand;			//	Probability of expanding to a given node

			int iSeedCount;						//	Number of seeds placed
			int iNodeCount;						//	Number of nodes placed
			};

		struct SSeed
			{
			SConqueror *pConqueror;
			CTopologyNodeList Nodes;
			};

		void CalcAdjacentNodes (CTopologyNodeList &NodeList, CTopologyNodeList *retOutput);
		int CalcNodeWeight (CTopologyNode *pNode, TArray<SNodeWeight> &Weights, int *retiSuccessChance = NULL);
		CTopologyNode *ChooseRandomNode (CTopologyNodeList &NodeList, TArray<SNodeWeight> &Weights);
		void ConquerNode (SConqueror *pConqueror, SSeed *pSeed, CTopologyNode *pNode);
		inline TArray<SNodeWeight> &GetExpandWeightTable (SConqueror *pConqueror) { return (pConqueror->Expand.GetCount() > 0 ? pConqueror->Expand : pConqueror->Seed); }
		inline TArray<SNodeWeight> &GetSeedWeightTable (SConqueror *pConqueror) { return (pConqueror->Seed.GetCount() > 0 ? pConqueror->Seed : pConqueror->Expand); }
		ALERROR LoadNodeWeightTable (SDesignLoadCtx &Ctx, CXMLElement *pDesc, TArray<SNodeWeight> *retTable);

		CTopologyNode::SCriteria m_Criteria;
		TArray<SConqueror> m_Conquerors;
	};

class CDistributeNodesProc : public ITopologyProcessor
	{
	public:
		virtual ~CDistributeNodesProc (void);

	protected:
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		struct SSystemEntry
			{
			CXMLElement *pDesc;
			};

		CTopologyNode::SCriteria m_Criteria;
		TArray<SSystemEntry> m_Systems;
	};

class CFillNodesProc : public ITopologyProcessor
	{
	public:
		virtual ~CFillNodesProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		CTopologyNode::SCriteria m_Criteria;
		TArray<ITopologyProcessor *> m_Procs;
	};

class CGroupTopologyProc : public ITopologyProcessor
	{
	public:
		virtual ~CGroupTopologyProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		CTopologyNode::SCriteria m_Criteria;
		TArray<ITopologyProcessor *> m_Procs;
	};

class CLocateNodesProc : public ITopologyProcessor
	{
	public:
		CLocateNodesProc (void) : m_pMapFunction(NULL) { }
		virtual ~CLocateNodesProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		struct SLocation
			{
			ITopologyProcessor *pProc;
			float rMin;
			float rMax;
			};

		ALERROR ParseRange (SDesignLoadCtx &Ctx, const CString &sRange, float *retrMin, float *retrMax);

		CTopologyNode::SCriteria m_Criteria;
		I2DFunction *m_pMapFunction;
		TArray<SLocation> m_Locations;
	};

class CPartitionNodesProc : public ITopologyProcessor
	{
	public:
		virtual ~CPartitionNodesProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		enum EPartitionOrders
			{
			orderFixed,
			orderRandom,
			};

		struct SPartition
			{
			ITopologyProcessor *pProc;			//	Processor to apply to partition
			DiceRange NodeCount;				//	Number of nodes in this partition
			bool bNodeCountPercent;				//	Node count is a percentage
			int iMaxPartitions;					//	Max number of instances of this partition

			int iPartitionsCreated;				//	Instances of this partition created so far
			};

		int CreatePartitionRandomWalk (int iCount, CTopologyNode *pStart, CTopologyNodeList *retList);
		bool IsAvailable (SPartition &Partition);

		CTopologyNode::SCriteria m_Criteria;
		TArray<SPartition> m_Partitions;
		EPartitionOrders m_iOrder;
	};

class CRandomPointsProc : public ITopologyProcessor
	{
	public:
		CRandomPointsProc (void) : m_pAreaDef(NULL) { }
		~CRandomPointsProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		enum ERotationTypes
			{
			rotationNone,						//	Do not rotate images
			rotationCenter,						//	m_xRotation and m_yRotation are the center
			rotationConstant,					//	m_xRotation is a constant
			rotationTangent,					//	m_xRotation and m_yRotation are the center
			};

		struct SPointProc
			{
			int iChance;						//	Chance that we use this point proc

			ITopologyProcessor *pProc;			//	Processor to apply to selected nodes
			int iMinNodeDist;					//	Exclude all points below this distance from the point (0 = no limit)
			int iMaxNodeDist;					//	Exclude all points beyond this distance from the point (-1 = no limit)

			CEffectCreatorRef pLabelEffect;		//	Effect to paint on label layer
			CEffectCreatorRef pMapEffect;		//	Effect to paint on galactic map
			};

		int GenerateRotation (int x, int y);

		DiceRange m_Points;						//	Number of points to generate
		int m_iMinSeparation;					//	Minimum separation between points
		CXMLElement *m_pAreaDef;				//	Area definition

		ERotationTypes m_iRotationType;			//	Used to compute image rotations
		int m_xRotation;
		int m_yRotation;

		CTopologyNode::SCriteria m_Criteria;
		TArray<SPointProc> m_PointProcs;
		int m_iTotalChance;
	};

class CTableTopologyProc : public ITopologyProcessor
	{
	public:
		virtual ~CTableTopologyProc (void);

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError);

	private:
		struct SEntry
			{
			ITopologyProcessor *pProc;
			int iChance;
			};

		CTopologyNode::SCriteria m_Criteria;
		TArray<SEntry> m_Procs;
		int m_iTotalChance;
	};

#endif

