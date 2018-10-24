#pragma once
#include "./ParticleSystem.h"
class PE_WarriorBuff:public ParticleSystem
{
	virtual bool InitilizeEmitter() override;
	virtual void InitilizeParticleSystem() override;
	virtual void UpdateParticle(float time) override;

public:
	PE_WarriorBuff(Object *owner):ParticleSystem(owner){};
	//virtual ~PE_WarriorBuff() {};

	bool Frame(float time) ;//, ID3D11DeviceContext *context);
};