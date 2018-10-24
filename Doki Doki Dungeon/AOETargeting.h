#pragma once
#include "DirectionalTargeting.h"

class AOETargeting : public DirectionalTargeting
{
	float3 aimPos;
public: 
	AOETargeting() {};
	AOETargeting(LeComp_MOB *_player, LeComp_PlayerCamera *_camera, float _maxDis, int _peIndex);
	~AOETargeting() {};

	virtual void Update(float dt) override;

	float3 GetAimPosition() 
	{ 
		return aimPos; 
	}
};