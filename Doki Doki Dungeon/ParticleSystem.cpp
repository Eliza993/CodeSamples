//#include "pch.h"
#include "stdafx.h"
#include "ParticleSystem.h"

//sort by Z value before Draw
bool SortParticle(const ParticleSystem::Particle* p1, const ParticleSystem::Particle* p2)
{
	return (p1->pos.z < p2->pos.z);
}

bool IsDead(const ParticleSystem::Particle* p)
{
	return (p->isAlive) ? false : true;
}

bool ParticleSystem::InitilizeEmitter()
{
	maxParticle = 100;
	transparent = 1.0f; //default = solid;
	rate = 0.1f;
	minDis = 0.5f;
	maxDis = 2.0f;
	direction = -1;
	spreadAngle = 0;
	speed = 2.0f;
	randSpeed = 0.5f;
	offset = { 0, 0, 0 };
	curTime = 0;
	minSize = { 0.3f, 0.3f };
	maxSize = { 0.4f, 0.4f };
	maxAliveTime = 1.0f;
	totalAlivePS = 0;
	maxAlivePS = 3.0f;
	fixedColor = {1, 1, 1};
	rotateSpeed = 0.0f;
	fixedRotate = 0;
	canRotate = true;
	scalable = false;
	randomColor = true;
	psCanDie = true;

	return true;
}

void ParticleSystem::ShutdownParticleSystem()
{
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		delete *iter;
	}
	particleList.clear();
}

bool ParticleSystem::InitilizeBuffer(ID3D11Device *device)
{
	memset(vertices, 0, sizeof(ParticleVertex)*maxParticle);
	int count = 0;
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		vertices[count].position = iter._Ptr->_Myval->pos;
		vertices[count].color = iter._Ptr->_Myval->color;
		vertices[count].size = iter._Ptr->_Myval->size;
		vertices[count].uv = iter._Ptr->_Myval->uv;
		vertices[count].rotateZ = iter._Ptr->_Myval->rotateZ;
		count++;
	}

	CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(sizeof(ParticleVertex) * maxParticle, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&desc, &data, &vertexBuffer);

	if (hr != S_OK)
	{
		return false;
	}

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = sizeof(float4x4);
	HRESULT result = DCurrentRenderer->device->CreateBuffer(&bufferDesc, nullptr, &instanceBuffer);

	return true;
}

bool ParticleSystem::UpdateBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	memset(vertices, 0, sizeof(ParticleVertex)*maxParticle);
	int count = 0;
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		vertices[count].position = iter._Ptr->_Myval->pos;
		vertices[count].color = iter._Ptr->_Myval->color;
		vertices[count].size = iter._Ptr->_Myval->size;
		vertices[count].uv = iter._Ptr->_Myval->uv;
		vertices[count].rotateZ = iter._Ptr->_Myval->rotateZ;
		count++;
	}

	DCurrentRenderer->UpdateBuffer(vertexBuffer.GetAddressOf(), vertices, sizeof(ParticleVertex)*maxParticle);
	DCurrentRenderer->UpdateBuffer(instanceBuffer.GetAddressOf(), &owner->transforms[0].transformMatrix, sizeof(float4x4));

	return true;
}

void ParticleSystem::ShutdownBuffer()
{
	vertexBuffer.Reset();
	instanceBuffer.Reset();
}

void ParticleSystem::RemoveDeadParticles()
{
	int count = 0;
	auto iter = particleList.begin();
	while (iter != particleList.end())
	{
		count++;
		if (iter._Ptr->_Myval->isAlive == false)
		{
			delete iter._Ptr->_Myval;
			iter = particleList.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void ParticleSystem::UpdateParticle(float time)
{
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		Particle *temp = iter._Ptr->_Myval;
		temp->age += time;
		if (temp->age > temp->aliveTime)
		{
			temp->isAlive = false;
		}
	}
}

ParticleSystem::ParticleSystem(Object * _owner) : ILeComponent(_owner)
{
}

ParticleSystem::~ParticleSystem()
{
	ShutDown();
}

bool ParticleSystem::InitializeParticle(ID3D11Device * device)
{
	InitilizeParticleSystem();

	if (!InitilizeBuffer(device))
	{
		return false;
	}

	return true;
}

void ParticleSystem::ShutDown()
{
	ShutdownBuffer();
	ShutdownParticleSystem();
}

bool ParticleSystem::Frame(float time)
{
	totalAlivePS += time;
	if (totalAlivePS >= maxAlivePS && psCanDie)
	{
		owner->SetIsAlive(false);	
		return false;
	}

	RemoveDeadParticles();
	Emit(time);
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

void ParticleSystem::SetDirection(int _direction)
{
	direction = _direction;
}

void ParticleSystem::SetTransparent(float _transparent)
{
	transparent = _transparent;
}

void ParticleSystem::SetMaxParticle(int _maxParticle)
{
	maxParticle = _maxParticle;
}

void ParticleSystem::SetEmissionRate(float _rate)
{
	rate = _rate;
}

void ParticleSystem::SetMinMaxDistance(float _min, float _max)
{
	minDis = _min;
	maxDis = _max;
}

void ParticleSystem::SetSpeadAngle(float _spreadAngle)
{
	spreadAngle = _spreadAngle;
}

void ParticleSystem::SetMovingSpeed(float _speed)
{
	speed = _speed;
}

void ParticleSystem::SetRandomSpeed(float _randSpeed)
{
	randSpeed = _randSpeed;
}

void ParticleSystem::SetOffset(float3 _offset)
{
	offset = _offset;
}

void ParticleSystem::SetAliveTimeEachParticle(float _maxAliveTime)
{
	maxAliveTime = _maxAliveTime;
}

void ParticleSystem::SetAliveTimePS(float _maxAlivePS)
{
	maxAlivePS = _maxAlivePS;
}

void ParticleSystem::SetCanRotate(bool _canRotate)
{
	canRotate = _canRotate;
}

void ParticleSystem::SetScalable(bool _scalable)
{
	scalable = _scalable;
}

void ParticleSystem::SetRandomColor(bool _randomColor)
{
	randomColor = _randomColor;
}

void ParticleSystem::SetFixedColor(float3 _fixedColor)
{
	fixedColor = _fixedColor;
}

void ParticleSystem::SetRotateSpeed(float _rotate)
{
	rotateSpeed = _rotate;
}

void ParticleSystem::SetFixedRotation(float _fixedRotation)
{
	fixedRotate = _fixedRotation;
}

void ParticleSystem::SetPSCanDie(bool _psCanDie)
{
	psCanDie = _psCanDie;
}

void ParticleSystem::SetMinMaxTextureSize(float2 _min, float2 _max)
{
	minSize = _min;
	maxSize = _max;
}

void ParticleSystem::Init()
{
	isActive = true;

	InitilizeEmitter();
}

void ParticleSystem::Register()
{
	owner->owningGameState->Renderer->AddParticleSystem(this);
}

void ParticleSystem::Update(float dt)
{
	Frame(dt);
}

void ParticleSystem::Destroy()
{
	DCurrentRenderer->RemoveParticleSystem(this);
}

void ParticleSystem::Emit(float time)
{
	curTime += time;
	if (curTime >= rate)
	{
		if (particleList.size() < (unsigned int)maxParticle)
		{
			float3 pos;
			pos.x = (float)(rand() / (float)RAND_MAX) * (maxDis - minDis);
			//NOTE: move up a little to see easier/faster, can also increase speed to make move up faster
			pos.y = 0;
			pos.z = (float)(rand() / (float)RAND_MAX) * (maxDis - minDis);
			pos += offset;

			float3 vel;
			vel.x = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
			vel.y = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;
			vel.z = (float)(rand() / (float)RAND_MAX) * randSpeed + speed;

			float4 c;
			if (randomColor)
			{
				c.x = (float)(rand() / (float)(RAND_MAX));
				c.y = (float)(rand() / (float)(RAND_MAX));
				c.z = (float)(rand() / (float)(RAND_MAX));
				c.w = transparent;
			}
			else
			{
				c = { fixedColor, transparent};
			}

			float2 size;
			size.x = (float)(rand() / (float)RAND_MAX) * (maxSize.x - minSize.x) + minSize.x;
			size.y = (float)(rand() / (float)RAND_MAX) * (maxSize.y - minSize.y) + minSize.y;

			float2 uv = { 0, 0 };
			float rotate = 0;
			Particle *p = new Particle{ pos, c, vel, size, uv, rotate, 0, maxAliveTime, true };

			particleList.push_back(p);
			particleList.sort(SortParticle);
		}
		curTime = 0;
	}
}
