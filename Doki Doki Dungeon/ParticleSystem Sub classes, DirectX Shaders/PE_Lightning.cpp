#include "stdafx.h"
#include "PE_Lightning.h"
#include "Collider.h"
#include "LeComp_Hitbox.h"

//sort by Distance value before Draw, can use U value in UV in this case
bool SortParticleByDistance(const ParticleSystem::Particle* p1, const ParticleSystem::Particle* p2)
{
	return (p1->uv.y < p2->uv.y);
}

void PE_Lightning::InitilizeParticleSystem()
{
	if (direction == 2 || direction == 3)
	{
		widthDir = {1, 0, 0}; //can be {0, 0, 1} too, x or z
	}
	//use speed instead, it is same for every arrow/bullet
	float3 vel = { 0, 0, 0 };
	float2 size = { 1, 1};
	float4 c = { 1, 1, 1, transparent};
	float2 uv1 = { 0, 0 };
	float2 uv2 = {1, 0};
	float rotate = 0;

	//Particle *p1 = new Particle{ startPos, c, vel,size, uv1, rotate, 0, true };
	//particleList.push_back(p1);
	//Particle *p2 = new Particle{ endPos, c, vel,size, uv2, rotate, 0, true };
	//particleList.push_back(p2);

	startP = new Particle{ startPos, c, vel,size, uv1, rotate, 0, maxAliveTime, true };
	particleList.push_back(startP);


	Particle *p;
	//float randDis;
	float3 pos;
	
	//add middle pos.
	for (int i = 0; i < numFragment; i++)
	{
		////random create Pos, not get too close to 2 ends
		//randDis = ((float)rand() / (float)RAND_MAX)*(distance- 0.2f) + 0.1f;
		//pos = startPos + dir*randDis;
		////uv
		//uv1.y = (randDis / distance) * (maxU - minU) + minU;

		float randWidth = ((float)rand() / (float)RAND_MAX) * maxWidth - maxWidth / 2.0f;
		pos = particleList.back()->pos + dir*fragmentLength;
		if (i%2 == 0)
		{
			pos += widthDir*randWidth;
		}
		else
		{
			pos -= widthDir*randWidth;
		}

		uv1.x = (startPos.GetDistance(pos) / distance) * (maxU - minU) + minU;
		//uv1.y = 0.5f;

		p = new Particle{ pos, c, vel, size, uv1, rotate, 0, maxAliveTime, true };
		particleList.push_back(p);
	}

	//endPos += widthDir*10.0f;
	endP = new Particle{ endPos, c, vel,size, uv2, rotate, 0, maxAliveTime, true };
	particleList.push_back(endP);

	//particleList.sort(SortParticleByDistance);

	if (useHitbox)
	{
		Object *hitboxObj = owner->owningGameState->AddObject(owner->GetName() + "LightingHitbox");
		hitboxObj->SetParent(shoter->owner);
		LeComp_Hitbox *hitbox = hitboxObj->AddComponent<LeComp_Hitbox>();
		//this will Cal damage
		hitbox->owner->tag = "AbilityBox";
		//NOTE: change tag if arrow belong to Enemy
		hitbox->tagsToAvoid.push_back("player");
		hitbox->hitSound = L"electric1.wav";
		hitboxCol = hitboxObj->AddComponent<SphereCollider>();
		hitboxObj->SetPosition(endPos - shoter->owner->GetPosition());
		((SphereCollider*)hitboxCol)->Init(size.x / 4.0f);
		hitboxCol->isTrigger = true;
		hitboxCol->SetActive(true);
	}
}

void PE_Lightning::UpdateParticle(float time)
{
	//float randWidth;

	////for all except 1st & last
	//for (auto iter = ++particleList.begin(); iter != --particleList.end(); iter ++)
	//{
	//	Particle *temp = iter._Ptr->_Myval;
	//	//move Up or Down
	//	randWidth = ((float)rand()/(float)RAND_MAX) * maxWidth - maxWidth/2.0f;
	//	temp->pos += widthDir*randWidth;
	//	cout << randWidth << endl;
	//}

	particleList.clear();

	if (direction == 2 || direction == 3)
	{
		widthDir = { 1, 0, 0 }; //can be {0, 0, 1} too
	}

	//use speed instead, it is same for every arrow/bullet
	float3 vel = { 0, 0, 0 };
	float2 size = { 1, 1 };
	float4 c = { 1, 1, 1, transparent};
	float2 uv1 = { 0, 0 };
	float2 uv2 = { 1, 0 };
	float rotate = 0;

	//Particle *p1 = new Particle{ startPos, c, vel,size, uv1, rotate, 0, true };
	//particleList.push_back(p1);
	//Particle *p2 = new Particle{ endPos, c, vel,size, uv2, rotate, 0, true };
	//particleList.push_back(p2);

	startP = new Particle{ startPos, c, vel,size, uv1, rotate, 0, true };
	particleList.push_back(startP);


	Particle *p;
	//float randDis;
	float3 pos;

	//add middle pos.
	for (int i = 0; i < numFragment; i++)
	{
		////random create Pos, not get too close to 2 ends
		//randDis = ((float)rand() / (float)RAND_MAX)*(distance- 0.2f) + 0.1f;
		//pos = startPos + dir*randDis;
		////uv
		//uv1.y = (randDis / distance) * (maxU - minU) + minU;

		float randWidth = ((float)rand() / (float)RAND_MAX) * maxWidth - maxWidth / 2.0f;
		pos = particleList.back()->pos + dir*fragmentLength;
		if (i % 2 == 0)
		{
			pos += widthDir*randWidth;
		}
		else
		{
			pos -= widthDir*randWidth;
		}

		uv1.x = (startPos.GetDistance(pos) / distance) * (maxU - minU) + minU;
		//uv1.y = 0.5f;
		p = new Particle{ pos, c, vel, size, uv1, rotate, 0, maxAliveTime, true };
		particleList.push_back(p);
	}

	//endPos += widthDir*10.0f;
	endP = new Particle{ endPos, c, vel,size, uv2, rotate, 0, maxAliveTime, true };
	particleList.push_back(endP);
}

bool PE_Lightning::InitilizeEmitter()
{
	ParticleSystem::InitilizeEmitter();
	maxAlivePS = 0.5f;
	return true;
}

bool PE_Lightning::Frame(float time)
{
	totalAlivePS += time;
	if (totalAlivePS >= maxAlivePS)
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
	return false;
}

void PE_Lightning::SetNumSegment(int _numFragment)
{
	numFragment = _numFragment;
}

void PE_Lightning::SetFragmentLength(float _fragmentLength)
{
	fragmentLength = _fragmentLength;
}

void PE_Lightning::SetMaxWidth(float _maxWidth)
{
	maxWidth = _maxWidth;
}
