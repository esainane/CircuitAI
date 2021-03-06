/*
 * EnergyLink.h
 *
 *  Created on: Apr 29, 2015
 *      Author: rlcevg
 */

#ifndef SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_
#define SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_

#include "unit/CircuitUnit.h"

#include <map>
#include <set>

namespace circuit {

class CEnergyLink {
public:
	struct SPylon {
		SPylon() : pos(-RgtVector), range(.0f) {}
		SPylon(const springai::AIFloat3& p, float r) : pos(p), range(r) {}
		springai::AIFloat3 pos;
		float range;
		std::set<SPylon*> neighbors;
	};
	struct SVertex {
		SVertex(int index, const springai::AIFloat3& pos) : index(index), pos(pos) {}
		std::set<SPylon*> pylons;
		int index;
		springai::AIFloat3 pos;
	};

	CEnergyLink(int idx0, const springai::AIFloat3& P0, int idx1, const springai::AIFloat3& P1);
	virtual ~CEnergyLink();

	void AddPylon(ICoreUnit::Id unitId, const springai::AIFloat3& pos, float range);
	bool RemovePylon(ICoreUnit::Id unitId);
	void CheckConnection();
	SPylon* GetConnectionHead(SVertex* v0, const springai::AIFloat3& P1);

	void SetBeingBuilt(bool value) { isBeingBuilt = value; }
	bool IsBeingBuilt() const { return isBeingBuilt; }
	bool IsFinished() const { return isFinished; }
	void SetValid(bool value) { isValid = value; }
	bool IsValid() const { return isValid; }
	void SetStartVertex(int index);
	SVertex* GetV0() const { return v0; }
	SVertex* GetV1() const { return v1; }

private:
	SVertex *v0, *v1;

	std::map<ICoreUnit::Id, SPylon*> pylons;  // owner
	bool isBeingBuilt;
	bool isFinished;
	bool isValid;
};

} // namespace circuit

#endif // SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_
