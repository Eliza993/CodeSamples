#pragma once
#include "ParticleSystem.h"

class PE_Ribbon : public ParticleSystem
{
	//update Pos from 2 vertices
	//SimpleVertex *v1;
	//SimpleVertex *v2;

	float3 *pos1;
	float3 *pos2;

	//update UV every time have 2 new pos, it stretch UV out, makes texture looks longer
	void UpdateUV();
	//make ribbon wider, easier to see
	void WideningRibbon(float3 *value1, float3 *value2);
	virtual bool InitilizeEmitter() override;
	virtual void Emit(float time) override;

public:
	//PE_Ribbon() : ParticleSystem() {};
	//PE_Ribbon(SimpleVertex *_v1, SimpleVertex *_v2);

	//can construct by both Vertex or Joint
	PE_Ribbon(Object *owner, SimpleVertex *_v1, SimpleVertex *_v2):ParticleSystem(owner)
	{
		particleVertexType = EParticleVertexType::eTriangle;
		pos1 = &(_v1->position);
		pos2 = &(_v2->position);
	}
	PE_Ribbon(Object *owner, JointFloat4x4 *_v1, JointFloat4x4 *_v2) :ParticleSystem(owner)
	{
		particleVertexType = EParticleVertexType::eTriangle;
		pos1 = _v1->transform.values[3].AsFloat3();
		pos2 = _v2->transform.values[3].AsFloat3();
	}

	PE_Ribbon(Object *owner, float4x4 *_v1, float4x4 *_v2) :ParticleSystem(owner)
	{
		particleVertexType = EParticleVertexType::eTriangle;
		pos1 = _v1->values[3].AsFloat3();
		pos2 = _v2->values[3].AsFloat3();
	}

	//PE_Ribbon(const PE_Ribbon &ribbon){};
	//virtual ~PE_Ribbon();
};