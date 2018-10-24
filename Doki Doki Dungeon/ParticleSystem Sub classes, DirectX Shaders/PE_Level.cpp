#include "stdafx.h"
#include "PE_Level.h"

void PE_Level::InitilizeParticleSystem()
{
	complete = false;

	float3 pos1 = { 0, 0.5f, 0 };
	float3 pos2 = { 0, 0.8f, 0 };

	float3 vel = { 0, 0, 0 };
	float4 c = { 1, 1, 1, transparent };
	
	float2 size1 = { 0.6f, 0.6f };
	float2 size2 = { 0.3f, 0.3f };

	float2 uv = { 0, 0 };
	float rotate = 0;

	Particle *p1 = new Particle{ pos1, c, vel, size1, uv, rotate, 0, 1.0f, true };
	Particle *p2 = new Particle{ pos2, c, vel, size2, uv, rotate, 0, 1.0f, true };

	particleList.push_back(p1);
	particleList.push_back(p2);
}

void PE_Level::UpdateParticle(float time)
{
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		Particle *temp = iter._Ptr->_Myval;
		temp->age += time;
		if (temp->age > temp->aliveTime)
		{
			temp->isAlive = false;
		}

		if (scalable)
		{
			temp->size = temp->size + float2{ 1.0f, 1.0f }*time;
		}

		if (canRotate)
		{
			temp->rotateZ += 50.0f*time;
			if (temp->rotateZ >= 360)
			{
				temp->rotateZ = 0;
			}
		}
	}
}

void PE_Level::Emit(float time)
{
	if (particleList.size() == 0 && !complete)
	{
		complete = true;

		float3 pos3 = { 0, 1.5f, 0 };
		float3 vel = { 0, 0, 0 };
		float4 c = { 1, 1, 1, transparent };
		float2 size3 = { 1.0f, 1.0f };
		float2 uv = { 0, 0 };
		float rotate = 0;
		Particle *p3 = new Particle{ pos3, c, vel, size3, uv, rotate, 0, 1.0f, true };
		particleList.push_back(p3);
	}
}
