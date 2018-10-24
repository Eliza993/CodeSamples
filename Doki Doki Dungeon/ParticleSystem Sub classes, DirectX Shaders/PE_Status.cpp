#include "stdafx.h"
#include "PE_Status.h"
#include "LeComp_MOB.h"

bool PE_Status::InitilizeEmitter()
{
	ParticleSystem::InitilizeEmitter();
	transparent = 0.7f;
	canRotate = false;
	psCanDie = false;
	maxDis = 3.0f;
	//maxAliveTime = 20.0f;
	return true;
}

void PE_Status::InitilizeParticleSystem()
{
	float3 pos = {0, 0, 0};
	if (player && player->getAlive())
	{
		pos = player->owner->GetGlobalPosition();
		pos.y = 0;
	}
	pos += offset;
	float3 vel = {0, 0, 0};
	float4 c = {fixedColor, transparent};
	if(playerIdChangeColor && player)
	{
		if (player->owner->GetPlayerNumber() == 0)
		{
			c = { 1, 1, 1, transparent };
		}
		else if (player->owner->GetPlayerNumber() == 1)
		{
			c = { 0, 0, 1, transparent };
		}
		else if (player->owner->GetPlayerNumber() == 2)
		{
			c = { 0, 1, 0, transparent };
		}
		else if (player->owner->GetPlayerNumber() == 3)
		{
			c = { 1, 0, 0, transparent };
		}
	}
	float2 size = maxSize;

	float2 uv = { 0, 0 };
	float rotate = 0;
	Particle *p = new Particle{ pos, c, vel, size, uv, rotate, 0, maxAliveTime, true };

	particleList.push_back(p);
}

void PE_Status::UpdateParticle(float time)
{
	Particle *temp = particleList.begin()._Ptr->_Myval;
	if (player && player->getCurHealth() > 0 && player->owner)
	{
		temp->pos.x = player->owner->GetGlobalPosition().x;
		temp->pos.z = player->owner->GetGlobalPosition().z;
	}
	else if(!player)
	{
		temp->pos.y = 0;
		temp->pos += offset;
	}
	if (canRotate)
	{
		temp->rotateZ += rotateSpeed*time;
		if (temp->rotateZ >= 360)
		{
			temp->rotateZ = 0;
		}
	}
	else
	{
		temp->rotateZ = fixedRotate;
	}

	temp->size = maxSize;
	temp->color.w = transparent;
}

bool PE_Status::Frame(float time)
{
	totalAlivePS += time;
	if ((havePlayer && !player) || (havePlayer && player->getCurHealth() <= 0) || (totalAlivePS >= maxAlivePS && psCanDie))
	{
		owner->SetIsAlive(false);
		//DGameState->RemoveObjectByName(owner->GetName());
		return false;
	}

	UpdateParticle(time);
	if (UpdateBuffer())
	{
		return true;
	}
	else
	{
		return false;
	}	
}

void PE_Status::SetPlayerIdChangeColor(bool _playerIdChangeColor)
{
	playerIdChangeColor = _playerIdChangeColor;
}
