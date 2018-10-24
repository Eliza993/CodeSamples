#pragma once
#include "./PE_Status.h"

class LeComp_PlayerCamera;

class DirectionalTargeting
{
protected:
	LeComp_MOB *player;
	LeComp_MOB* target;
	ControllerAxis stickDir;
	LeComp_PlayerCamera *camera = nullptr;
	float maxDis; //skill effect area/dis
	bool active;
	float3 forward;
	float3 right;
	bool defaultTarget = true;

	//for PE
	Object *obj;
	PE_Status *peArrow;
	float3 arrowPos;
	float3 dir;
	float rotate;
	float dis; //between P & E

	int MinTargetAngle(vector<float> vec);

public:
	DirectionalTargeting() {};
	DirectionalTargeting(LeComp_MOB *_player, LeComp_PlayerCamera *_camera, float _maxDis, int _peIndex);
	~DirectionalTargeting() {};
	
	LeComp_MOB* GetTarget();

	float GetMaxDistance()  const {return maxDis; }
	bool GetActive() { return active; }

	void SetStickDir(ControllerAxis _stick) { stickDir = _stick; }
	void SetMaxDis(float _maxDis) { maxDis = maxDis; }
	void SetActive(bool _active) { active = _active; }
	virtual void Update(float dt);
};