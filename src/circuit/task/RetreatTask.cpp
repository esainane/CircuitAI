/*
 * RetreatTask.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: rlcevg
 */

#include "task/RetreatTask.h"
#include "task/TaskManager.h"
#include "module/BuilderManager.h"
#include "module/FactoryManager.h"
#include "setup/SetupManager.h"
#include "terrain/PathFinder.h"
#include "terrain/TerrainManager.h"
#include "unit/action/DGunAction.h"
#include "unit/action/MoveAction.h"
#include "CircuitAI.h"
#include "util/utils.h"

#include "AISCommands.h"
//#include "Weapon.h"

namespace circuit {

using namespace springai;

CRetreatTask::CRetreatTask(ITaskManager* mgr, int timeout)
		: IUnitTask(mgr, Priority::NORMAL, Type::RETREAT, timeout)
		, repairer(nullptr)
{
}

CRetreatTask::~CRetreatTask()
{
	PRINT_DEBUG("Execute: %s\n", __PRETTY_FUNCTION__);
}

void CRetreatTask::AssignTo(CCircuitUnit* unit)
{
	IUnitTask::AssignTo(unit);

	CCircuitDef* cdef = unit->GetCircuitDef();
	if (cdef->GetDGunMount() != nullptr) {
		CDGunAction* act = new CDGunAction(unit, cdef->GetDGunRange() * 0.8f);
		unit->PushBack(act);
	}
	unit->PushBack(new CMoveAction(unit));
	unit->SetRetreat(true);

	// Mobile repair
	CBuilderManager* builderManager = manager->GetCircuit()->GetBuilderManager();
	builderManager->EnqueueRepair(IBuilderTask::Priority::NORMAL, unit);
}

void CRetreatTask::RemoveAssignee(CCircuitUnit* unit)
{
	IUnitTask::RemoveAssignee(unit);

	unit->SetRetreat(false);
	if (units.empty()) {
		manager->DoneTask(this);
	}
}

void CRetreatTask::Execute(CCircuitUnit* unit)
{
	IUnitAction* act = static_cast<IUnitAction*>(unit->End());
	if (act->GetType() != IUnitAction::Type::MOVE) {
		return;
	}
	CMoveAction* moveAction = static_cast<CMoveAction*>(act);

	CCircuitAI* circuit = manager->GetCircuit();
	int frame = circuit->GetLastFrame();
	CFactoryManager* factoryManager = circuit->GetFactoryManager();
	AIFloat3 startPos = unit->GetPos(frame);
	AIFloat3 endPos = factoryManager->GetClosestHaven(unit);
	if (endPos == -RgtVector) {
		endPos = circuit->GetSetupManager()->GetBasePos();
	}
	F3Vec path;

	CPathFinder* pathfinder = circuit->GetPathfinder();
	float range = factoryManager->GetAssistDef()->GetBuildDistance() * 0.6f + pathfinder->GetSquareSize();
	pathfinder->SetMapData(unit, circuit->GetThreatMap(), frame);
	pathfinder->MakePath(path, startPos, endPos, range);

	if (path.empty()) {
		path.push_back(endPos);
	}
	moveAction->SetPath(path);
	unit->Update(circuit);
}

void CRetreatTask::Update()
{
	CCircuitAI* circuit = manager->GetCircuit();
	bool isExecute = (++updCount % 4 == 0);
	auto assignees = units;
	for (CCircuitUnit* unit : assignees) {
		Unit* u = unit->GetUnit();
		const float healthPerc = u->GetHealth() / u->GetMaxHealth();
		bool isRepaired;
		// FIXME: Wait until 101.0 engine
//		if (ass->GetShield() != nullptr) {
//			isRepaired = (healthPerc > 0.9f) && (unit->GetShield()->GetShieldPower() > unit->GetCircuitDef()->GetMaxShield() * 0.9f);
//		} else {
			isRepaired = (healthPerc > (unit->GetCircuitDef()->IsAbleToFly() ? 0.99f : 0.9f));
//		}

		if (isRepaired) {
			RemoveAssignee(unit);
		} else if (unit->IsForceExecute() || isExecute) {
			Execute(unit);
		} else {
			unit->Update(circuit);
		}
	}
}

void CRetreatTask::OnUnitIdle(CCircuitUnit* unit)
{
	CCircuitAI* circuit = manager->GetCircuit();
	CFactoryManager* factoryManager = circuit->GetFactoryManager();
	AIFloat3 haven = factoryManager->GetClosestHaven(unit);
	if (haven == -RgtVector) {
		haven = circuit->GetSetupManager()->GetBasePos();
	}
	const float maxDist = factoryManager->GetAssistDef()->GetBuildDistance();
	int frame = circuit->GetLastFrame();
	const AIFloat3& unitPos = unit->GetPos(frame);
	if (unitPos.SqDistance2D(haven) > maxDist * maxDist) {
		// TODO: push MoveAction into unit? to avoid enemy fire
		unit->GetUnit()->MoveTo(haven, UNIT_COMMAND_OPTION_INTERNAL_ORDER, frame + FRAMES_PER_SEC * 1);
		// TODO: Add fail counter?
	} else {
		// TODO: push WaitAction into unit
		unit->GetUnit()->ExecuteCustomCommand(CMD_PRIORITY, {0.0f});

		AIFloat3 pos = unitPos;
		const float size = SQUARE_SIZE * 50;
		CTerrainManager* terrainManager = circuit->GetTerrainManager();
		float centerX = terrainManager->GetTerrainWidth() / 2;
		float centerZ = terrainManager->GetTerrainHeight() / 2;
		pos.x += (pos.x > centerX) ? size : -size;
		pos.z += (pos.z > centerZ) ? size : -size;
		AIFloat3 oldPos = pos;
		terrainManager->CorrectPosition(pos);
		if (oldPos.SqDistance2D(pos) > SQUARE_SIZE * SQUARE_SIZE) {
			pos = unitPos;
			pos.x += (pos.x > centerX) ? -size : size;
			pos.z += (pos.z > centerZ) ? -size : size;
		}
		unit->GetUnit()->PatrolTo(pos);

		IUnitAction* act = static_cast<IUnitAction*>(unit->End());
		if (act->GetType() == IUnitAction::Type::MOVE) {
			static_cast<CMoveAction*>(act)->SetFinished(true);
		}
	}
}

void CRetreatTask::OnUnitDamaged(CCircuitUnit* unit, CEnemyUnit* attacker)
{
	//TODO: Rebuild retreat path?
}

void CRetreatTask::OnUnitDestroyed(CCircuitUnit* unit, CEnemyUnit* attacker)
{
	RemoveAssignee(unit);
}

} // namespace circuit
