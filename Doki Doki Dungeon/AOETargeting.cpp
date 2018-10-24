#include "stdafx.h"
#include "AOETargeting.h"
#include "./Enemy.h"
#include "LeComp_MOB.h"
#include "LeComp_PlayerCamera.h"

AOETargeting::AOETargeting(LeComp_MOB * _player, LeComp_PlayerCamera *_camera, float _maxDis, int _peIndex)
{
	player = _player;
	camera = _camera;
	maxDis = _maxDis;
	active = false;

	forward = float3(0, 0, 1).GetNormalized();
	right = float3(1, 0, 0).GetNormalized();

	float3 playerPos = player->owner->GetPosition();
	aimPos = playerPos + player->owner->GetForward()*(float)maxDis;

	float3 tempPos = { aimPos.x, playerPos.y, aimPos.z };
	arrowPos = (playerPos + tempPos) / 2;
	dir = (tempPos - playerPos).Normalize();
	dis = tempPos.GetDistance(playerPos);

	rotate = XMConvertToDegrees(atan2(dir.x, dir.z));
	rotate = 180 - rotate;

	obj = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE AOE Arrow" + std::to_string(_peIndex));
	peArrow = obj->AddComponent<PE_Status>(new PE_Status(nullptr, obj));
	peArrow->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("directionalTarget"));
	peArrow->SetHavePlayer(false);
	peArrow->SetFixedRotation(rotate);
	peArrow->SetCanRotate(false);
	peArrow->SetPSCanDie(false);
	peArrow->SetMinMaxTextureSize({ 0, 0 }, { 1.0f, dis });
	peArrow->SetOffset({ 0, 0.3f, 0 });
	peArrow->SetPlayerIdChangeColor(false);
	peArrow->SetMaxParticle(1);
	peArrow->InitializeParticle(LeRenderer::device);
	peArrow->SetTransparent(0.7f);
	peArrow->SetActive(false);
	obj->SetPosition(arrowPos);
}

void AOETargeting::Update(float dt)
{
	if (active)
	{
		float3 playerPos = player->owner->GetPosition();

		aimPos = playerPos + player->owner->GetForward()*(float)maxDis;
		if (stickDir.magnitude > 0.0f && stickDir.magnitude <= 1.5f)
		{
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

			aimPos = playerPos + dir*(float)maxDis;
		}

		float3 tempPos = { aimPos.x, playerPos.y, aimPos.z };
		arrowPos = (playerPos + tempPos) / 2;
		dir = (tempPos - playerPos).Normalize();
		dis = tempPos.GetDistance(playerPos);
		rotate = XMConvertToDegrees(atan2(dir.x, dir.z));
		rotate = 180 - rotate;
		
		obj->SetPosition(arrowPos);
		peArrow->SetFixedRotation(rotate);
		peArrow->SetMinMaxTextureSize({ 0, 0 }, { 1.0f, dis });
		peArrow->SetActive(true);
	}
	else
	{
		peArrow->SetActive(false);
	}
}
