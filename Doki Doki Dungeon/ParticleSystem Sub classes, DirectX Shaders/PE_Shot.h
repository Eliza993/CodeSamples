#pragma once
#include "ParticleSystem.h"
#include "LeComp_MOB.h"
#include "GameDataTypes.h"

class Collider;

//Bullet/arrow move from Start to End
class PE_Shot : public ParticleSystem
{
	float3 startPos;
	float3 endPos;
	float3 curPos;
	float3 dir;

	float2 size;
	AbilityName abilityName;

	LeComp_MOB *shoter;
	Collider *hitboxCol = nullptr;
	bool useHitbox;
	bool endEffect;

	bool isDead = false;
	float curDis;
	float maxDis; //arrow will be destroy after this far
	virtual bool InitilizeEmitter() override;
	virtual void InitilizeParticleSystem() override;

	virtual void UpdateParticle(float time) override;

public:
	//NOTE: may need to set Start/End pos higher because it may in the same pos with Floor -> flickering image
	//can create reuseable Hitbox or create 1 time used hitbox in here
	PE_Shot(Object *owner, float3 _startPos, float3 _endPos, AbilityName _abilityName = AbilityName::Null, bool _hitbox = false, LeComp_MOB *_shoter = nullptr, bool _endEffect = false) :ParticleSystem(owner)
	//PE_Shot(Object *owner, float3 _startPos, float3 _endPos, bool _hitbox = false, LeComp_MOB *_shoter = nullptr, bool _endEffect = false) :ParticleSystem(owner)
	{
		startPos = _startPos;
		endPos = _endPos;
		curPos = _startPos;
		dir = (_endPos - _startPos).GetNormalized();

		curDis = 0.0f;
		maxDis = 15.0f;

		abilityName = _abilityName;
		useHitbox = _hitbox;
		shoter = _shoter;
		endEffect = _endEffect;
	}
	//virtual ~PE_Shot() {};
	//~PE_Shot() 
	//{
	//	delete hitboxCol;
	//}

	virtual bool Frame(float time) override;
	float3 GetPosition() const { return curPos; }
	float2 GetSize() const { return size; }
	float3 GetPosition() { return (curPos + offset); }
	void SetMaxDistance(float _distance) { maxDis = _distance; }
	//virtual void OnCollisionEnter(Collider* owners, Collider* colliding);
};