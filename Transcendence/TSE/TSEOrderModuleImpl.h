//	TSEOrderModuleImpl.h
//
//	Transcendence IOrderModule classes

#ifndef INCL_TSE_ORDER_MODULES
#define INCL_TSE_ORDER_MODULES

class CAttackOrder : public IOrderModule
	{
	public:
		CAttackOrder (void) : IOrderModule(objCount)
			{ }

	protected:
		//	IOrderModule virtuals
		virtual bool IsAttacking (void) { return true; }
		virtual bool IsTarget (CSpaceObject *pObj) { return (pObj == m_Objs[objTarget]); }
		virtual bool IsTarget (int iObj) { return (iObj == objTarget); }
		virtual void OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx);
		virtual void OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData);
		virtual IShipController::OrderTypes OnGetOrder (void) { return IShipController::orderDestroyTarget; }
		virtual CSpaceObject *OnGetTarget (void) { return m_Objs[objTarget]; }
		virtual void OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		enum Objs
			{
			objTarget =		0,
			objAvoid =		1,

			objCount =		2,
			};

		enum States
			{
			stateAttackingTargetAndAvoiding,
			stateAvoidingEnemyStation,
			};

		States m_iState;						//	Current behavior state
		int m_iCountdown;						//	Stop attacking after this time
	};

class CAttackStationOrder : public IOrderModule
	{
	public:
		CAttackStationOrder (void) : IOrderModule(objCount)
			{ }

	protected:
		//	IOrderModule virtuals
		virtual bool IsAttacking (void) { return true; }
		virtual bool IsTarget (CSpaceObject *pObj) { return ((pObj == m_Objs[objTarget]) || (pObj == m_Objs[objDefender])); }
		virtual bool IsTarget (int iObj) { return true; }
		virtual void OnAttacked (CShip *pShip, CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual void OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx);
		virtual void OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData);
		virtual IShipController::OrderTypes OnGetOrder (void) { return IShipController::orderAttackStation; }
		virtual CSpaceObject *OnGetTarget (void) { return m_Objs[objTarget]; }
		virtual void OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		enum Objs
			{
			objTarget =		0,
			objDefender =	1,

			objCount =		2,
			};

		enum States
			{
			stateAttackingTarget,
			stateAttackingDefender,
			};

		States m_iState;						//	Current behavior state
		int m_iCountdown;						//	Stop attacking after this time
	};

class CGuardOrder : public IOrderModule
	{
	public:
		CGuardOrder (void) : IOrderModule(0)
			{ }

	protected:
		//	IOrderModule virtuals
		virtual void OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx);
		virtual void OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData);
		virtual IShipController::OrderTypes OnGetOrder (void) { return IShipController::orderGuard; }
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		enum States
			{
			stateWaitingForThreat,
			stateReturningViaNavPath,
			stateReturningFromThreat,
			};

		States m_iState;						//	Current behavior state
		CSpaceObject *m_pBase;					//	Object that we're guarding
	};

#endif
