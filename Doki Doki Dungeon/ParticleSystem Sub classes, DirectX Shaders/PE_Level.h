#pragma once
#include "ParticleSystem.h"

class PE_Level:public ParticleSystem
{
	virtual void InitilizeParticleSystem() override;
	virtual void UpdateParticle(float time) override;
	virtual void Emit(float time) override;

	bool complete;
public:
	PE_Level(Object* owner) :ParticleSystem(owner)
	{
	};
	~PE_Level() {};


};
