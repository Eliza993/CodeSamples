#include "stdafx.h"
#include "LeComp_Familiar.h"
#include "LeComp_BattleMageBehavior.h"
#include "PE_Shot.h"

void LeComp_Familiar::FollowPlayer(float dt)
{
	pos = player->owner->GetPosition() + player->owner->GetForward() * (-2.0f) + offset;
	owner->SetPosition(pos);

	////can remove this if want to follow smoother, do this reduce load 
	//followDeltaTime += dt;
	////check pos every X delta
	//if (followDeltaTime >= 0.5f)
	//{
	//	pos = player->owner->GetPosition() + player->owner->GetForward() * (-2.0f) + offset;
	//	owner->SetPosition(pos);
	//	followDeltaTime = 0;
	//}
}


LeComp_Familiar::LeComp_Familiar(Object * owner):LeComp_MOB(owner)
{
}

LeComp_Familiar::LeComp_Familiar(Object * owner, LeComp_BattleMageBehavior * p): LeComp_MOB(owner)
{
	player = p;
	followDeltaTime = 0;
	offset = {0, 3.0f, 0};
	skillTree = SkillTreeFamiliar(player, this);

	pos = player->owner->GetPosition() + player->owner->GetForward() * (-2.0f) + offset;
	LeComp_StaticMesh *objMesh = owner->AddComponent<LeComp_StaticMesh>();
	objMesh->LoadObject("Boom.lme");
	objMesh->mat = player->owner->owningGameState->Renderer->GetMaterial("basicBoom");
	owner->SetPosition(pos);
}

LeComp_Familiar::~LeComp_Familiar()
{
}

void LeComp_Familiar::Update(float dt)
{
	FollowPlayer(dt);

	if (player->GetClass() == PlayerClass::Warrior)
	{
		skillTree.AttackTarget();
	}
	else if(player->GetClass() == PlayerClass::Mage)
	{
		skillTree.RecoverMp();
	}
	else if (player->GetClass() == PlayerClass::Ranger)
	{
		skillTree.RecoverHp();
	}
	else if (player->GetClass() == PlayerClass::Rogue)
	{
		skillTree.StealMoney();
	}
}
