#include "stdafx.h"
#include "PE_WarriorBuff.h"

bool PE_WarriorBuff::InitilizeEmitter() 
{
	//maxParticle = 1;
	transparent = 0.8f;
	rate = 1.0f;
	minDis = 0;
	maxDis = 0;
	direction = -1;
	spreadAngle = 1;
	speed = 5.0f;
	randSpeed = 0.5f;
	curTime = 0;
	minSize = { 1, 1 };
	maxSize = { 1, 1 };
	maxAliveTime = 0.0f;
	totalAlivePS = 0;
	maxAlivePS = 3.0f;
	fixedColor = {1, 1, 1};
	randomColor = false;
	psCanDie = true;
	return true;
}

void PE_WarriorBuff::InitilizeParticleSystem()
{
	//float3 pos = player->owner->GetGlobalPosition();
	float3 pos = {0, 2.0, 0};
	float3 vel;
	vel.x = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
	vel.y = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
	vel.z = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;

	float4 c = { fixedColor, transparent };
	float2 size = maxSize;

	float2 uv = { 0, 0 };
	float rotate = 0;
	Particle *p = new Particle{ pos, c, vel, size, uv, rotate, 0, maxAliveTime, true };

	particleList.push_back(p);
}

void PE_WarriorBuff::UpdateParticle(float time)
{
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		Particle *temp = iter._Ptr->_Myval;
		temp->age += time;
		if (temp->age > temp->aliveTime)
		{
			temp->isAlive = false;
		}
		temp->pos.y += temp->velocity.y*time;
		if (temp->size.x > maxSize.x*2)
		{
			temp->size.x += 1.0f*time;
		}
		if (temp->size.y > maxSize.y*2)
		{
			temp->size.y += 1.0f*time;
		}
	}
}

bool PE_WarriorBuff::Frame(float time)
{
	totalAlivePS += time;
	if (totalAlivePS >= maxAlivePS)
	{
		owner->SetIsAlive(false);
		//DGameState->RemoveObjectByName(owner->GetName());
		return false;
	}
	//RemoveDeadParticles();

	Emit(time);
	UpdateParticle(time);

	if (UpdateBuffer())// context))
	{
		return true;
	}
	else
	{
		return false;
	}	
}
