#include "stdafx.h"
#include "PE_Shot.h"
#include "LeMath.h"
#include "Collider.h"
#include "LeComp_Hitbox.h"
#include "PE_Status.h"
#include "./Enemy.h"
#include "LeComp_Familiar.h"

bool PE_Shot::InitilizeEmitter()
{
	transparent = 1.0f;
	rate = 1.0f;
	minDis = 0;
	maxDis = 0;
	direction = 0;
	spreadAngle = 0;
	speed = 10.0f;
	randSpeed = 0.0f;
	curTime = 0;
	minSize = { 0, 0 };
	maxSize = { 0, 0 };
	maxAliveTime = 3.0f;
	totalAlivePS = 0;
	maxAlivePS = 5.0f;
	maxSize = {1.5f, 1.5f};
	offset = {0, 0, 0};

	return true;
}

void PE_Shot::InitilizeParticleSystem()
{
	float3 vel;
	vel.x = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
	vel.y = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
	vel.z = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;

	size = maxSize;
	float4 c = { fixedColor, transparent};

	//for arrow/bullet, use Rotate to make arrow point to Target

	//way1: does NOT work because Dot only work -90 to 90 -> half will be wrong
	//float2 vec1 = {1, 1};
	//float2 vec2 =  {dir.x, dir.z}; //use x, z, ignore Y
	//float dot = vec1.GetDot(vec2);
	//float rotate = XMConvertToDegrees(acos(dot / (vec1.GetLength()*vec2.GetLength())));

	//way2:
	float rotate = 180 - XMConvertToDegrees(atan2(dir.x, dir.z));
	Particle *p1 = new Particle{ startPos + offset, c, vel,size,{ 0, 0 }, rotate, 0, maxAliveTime, true };
	particleList.push_back(p1);	

	if (useHitbox)
	{
		Object *hitboxObj = owner->owningGameState->AddObject(owner->GetName() + "ShotHitbox");
		hitboxObj->SetParent(shoter->owner);
		LeComp_Hitbox *hitbox = hitboxObj->AddComponent<LeComp_Hitbox>();
		hitbox->hitSound = L"silence.wav"; //default sound
		hitboxCol = hitboxObj->AddComponent<SphereCollider>();
		hitboxObj->SetPosition({ 0, 0, 0 });

		//NOTE: change tag if arrow belong to Enemy
		LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(shoter);
		if (battleMage)
		{
			((SphereCollider*)hitboxCol)->Init(size.x / 4.0f);
			hitbox->tagsToAvoid.push_back("player");

			//this will Cal damage
			if (abilityName != AbilityName::Null)
			{
				hitbox->owner->tag = "AbilityBox";
			}

			if (abilityName == AbilityName::Entangle
				|| abilityName == AbilityName::Poison_Shot
				|| abilityName == AbilityName::Multishot
				)
			{
				hitbox->hitSound = L"arrow.wav";
			}
			else if (abilityName == AbilityName::Fireball)
			{
				hitbox->hitSound = L"largeFireball.wav";
			}
			else if(abilityName == AbilityName::Null) //default atk
			{
				if (battleMage->GetClass() == PlayerClass::Ranger)
				{
					hitbox->hitSound = L"arrow.wav";
				}
				else if(battleMage->GetClass() == PlayerClass::Mage)
				{
					//put hit sound for mage default atk here
					//hitbox->hitSound = L"magical2.wav";
				}
			}
		}
		else
		{
			LeComp_Familiar* fam = (LeComp_Familiar*)shoter;
			hitbox->tagsToAvoid.push_back("player");
			hitbox->hitSound = L"largeFireball.wav";
			hitbox->damage = fam->GetPlayer()->level * 2.0f;
			((SphereCollider*)hitboxCol)->Init(size.x*4.0f);
		}

		hitboxCol->isTrigger = true;
		hitboxCol->SetActive(true);
	}
}

void PE_Shot::UpdateParticle(float time)
{
	Particle *temp = particleList.begin()._Ptr->_Myval;	
	if (direction == 0 || direction == 1 || direction == 4 || direction == 5)
	{
		//block Y

		//curPos.x += dir.x*speed*time;
		//curPos.z += dir.z*speed*time;

		curPos.x += dir.x*temp->velocity.x*time;
		curPos.z += dir.z*temp->velocity.z*time;
	}
	else if(direction == 2 || direction == 3)
	{
		curPos.y += dir.y*temp->velocity.y*time;
	}
	else
	{
		curPos += dir*temp->velocity.x*time;
	}

	temp->pos = curPos + offset;
	if (useHitbox)
	{
		hitboxCol->owner->SetPosition(temp->pos - shoter->owner->GetPosition());
	}
	curDis = curPos.GetDistance(startPos);
}

bool PE_Shot::Frame(float time)//, ID3D11DeviceContext * context)
{
	//too old or move too far, sometime dont have maxDis
	totalAlivePS += time;

	//nearly end, give it a little time to check collision
	if (totalAlivePS >= maxAlivePS*0.8f || (maxDis && curDis >= maxDis*0.8f))
	{
		if (endEffect)
		{
			//this will display Explosion effect
			float effectRad = 5.0f;

			if (useHitbox)
			{
				((SphereCollider*)hitboxCol)->SetRadius(effectRad);

				Object *obj = shoter->owner->owningGameState->AddObject(shoter->owner->GetName() + "ExplosionEndEffect");
				PE_Status *explosion = obj->AddComponent<PE_Status>(new PE_Status(nullptr, obj));
				explosion->SetMaterial(shoter->owner->owningGameState->Renderer->GetMaterial("explosion"));
				explosion->SetHavePlayer(false);
				explosion->SetPlayerIdChangeColor(false);
				explosion->SetPSCanDie(true);
				explosion->SetAliveTimePS(1.0f);
				explosion->SetOffset({ 0, 0.2f, 0 });
				explosion->SetMinMaxTextureSize({ 0, 0 }, { effectRad*2.0f, effectRad*2.0f });
				explosion->SetMaxParticle(1);
				explosion->InitializeParticle(LeRenderer::device);
				obj->SetPosition(endPos);
			}

			endEffect = false;
		}

	}

	if (totalAlivePS >= maxAlivePS || (maxDis && curDis >= maxDis))
	{
		isActive = false;

		if (useHitbox)
		{
			hitboxCol->Destroy();
		}

		owner->SetIsAlive(false);
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

//void PE_Shot::OnCollisionEnter(Collider* owners, Collider* colliding)
//{
//	owner->SetIsAlive(false);
//	//isDead = true;
//	//DGameState->RemoveObjectByName(owner->GetName());
//}