#pragma once
#include "ParticleSystem.h"

class PE_Respawn : public ParticleSystem
{
	virtual void UpdateParticle(float time) override;
	
public:
	PE_Respawn(Object *owner) :ParticleSystem(owner) {};
	//virtual ~PE_Respawn() {};
};