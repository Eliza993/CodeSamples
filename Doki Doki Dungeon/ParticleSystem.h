#pragma once
#include "DataTypes.h"
#include <d3d11.h>
#include <wrl.h>
#include <list>
#include <iostream>
#include "./QueryPerformanceTimer.h"
#include "./LeRenderer.h"

using Microsoft::WRL::ComPtr;
using namespace std;

enum class EParticleVertexType {
	ePoint,
	eLine,
	eTriangle
};

class ParticleSystem: public ILeComponent
{
public:
	struct Particle
	{
		float3 pos;
		float4 color;
		float3 velocity;
		float2 size;
		float2 uv;
		float rotateZ; //in radian
		float age;
		float aliveTime;
		bool isAlive;
	};

private:
	int maxParticle = 100; //do NOT set this at lower number in InitilizeEmitter() in child class, will lead to Error with VertexBuffer, use SetMaxParticle() instead

protected:
	EParticleVertexType particleVertexType = EParticleVertexType::ePoint;

	int direction; //0 = x, 1 = -x, 2 = y, 3 = -y, 4 = z, 5 = -z, -1 = NONE
	float transparent;
	//check when to emit
	float curTime;
	float rate; //particle per second (emision speed)
	float minDis;
	float maxDis;
	float3 offset; //how far away is PS from owner?
	float spreadAngle;
	float speed; //moving speed
	float randSpeed;
	//max time of each particle
	float maxAliveTime;
	//time of Particle System
	float maxAlivePS;
	float3 fixedColor;
	float rotateSpeed;
	float fixedRotate;
	bool canRotate;
	bool randomColor;
	bool scalable;
	bool psCanDie;
	
	float totalAlivePS; //age of PS
	bool isActive = false;

	Material *material;
	float2 minSize;
	float2 maxSize;

	list<Particle*> particleList;

	ParticleVertex vertices[100];
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> instanceBuffer;
	HRESULT hr;

	virtual void InitilizeParticleSystem() {}; //use when don't want to Emit for need something before Emit
	virtual void UpdateParticle(float time); //update time for all Particle
	virtual void Emit(float time); //create 1 Particle on Rate at random element

	bool InitilizeBuffer(ID3D11Device *device);
	bool UpdateBuffer();
	void RemoveDeadParticles();
	void ShutdownParticleSystem();
	void ShutdownBuffer();

public:
	ParticleSystem(Object *owner);
	virtual ~ParticleSystem();

	virtual bool InitilizeEmitter(); //default values
	bool InitializeParticle(ID3D11Device *device); //NOTE: call this after Set values
	void ShutDown();
	//run per frame
	virtual bool Frame(float time);

	int GetMaxParticle() const { return maxParticle; }

	bool GetActive() { return isActive; }
	void SetActive(bool _active) { isActive = _active; }

	void SetDirection(int _direction); //0 = x, 1 = y, 2 = z
	void SetTransparent(float _transparent);
	void SetMaxParticle(int _maxParticle);
	void SetEmissionRate(float _rate);
	void SetMinMaxDistance(float _min, float _max);
	void SetSpeadAngle(float _spreadAngle);
	void SetMovingSpeed(float _speed); 
	void SetRandomSpeed(float _randSpeed);
	void SetOffset(float3 _offset);
	void SetAliveTimeEachParticle(float _maxAliveTime);
	void SetAliveTimePS(float _maxAlivePS);
	void SetCanRotate(bool _canRotate);
	void SetScalable(bool _scalable);
	void SetRandomColor(bool _randomColor);
	void SetFixedColor(float3 _fixedColor);
	void SetRotateSpeed(float _rotate);
	void SetFixedRotation(float _fixedRotation);
	void SetPSCanDie(bool _psCanDie);
	void SetMinMaxTextureSize(float2 _min, float2 _max);

	ID3D11Buffer * const GetVertexBuffer() const { return vertexBuffer.Get(); }
	ID3D11Buffer * const GetInstanceBuffer() const { return instanceBuffer.Get(); }
	int GetVertexCount() const { return (int)particleList.size(); }
	EParticleVertexType GetParticleVertexType() const { return particleVertexType; }

	Material * GetMaterial()const { return material; };
	void SetMaterial(Material *m) { material = m; };

	//NOTE: use deltaTime to make sure Vel is consistent bet. diff. framerate
	//ie: effect in Debug mode moves slower than in Release mode
	//deltaTime is about 0.003

	// Inherited via ILeComponent
	virtual void Init() override;
	virtual void Register() override;
	virtual void Update(float dt) override;
	virtual void Destroy() override;
};


