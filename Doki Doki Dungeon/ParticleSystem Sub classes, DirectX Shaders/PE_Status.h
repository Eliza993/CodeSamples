#pragma once
#include "ParticleSystem.h"

//create a Particle in player's pos, move with Player
//change color with Player's id
//use to make recognizing Player easier

class LeComp_MOB;
class PE_Status : public ParticleSystem
{
	//Object *owner;
	LeComp_MOB *player;
	//false when dont use player in constructor
	bool havePlayer;
	bool playerIdChangeColor;
	virtual void InitilizeParticleSystem() override;
	virtual void UpdateParticle(float time) override; //follow player's pos

public:
	PE_Status(LeComp_MOB *_player, Object *_owner) : ParticleSystem(_owner)
	{
		player = _player;
		havePlayer = true;
		playerIdChangeColor = true;
	}
	//virtual ~PE_Status() {};

	virtual bool InitilizeEmitter() override;
	virtual bool Frame(float time) override;
	void SetPlayerIdChangeColor(bool _playerIdChangeColor);
	void SetHavePlayer(bool _havePlayer) { havePlayer = _havePlayer; }
};