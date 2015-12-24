/*
 * FighterTask.h
 *
 *  Created on: Aug 31, 2015
 *      Author: rlcevg
 */

#ifndef SRC_CIRCUIT_TASK_FIGHTER_FIGHTERTASK_H_
#define SRC_CIRCUIT_TASK_FIGHTER_FIGHTERTASK_H_

#include "task/UnitTask.h"
#include "util/Defines.h"

namespace circuit {

class CEnemyUnit;

class IFighterTask: public IUnitTask {
public:
	enum class FightType: char {DEFEND, SCOUT, ATTACK};

protected:
	IFighterTask(ITaskManager* mgr, FightType type, int timeout = ASSIGN_TIMEOUT);
public:
	virtual ~IFighterTask();

	virtual void AssignTo(CCircuitUnit* unit);
	virtual void RemoveAssignee(CCircuitUnit* unit);

	virtual void Update();

	virtual void OnUnitIdle(CCircuitUnit* unit);
	virtual void OnUnitDamaged(CCircuitUnit* unit, CEnemyUnit* attacker);
	virtual void OnUnitDestroyed(CCircuitUnit* unit, CEnemyUnit* attacker);

	FightType GetFightType() const { return fightType; }

protected:
	FightType fightType;
	springai::AIFloat3 position;  // attack/scout position

	float attackPower;
	CEnemyUnit* target;
};

} // namespace circuit

#endif // SRC_CIRCUIT_TASK_FIGHTER_FIGHTERTASK_H_
