#include "stdafx.h"
#include "DirectionalTargeting.h"
#include "./Enemy.h"
#include "LeComp_PlayerCamera.h"

int DirectionalTargeting::MinTargetAngle(vector<float> vec)
{
	int minIndex = 0;
	for (unsigned int i = 0; i < vec.size() - 1; i++)
	{
		if (vec[i] > vec[i + 1])
		{
			minIndex = i + 1;
		}
	}	                   
	return minIndex;
}

DirectionalTargeting::DirectionalTargeting(LeComp_MOB * _player, LeComp_PlayerCamera *_camera, float _maxDis, int _peIndex)
{
	player = _player;
	camera = _camera;
	maxDis = _maxDis;
	active = false;


	forward = float3(0, 0, 1).GetNormalized();
	right = float3(1, 0, 0).GetNormalized();

	LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage && battleMage->GetTarget())
	{
		target = battleMage->GetTarget();

		arrowPos = (player->owner->GetPosition() + target->owner->GetPosition()) / 2;
		dir = target->owner->GetPosition() - player->owner->GetPosition();
		dis = target->owner->GetPosition().GetDistance(player->owner->GetPosition());
		rotate = 180-XMConvertToDegrees(atan2(dir.x, dir.z));
	}
	else
	{
		arrowPos = {0, 0, 0};
		dir = { 0, 0, 0 };
		dis = 0;
		rotate = 0;
	}

	obj = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE Directional Targeting" + std::to_string(_peIndex));
	peArrow = obj->AddComponent<PE_Status>(new PE_Status(nullptr, obj));
	peArrow->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("directionalTarget"));
	peArrow->SetHavePlayer(false);
	peArrow->SetFixedRotation(rotate);
	peArrow->SetCanRotate(false);
	peArrow->SetPSCanDie(false);
	peArrow->SetMinMaxTextureSize({ 0, 0 }, {1.0f, dis });
	peArrow->SetOffset({ 0, 0.2f, 0 });
	peArrow->SetPlayerIdChangeColor(false);
	peArrow->SetMaxParticle(1);
	peArrow->SetTransparent(0.7f);
	peArrow->SetActive(false);
	peArrow->InitializeParticle(LeRenderer::device);
	obj->SetPosition(arrowPos);	
}

LeComp_MOB * DirectionalTargeting::GetTarget()
{
	defaultTarget = true;

	if (target)
	{
		return target;
	}
	return nullptr;
}

void DirectionalTargeting::Update(float dt)
{
	if (active)
	{	
		vector<EnemyAI*> targets;
		vector<float> targetAngle; //compare to Stick
		float3 playerPos = player->owner->GetPosition();

		DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), maxDis, targets);
		LeComp_MOB* tempTarget = nullptr;

#pragma region //Cam's rotation effect Dir
		float3 camPos = camera->owner->GetPosition();
		camPos.y = playerPos.y; //remove X rotate of dir
		float3 camDir = (playerPos - camPos).Normalize();
		float camAngle = atan2(camDir.x, camDir.z);

		float3 tempDir = ((forward * stickDir.direction.y) + (right * stickDir.direction.x)).Normalize();
		float3 dir;	//after rotate by Camera
		dir.z = tempDir.z * cos(camAngle) - tempDir.x * sin(camAngle);
		dir.x = tempDir.z * sin(camAngle) + tempDir.x * cos(camAngle);
		dir.y = tempDir.y;
		dir.Normalize();
#pragma endregion

		float angleStick = XMConvertToDegrees(atan2(stickDir.direction.x, stickDir.direction.y));

		if (angleStick != 0)
		{

			float angleE;
			float3 dirE;
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				dirE = targets[i]->owner->GetPosition() - playerPos;
				angleE = XMConvertToDegrees(acos(LeMath::Dot(dir, dirE) / (dir.GetLength() * dirE.GetLength())));
				
				targetAngle.push_back(angleE);
			}
			if (targetAngle.size() > 0)
			{
				int minIndex = MinTargetAngle(targetAngle);
				tempTarget = targets[minIndex];
			}
		}

		LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
		if (battleMage && battleMage->GetTarget() && defaultTarget || tempTarget && !defaultTarget)
		{
			if (tempTarget && !defaultTarget)
			{
				target = tempTarget;
			}
			else if(battleMage->GetTarget() && defaultTarget)
			{
				target = battleMage->GetTarget();
				defaultTarget = false;
			}

			arrowPos = (player->owner->GetPosition() + target->owner->GetPosition()) / 2;
			dir = target->owner->GetPosition() - playerPos;
			dis = target->owner->GetPosition().GetDistance(playerPos);
			rotate = 180 - XMConvertToDegrees(atan2(dir.x, dir.z));

			obj->SetPosition(arrowPos);
			peArrow->SetFixedRotation(rotate);
			peArrow->SetMinMaxTextureSize({ 0, 0 }, { 1.0f, dis });
			peArrow->SetActive(true);
		}
	}
	else
	{
		target = nullptr;
		peArrow->SetActive(false);
	}

}
