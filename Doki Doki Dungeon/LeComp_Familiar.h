#pragma once
#include "stdafx.h"
#include "LeComp_Behavior.h"
#include "LeComp_MOB.h"
#include "Skills.h"

class LeComp_BattleMageBehavior;

//Familiar flow Player & attack the Enemy + heal Player
class LeComp_Familiar : public LeComp_MOB
{
	LeComp_BattleMageBehavior *player;
	SkillTreeFamiliar skillTree;

	float3 pos;
	float3 curPos;
	float3 offset;

	//float curAtkTime;
	const float atkTime = 3.0f;
	float followDeltaTime;
	int numPE = 0;

	//Find path to Player & go to Player
	void FollowPlayer(float dt);

public:
	LeComp_Familiar(Object *owner);
	LeComp_Familiar(Object *owner, LeComp_BattleMageBehavior *p);
	~LeComp_Familiar();

	void Update(float dt);
	float3 GetPos() { return pos; }
	LeComp_BattleMageBehavior* GetPlayer() { return player; }
};