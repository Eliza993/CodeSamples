#pragma once
#include "ParticleSystem.h"
#include "LeMath.h"
#include "LeComp_MOB.h"

class Collider;

//no need to track position, lighting happen quick
//make it appear from start to end, not all at a time ->send to VertexBuffer slowly
class PE_Lightning : public ParticleSystem
{
	float3 startPos;
	float3 endPos;
	float3 dir;
	float3 widthDir; //this perpendicular with dir vector, is used to move point
	float distance;
	
	vector<float> fragments;
	int numFragment;
	float fragmentLength;
	float minU, maxU; //only expand middle
	float maxWidth; //how far Particle can go far from original line, smaller value -> straighter line
	
	LeComp_MOB *shoter;
	Collider *hitboxCol = nullptr;
	bool useHitbox = false;

	Particle *startP;
	Particle *endP;

	virtual void InitilizeParticleSystem() override;
	//random move Particle Up/Down
	virtual void UpdateParticle(float time) override;

public:
	PE_Lightning(Object *owner, float3 _startPos, float3 _endPos, bool _useHitbox = false, LeComp_MOB *_shoter = nullptr) :ParticleSystem(owner)
	{
		particleVertexType = EParticleVertexType::eLine;
		srand((unsigned int)time(0));

		startPos = _startPos;
		endPos = _endPos;

		useHitbox = _useHitbox;
		shoter = _shoter;

		//raise Pos Up to see easier
		startPos.y += 1.0f;
		endPos.y += 1.0f;

		dir = (_endPos - _startPos).GetNormalized();
		//vector perpendicular with dir and Up vector
		widthDir = LeMath::Cross(dir, float3{ 0, 1, 0 }).GetNormalized();
		distance = startPos.GetDistance(endPos);		
		numFragment = (int)distance * 2; //to increase Num Fragment when dis. increase
		fragmentLength = 0.45f;
		minU = 0.2f;
		maxU = 0.8f;
		maxWidth = 1.0f;
	}
	//virtual ~PE_Lightning() {};
	//~PE_Lightning() 
	//{
	//	delete hitboxCol;
	//}

	virtual bool InitilizeEmitter() override;
	virtual bool Frame(float time) override;

	void SetNumSegment(int _numFragment);
	void SetFragmentLength(float _fragmentLength);
	void SetMaxWidth(float _maxWidth);
};