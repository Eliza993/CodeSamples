//#include "pch.h"
#include "stdafx.h"
#include "PE_Ribbon.h"

void PE_Ribbon::UpdateUV()
{
	int count = 0;
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		float v = 1 - (float)(count) / (float)particleList.size();
		if (count%2 == 0) //even
		{
			//iter._Ptr->_Myval->uv = { 0, v };
			iter._Ptr->_Myval->uv = { v, 0 };
		}
		else
		{
			//iter._Ptr->_Myval->uv = { 1, v };
			iter._Ptr->_Myval->uv = { v, 1 };
		}
		count++;
	}
}



void PE_Ribbon::WideningRibbon(float3 * value1, float3 * value2)
{
	float3 temp1;
	float3 temp2;
	float3 dir = *value2 - *value1;
	temp1 = *value1 + dir;
	temp2 = *value2 - dir;

	*value1 = temp1;
	*value2 = temp2;
}

bool PE_Ribbon::InitilizeEmitter()
{
	transparent = 0.5f;
	rate = 0.01f; //lower get smoother curve
	minDis = 0;
	maxDis = 0;
	direction = -1;
	spreadAngle = 0;
	speed = 0.05f;
	randSpeed = 0;
	curTime = 0;
	minSize = { 0, 0};
	maxSize = { 0, 0};
	maxAliveTime = 0.2f;
	totalAlivePS = 0;
	maxAlivePS = 1.0f;
	return true;
}

void PE_Ribbon::Emit(float time)
{
	curTime += time;
	if (curTime >= rate)
	{
		if (particleList.size() < (unsigned int)GetMaxParticle())
		{
			float3 vel = { 0, 0, 0 };
			float2 size = {0, 0};
			float4 c = {1, 1, 1, transparent};

			Particle *p1 = nullptr;
			Particle *p2 = nullptr;

#if 0
			//way1: no widening
			p1 = new Particle{ *pos1, c, vel,size,{ 0, 0 }, 0, 0, true };
			p2 = new Particle{ *pos2, c, vel,size,{ 1, 0 }, 0, 0, true };
#else
			//way2: use widening
			float3 value1 = *pos1;
			float3 value2 = *pos2;
			WideningRibbon(&value1, &value2);

			p1 = new Particle{ value1, c, vel,size,{ 0, 0 }, 0, 0, maxAliveTime, true };
			p2 = new Particle{ value2, c, vel,size,{ 1, 0 }, 0, 0, maxAliveTime, true };
#endif // 0

			particleList.push_back(p1);
			particleList.push_back(p2);
			UpdateUV();
		}
		curTime = 0;
	}
}

//PE_Ribbon::PE_Ribbon(SimpleVertex *_v1, SimpleVertex *_v2)
//{
//	v1 = _v1;
//	v2 = _v2;
//}

//PE_Ribbon::~PE_Ribbon()
//{
//	//delete pos1;
//	//delete pos2;
//}
