#include "Skills.h"
#include "stdafx.h"
#include "Skills.h"

//add in .cpp instead of .h to avoid circulated header
#include "LeComp_BattleMageBehavior.h"
#include "TestGameState.h" 
#include "./Enemy.h"
#include "LeComp_MOB.h"
#include "./DirectionalTargeting.h"
#include "PE_Shot.h"
#include "PE_Lightning.h"
#include "PE_Ribbon.h"
#include "./LeComp_Familiar.h"
#include "LeComp_Hitbox.h"
#include "Collider.h"
#include "LeComp_SkeletalMesh.h"
#include "LeComp_Socket.h"

int PlayerSkillNode::numPE = 0;

void PlayerSkillNode::CreateHitbox()
{
	Object *hitboxObj = player->owner->owningGameState->AddObject(player->owner->GetName() + "hitbox" + enumString[(int)ability.name]);
	hitboxObj->SetParent(player->owner);
	hitbox = hitboxObj->AddComponent<LeComp_Hitbox>();
	//this will Cal damage
	hitbox->owner->tag = "AbilityBox";
	hitbox->tagsToAvoid.push_back("player");

	wstring hitSound = L"silence.wav";
	float3 posCol = {0, 0, 0};

	mesh = player->owner->GetComponent<LeComp_SkeletalMesh>();
	//26, 27 is start/end sword
	swordTip = player->owner->AddComponent<LeComp_Socket>();
	swordTip->SetJoint(mesh, 26);
	swordEnd = player->owner->AddComponent<LeComp_Socket>();
	swordEnd->SetJoint(mesh, 26);
	swordEnd->SetOffset(float3(0, 1, 0));
	midSword = (swordTip->GetSocket()->GetPosition() + swordEnd->GetSocket()->GetPosition()) / 2.0f;

	switch (ability.name)
	{
	case AbilityName::Null:
		break;
	case AbilityName::Battle_Rally:
		break;
	case AbilityName::Cleave:
		hitSound = L"slashHit.wav";
		posCol = {0, 0, 0};
		hitboxRad = 0.5f;
		break;
	case AbilityName::Ironclad:
		break;
	case AbilityName::Two_Handed_Strike:
		hitSound = L"slashHit.wav";
		posCol = float3(0, 1, 0);
		hitboxRad = 0.7f;
		break;
	case AbilityName::Shield_Bash:
		hitSound = L"slashHit.wav";
		posCol = float3(0, 1, 0);
		hitboxRad = 0.7f;
		break;
	case AbilityName::Mana_Burst:
		break;
	case AbilityName::Restoration:
		break;
	case AbilityName::Fireball:
		break;
	case AbilityName::Lightning_Bolt:
		break;
	case AbilityName::Blizzard:
		posCol = float3(0, 1, 0);
		hitboxRad = 1.0f;
		break;
	case AbilityName::Hunters_Mark:
		break;
	case AbilityName::Entangle:
		hitSound = L"arrow.wav";
		posCol = float3(0, 1, 0) + (player->owner->GetForward() * 3.0f);
		hitboxRad = 0.5f;
		break;
	case AbilityName::Multishot:
		break;
	case AbilityName::Arrow_Volley:
		hitSound = L"arrow.wav";
		posCol = float3(0, 1, 0) + (player->owner->GetForward() * 4.0f);
		//hitboxRad = 2.5f;
		hitboxRad = 5.0f;
		break;
	case AbilityName::Poison_Shot:
		hitSound = L"arrow.wav";
		posCol = float3(0, 1, 0) + (player->owner->GetForward() * 2.5f);
		hitboxRad = 0.5f;
		break;
	case AbilityName::Shadow_Sneak:
		break;
	case AbilityName::Flurry:
		hitSound = L"slashHit.wav";
		posCol = float3(0, 1, 0);
		//hitboxRad = 0.5f;
		hitboxRad = 0.7f;
		break;
	case AbilityName::Cloak_and_Dagger:
		hitSound = L"slashHit.wav";
		posCol = float3(0, 1, 0);
		hitboxRad = 0.7f;
		break;
	case AbilityName::Bomb_trap:
		break;
	case AbilityName::Dash_Attack:
		hitSound = L"slashHit.wav";
		posCol = float3(0, 1, 0);
		hitboxRad = 0.7f;
		break;
	default:
		break;
	}

	hitbox->hitSound = hitSound;
	hitboxCol = hitboxObj->AddComponent<SphereCollider>();
	hitboxObj->SetPosition(posCol);
	((SphereCollider*)hitboxCol)->Init(hitboxRad);

	hitboxCol->isTrigger = true;
	hitboxCol->SetActive(false);
}

void PlayerSkillNode::DisableHitbox(float dt)
{
	curTime += dt;
	if (curTime >= activeTime)
	{
		hitboxCol->SetActive(false);
		
		LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
		if (battleMage)
		{
			battleMage->SetSlowdown(1.0f);
		}

		if (ability.name == AbilityName::Entangle
			|| ability.name == AbilityName::Poison_Shot
			)
		{
			hitboxCol->owner->SetPosition({ 0, 0, 0 });
		}
		active = false;
	}
}

void PlayerSkillNode::EnableHitboxAfterDelay(float time)
{
	curDelayHBTime += time;
	if (curDelayHBTime >= delayHitboxTime)
	{
		hitboxCol->SetActive(true);
	}
}

void PlayerSkillNode::UpdateHitboxPosition()
{
	float3 playerPos = player->owner->GetPosition();
	if (ability.name == AbilityName::Cleave)
	{
		midSword = (swordTip->GetSocket()->GetPosition() + swordEnd->GetSocket()->GetPosition()) / 2.0f;
		hitboxCol->owner->SetPosition(midSword - playerPos);
	}

	if (ability.name == AbilityName::Fireball
		|| ability.name == AbilityName::Entangle
		|| ability.name == AbilityName::Poison_Shot
		)
	{
		//NOTE: pos is base on Parent (player in this case), don't use Global pos, use offset
		hitboxCol->owner->SetPosition(peShot->GetPosition() - playerPos);
	}
}

void PlayerSkillNode::BuffStatsTeam(AbilityName skillName)
{
	bool immune = false;
	if (AbilityName::Mana_Burst == skillName && ability.level == maxLevel)
	{
		immune = true;
	}

	if (!AlreadyBuff(player, skillName))
	{
		cout << "Before: Player stats: " << player->getCurStr() << "-STR " << player->getCurCon() << "-CON " << player->getBaseStr() << "-RES "
			<< player->getCurIntell() << "-INT " << player->getCurWis() << "-WIS " << player->getCurDex() << "-DEX " << player->getCurAgi() << "-ALI " << endl;

		player->Buff(buff);

		cout << "After: Player stats: " << player->getCurStr() << "-STR " << player->getCurCon() << "-CON " << player->getBaseStr() << "-RES "
			<< player->getCurIntell() << "-INT " << player->getCurWis() << "-WIS " << player->getCurDex() << "-DEX " << player->getCurAgi() << "-ALI " << endl;

		if (immune)
		{
			player->SetImmune(true);
		}
	}
	else
	{
		cout << "Already have" << enumString[(int)skillName] << " buff" << endl;
	}

	//cout<<"Buff "<<buff.str<<"-STR "<<buff.con <<"-CON "<< buff.res <<"-RES "
	//	<< buff.intell <<"-INT "<<buff.wis<<"-WIS "<<buff.dex<<"-DEX "<<buff.agility<<"-ALI "
	//	<< "/nPlayer stats: " << player->getCurStr()<< "-STR " << player->getCurCon() << "-CON " << player->getBaseStr() << "-RES "
	//	<< player->getCurIntell() << "-INT " << player->getCurWis() << "-WIS " << player->getCurDex() << "-DEX " << player->getCurAgi() << "-ALI "<<endl;

	for (int i = 0; i < 3; i++)
	{
		if (team[i] != nullptr)
		{
			float dis = player->owner->GetPosition().GetDistance(team[i]->owner->GetPosition());
			if (dis <= buffDis)
			{
				if (!AlreadyBuff(team[i], skillName))
				{
					team[i]->Buff(buff);
					cout << "Buff team member" << endl;

					if (immune)
					{
						team[i]->SetImmune(true);
					}
				}
				else
				{
					cout << "Already have" << enumString[(int)skillName - 1] << " buff" << endl;
				}
				buffMember[i] = 1;				
			}
		}
		else
		{
			break;
		}
	}
}

void PlayerSkillNode::BuffAutoAttackTeam()
{
	float speed = 1.2f;

	//TODO: careful with this, may effect conflict with animation time
	defaultStat[4] = player->getAttackSpeed();
	if (!AlreadyBuff(player, AbilityName::Shadow_Sneak))
	{
		player->setAttackSpeed(defaultStat[4] * speed);
		cout << "Buff Auto Atk: " << defaultStat[4] * speed
			<< "/nPlayer auto atk: " << player->getAttackSpeed() << endl;
	}
	else
	{
		cout << "Already have" << enumString[(int)AbilityName::Shadow_Sneak - 1] << " buff" << endl;
	}
	for (int i = 0; i < 3; i++)
	{
		if (team[i] != nullptr)
		{
			float dis = player->owner->GetPosition().GetDistance(team[i]->owner->GetPosition());
			if (dis <= buffDis)
			{
				if (!AlreadyBuff(player, AbilityName::Shadow_Sneak))
				{
					defaultStat[i] = team[i]->getAttackSpeed();
					team[i]->setAttackSpeed(defaultStat[i] * speed);
					cout << "Buff Auto Atk: " << defaultStat[i] * speed << endl;
				}
				else
				{
					cout << "Already have" << enumString[(int)AbilityName::Shadow_Sneak - 1] << " buff" << endl;
				}
				buffMember[i] = 1;
			}
		}
		else
		{
			break;
		}
	}
}

void PlayerSkillNode::DebuffStatsTeam(AbilityName skillName)
{
	bool immune = false;
	if (AbilityName::Mana_Burst == skillName && ability.level == maxLevel)
	{
		immune = true;
	}

	if (!player || (player && !player->getAlive()))
	{
		return;
	}
	else if (player && (timer.GetCurTimeSecond() >= buffTime))
	{
		try
		{
			cout << "Debuff stats, time: " << timer.GetCurTimeSecond() << endl;
			player->Debuff(buff);		

			if (immune)
			{
				player->SetImmune(false);
			}

			if (AbilityName::Ironclad == skillName)
			{
				LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
				if (battleMage)
				{
					battleMage->SetSlowdown(1.0f);
				}
			}

			//cout << "Debuff stats, time: " << timer.GetCurTimeSecond()
			//	<< "/nPlayer stats: " << player->getCurStr() << "-STR " << player->getCurCon() << "-CON " << player->getBaseStr() << "-RES "
			//	<< player->getCurIntell() << "-INT " << player->getCurWis() << "-WIS " << player->getCurDex() << "-DEX " << player->getCurAgi() << "-ALI " << endl;
			//cout << "Player stats: " << player->getCurStr() << "-STR " << player->getCurCon() << "-CON " << player->getBaseStr() << "-RES "
			//	<< player->getCurIntell() << "-INT " << player->getCurWis() << "-WIS " << player->getCurDex() << "-DEX " << player->getCurAgi() << "-ALI " << endl;

			for (int i = 0; i < 3; i++)
			{
				if (buffMember[i] == 1) //only debuff member with this Buff
				{
					team[i]->Debuff(buff);
					buffMember[i] = 0;

					if (immune)
					{
						team[i]->SetImmune(false);
					}
				}
				else
				{
					break;
				}
			}
			active = false;
			ToggleBoolBuffSkillTeam(skillName);
			cout<<"Debuff stats team"<<endl;
			restTime.StartTimer();
		}
		catch (...)
		{
			//use to avoid error when user close windows when buff is still active
			cout << "Error, can't debuff" << endl;
		}
		ability.onCooldown = false;
		return;
	}

	//remove buff bool if member die
	for (int i = 0; i < 3; i++)
	{
		if (!team[i] || (team[i] && !team[i]->getAlive()))
		{
			buffMember[i] = false;
		}
	}	
}

void PlayerSkillNode::DebuffAutoAttackTeam()
{
	if (!player || (player && !player->getAlive()))
	{
		return;
	}
	else if (player && (timer2.GetCurTimeSecond() >= buffTime))// || !player->getAlive()))
	{
		try
		{
			player->setAttackSpeed(defaultStat[4]);
			cout << "Debuff auto atk, time: " << timer2.GetCurTimeSecond()
					<<"/nPlayer auto atk: " << player->getAttackSpeed() << endl;
			for (int i = 0; i < 3; i++)
			{
				if (buffMember[i] == 1) //only debuff member with this Buff
				{
					team[i]->setAttackSpeed(defaultStat[i]);
					buffMember[i] = 0;
				}
				else
				{
					break;
				}
			}
			//restTime.StartTimer();
		}
		catch (...)
		{
			//use to avoid error when user close windows when buff is still active
			cout << "Error, can't debuff auto atk" << endl;
		}
		ability.onCooldown = false;
		return;
	}

	//remove buff bool if team member die
	for (int i = 0; i < 3; i++)
	{
		if (!team[i] || (team[i] && !team[i]->getAlive()))
		{
			buffMember[i] = false;
		}
	}	
}

void PlayerSkillNode::MoveObjectOverTime(Object * obj, float3 targetPos, float moveTimeMilli, float speed)
{
	if (!obj || (obj && !obj->GetIsAlive()))
	{
		return;
	}

	float dis = obj->GetPosition().GetDistance(targetPos);
	float3 dir = (targetPos - obj->GetPosition()).Normalize();
	//close enough to targetPos
	if (dis >= 0.2f && timer.GetCurTimeMillisecond() <= moveTimeMilli)
	{
		if (!obj)
		{
			return;
		}
		else if (obj && obj->GetIsAlive())
		{
			try
			{
				if (deltaTimerMove.GetCurTimeMillisecond() >= 30)
				{
					//not work, this move sphere but not Enemy itself
					//obj->transforms[0].position = obj->GetPosition() + dir*1.5f;

					obj->SetPosition(obj->GetPosition() + dir*speed);
					dis = obj->GetPosition().GetDistance(targetPos);
					dir = targetPos - obj->GetPosition();
					deltaTimerMove.StartTimer();
				}
				//restTime.StartTimer();
			}
			catch (...)
			{
				cout << "Error, can't move Object" << endl;
			}
		}
	}
	else
	{
		active = false;

		if (ability.name == AbilityName::Dash_Attack)
		{
			if (target)
			{
				// ------ Set a hitbox to active here ------ //
				hitboxCol->SetActive(true);
			}
		}
	}
}

void PlayerSkillNode::MoveGroupTargetOverTime(float3 targetPos, float speed, float dt)
{
	curTime += dt;
	//if (timer.GetCurTimeMillisecond() <= moveTimeMilli)
	if (curTime <= activeTime)
	{
		for (int i = 0; i < targets.size(); i++)
		{
			Object *obj = targets[i]->owner;

			float dis;
			float3 dir;

			if (!obj)
			{
				return;
			}
			else if (obj && obj->GetIsAlive())
			{
				try
				{
					if (deltaTimerMove.GetCurTimeMillisecond() >= 30)
					{
						dis = targets[i]->owner->GetPosition().GetDistance(targetPos);
						dir = (targetPos - targets[i]->owner->GetPosition()).Normalize();

						obj->SetPosition(obj->GetPosition() + dir*speed);
						dis = obj->GetPosition().GetDistance(targetPos);
						dir = targetPos - obj->GetPosition();
					}
					//restTime.StartTimer();
				}
				catch (...)
				{
					cout << "Error, can't move Object" << endl;
				}
			}
		}

		if (deltaTimerMove.GetCurTimeMillisecond() >= 30)
		{
			deltaTimerMove.StartTimer();
		}
	}
	else
	{
		active = false;
	}
}

void PlayerSkillNode::FlickingHitbox()
{
	if (timer.GetCurTimeSecond() <= 5.0f)
	{
		if (deltaTimer.GetCurTimeMillisecond() >= hbDeltaTime)
		{
			hitboxCol->SetActive(true);
		}		
		//reset after use, delay for Collider have to to work
		if (deltaTimer.GetCurTimeMillisecond() >= (hbDeltaTime + 100.0f))
		{
			deltaTimer.StartTimer();
			hitboxCol->SetActive(false);
		}
	}
	else
	{
		active = false;
	}
}

void PlayerSkillNode::ToggleBoolBuffSkill(LeComp_MOB * player, AbilityName skillName)
{
	LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage)
	{
		if (skillName == AbilityName::Battle_Rally)
		{
			battleMage->battleRallyBuff = !battleMage->battleRallyBuff;
		}
		else if (skillName == AbilityName::Ironclad)
		{
			battleMage->ironcladBuff = !battleMage->ironcladBuff;
		}
		else if (skillName == AbilityName::Mana_Burst)
		{
			battleMage->manaBurstBuff = !battleMage->manaBurstBuff;
		}
		else if (skillName == AbilityName::Hunters_Mark)
		{
			battleMage->huntersMarkBuff = !battleMage->huntersMarkBuff;
		}
		else if (skillName == AbilityName::Shadow_Sneak)
		{
			battleMage->shadowSneakBuff = !battleMage->shadowSneakBuff;
		}
	}
}

void PlayerSkillNode::ToggleBoolBuffSkillTeam(AbilityName skillName)
{
	ToggleBoolBuffSkill(player, skillName);
	for (int i = 0; i < 3; i++)
	{
		if (team[i] != nullptr)
		{
			ToggleBoolBuffSkill(team[i], skillName);
		}
	}
}

bool PlayerSkillNode::AlreadyBuff(LeComp_MOB * player, AbilityName skillName)
{
	LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage)
	{
		if (skillName == AbilityName::Battle_Rally)
		{
			return battleMage->battleRallyBuff;
		}
		else if (skillName == AbilityName::Ironclad)
		{
			return battleMage->ironcladBuff;
		}
		else if (skillName == AbilityName::Mana_Burst)
		{
			return battleMage->manaBurstBuff;
		}
		else if (skillName == AbilityName::Hunters_Mark)
		{
			return battleMage->huntersMarkBuff;
		}
		else if (skillName == AbilityName::Shadow_Sneak)
		{
			return battleMage->shadowSneakBuff;
		}
	}
	return false;
}

void PlayerSkillNode::BombExplode()
{
	float triggerDis = 2.0f;
	float aoe = 10.0f;

	float3 bombPos;
	int index = 0;
	for (auto iter = bombObj.begin(); iter != bombObj.end();)
	{	
		Object * bomb = DCurrentGameState->GetObjectByName((*iter));
		bombPos = bomb->GetPosition();
		//bombPos = (*iter)->GetPosition();

		//check if any E get close
		targets.clear();
		DCurrentGameState->CollisionSystem->ComponentSphereCast(bombPos, triggerDis, targets);

		//bomb trap explodes after a certain amount of time OR when triggered by enemy proximity
		if (bombTimer[index].GetCurTimeSecond() >= 10.0f || targets.size() > 0)
		{
			//injure all E in the AOE
			targets.clear();
			DCurrentGameState->CollisionSystem->ComponentSphereCast(bombPos, aoe, targets);

			cout<<"Total bomb: "<< bombObj.size() <<" - Explode bomb "<<(index+1)<<endl;

			bomb->SetIsAlive(false);
			//(*iter)->SetIsAlive(false);
			iter = bombObj.erase(iter);
			auto timerIter = bombTimer.begin();
			for (int j = 0; j < index; j++)
			{
				timerIter++;
			}
			timerIter = bombTimer.erase(timerIter);

			Object *obj = player->owner->owningGameState->AddObject(player->owner->GetName() + "Explosion" + std::to_string(numPE++));
			PE_Status *explosion = obj->AddComponent<PE_Status>(new PE_Status(nullptr, obj));
			explosion->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("explosion"));
			explosion->SetHavePlayer(false);
			explosion->SetPlayerIdChangeColor(false);
			explosion->SetPSCanDie(true);
			explosion->SetAliveTimePS(1.0f);
			explosion->SetOffset({ 0, 0.2f, 0 });
			explosion->SetMinMaxTextureSize({ 0, 0 }, { aoe, aoe });
			explosion->SetMaxParticle(1);
			explosion->InitializeParticle(LeRenderer::device);
			obj->SetPosition(bombPos);

			DSoundSystem->StartSoundByPath(L"grenade.wav");

			for (unsigned int i = 0; i < targets.size(); i++)
			{
				if (rand() % (maxLevel - (ability.level - 1)) == 0)
				{
					targets[i]->SetStatusEffect(Status::Confusion);
				}

				// ------ Set a hitbox to active here ------ //
				
				targets[i]->takeDamage(7);
			}
		}
		else {
			iter++;
			index++;
		}
	}
}

void PlayerSkillNode::BattleRallySkill()
{
	//Increases physical attack and defense for a short duration 
	//Upgrades will increase duration, percentage buff, and later on will also provide temporary hit points

	//ie: lv1: 5%, lv2: 10%
	float percentageBuff = (ability.level * 5.0f)/100.0f;
	int buffStr = (int)((float)player->getBaseStr() * percentageBuff);
	int buffRes = (int)(player->getBaseRes() * percentageBuff);
	int buffCon = 0;
	if (ability.level == maxLevel)
	{
		buffCon = (int)(player->getBaseCon() * percentageBuff);
	}
	CapMin(&buffStr);
	CapMin(&buffRes);

	buffTime = ability.level * 5.0f;
	buff = {buffStr, buffCon, buffRes, 0, 0, 0, 0};	
	BuffStatsTeam(AbilityName::Battle_Rally);
	ability.onCooldown = true;
	ToggleBoolBuffSkillTeam(AbilityName::Battle_Rally);
	cout<<"Battle Rally: +"<< buff.str<<" STR, +"<< buff.con<<" CON, +"<< buff.res<<" RES"<<endl;

#pragma region Buff effect
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "skill" + to_string(numPE++));
	PE_WarriorBuff *peBuff = particleSys->AddComponent<PE_WarriorBuff>();
	peBuff->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("buffStar"));
	peBuff->SetFixedColor({1, 0, 0});
	peBuff->SetMaxParticle(1);
	peBuff->SetAliveTimePS(0.5f);
	peBuff->SetAliveTimeEachParticle(0.5f);
	peBuff->InitializeParticle(LeRenderer::device);
	particleSys->SetPosition(player->owner->GetGlobalPosition());
#pragma endregion
}

void PlayerSkillNode::CleaveSkill()
{	
	activeTime = mesh->GetAnimationMaxTime();
	float3 playerPos = player->owner->GetPosition();

	//Spin in a small circle(enough to hit adjacent enemies) to deal damage around the warrior (ribbon)
	Object *obj = player->owner->owningGameState->AddObject(std::string("Ribbon Cleave") + std::to_string(numPE++));
	PE_Ribbon *ribbon = obj->AddComponent<PE_Ribbon>(new PE_Ribbon(obj, swordTip->GetSocket()->GetWorldMatrix(), swordEnd->GetSocket()->GetWorldMatrix()));
	ribbon->SetAliveTimePS(mesh->GetAnimationLastFrameTime());
	ribbon->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("swordTrail"));
	ribbon->SetAliveTimePS(activeTime);
	ribbon->InitializeParticle(LeRenderer::device);

	// ------ Set a hitbox to active here ------ //
	midSword = (swordTip->GetSocket()->GetPosition() + swordEnd->GetSocket()->GetPosition()) / 2.0f;
	hitboxCol->owner->SetPosition(midSword - playerPos);
	hitboxCol->SetActive(true);
	cout << "Enable hitbox" << endl;

	//After enough upgrades, the warrior can slowly move while attacking
	LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage)
	{
		battleMage->SetSlowdown(0.2f * ability.level);
	}

	active = true;
}

void PlayerSkillNode::IroncladSkill()
{
	//Greatly increases defense and magical defense 
	float percentageBuff = (ability.level * 5.0f)/100.0f;
	int buffRes = (int)(player->getBaseRes() * percentageBuff);
	int buffWis = (int)(player->getBaseWis() * percentageBuff);
	CapMin(&buffRes);
	CapMin(&buffWis);

	buffTime = ability.level * 5.0f;
	buff = { 0, 0, buffRes, 0, buffWis, 0, 0};
	BuffStatsTeam(AbilityName::Ironclad);
	ability.onCooldown = true;
	ToggleBoolBuffSkillTeam(AbilityName::Ironclad);
	cout<<"IroncladSkill: +"<< buff.res<<" RES +"<< buff.wis<<" WIS"<<endl;

#pragma region PE
	Object *pS = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE Ironclad" + std::to_string(numPE++));
	PE_Status* peVortex = pS->AddComponent<PE_Status>(new PE_Status(player, pS));
	peVortex->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("shield"));
	peVortex->SetHavePlayer(true);
	peVortex->SetPSCanDie(true);
	peVortex->SetAliveTimePS(1.0f);
	peVortex->SetMinMaxDistance(4.5f, 4.5f);
	peVortex->SetPlayerIdChangeColor(false);
	peVortex->SetMaxParticle(1);
	peVortex->SetOffset({ 0, 4.5f, 0 });
	peVortex->SetMinMaxTextureSize({ 0, 0 }, { 1.0f, 1.0f });
	peVortex->InitializeParticle(LeRenderer::device);
#pragma endregion

	//losing auto - attack
	
	//losing movement speed
	LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage)
	{
		battleMage->SetSlowdown(0.5f);
	}

	//draws the aggro of nearby enemies by a large portion
	targets.clear();
	DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), 15.0f, targets);
	for (unsigned int i = 0; i < targets.size(); i++)
	{
		targets[i]->AddAgro(player->owner, (float)INT_MAX);
	}
}

void PlayerSkillNode::TwoHandedStrikeSkill()
{
	activeTime = 1.0f;
	//Will activate on his next auto attack
	LeComp_BattleMageBehavior *battleMage = dynamic_cast<LeComp_BattleMageBehavior *> (player);
	if (battleMage && battleMage->GetTarget())
	{
		//does extra damage
		//battleMage->GetTarget()->takeDamage(10);

		// ------ Set a hitbox to active here ------ //
		delayHitboxTime = mesh->GetAnimationMaxTime() * 0.5f;
		curDelayHBTime = 0;

		active = true;
		//possibility of bleed damage
		//lv 1 = 20%, 2 = 25%, 3 = 33%, 4 = 50%, 5 = 100%
		if (rand() % (maxLevel - (ability.level - 1)) == 0)
		{
			battleMage->GetTarget()->ChangeStatus(Status::Bleed, ability.level);
		}
	}
}

void PlayerSkillNode::ShieldBashSkill()
{
	activeTime = mesh->GetAnimationMaxTime();
	//TODO: delay lost hp after player touch enemy & e move back after lose hp

	//LeComp_BattleMageBehavior *battleMage = dynamic_cast<LeComp_BattleMageBehavior *> (player);
	//if (battleMage && battleMage->GetTarget())
	if(target)
	{
		LeComp_BattleMageBehavior* battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
		if (battleMage)
		{
			battleMage->SetSlowdown(0.1f * ability.level);
		}

		//EnemyAI* enemy = dynamic_cast<EnemyAI*>(battleMage->GetTarget());
		EnemyAI* enemy = dynamic_cast<EnemyAI*>(target);
		float3 enemyPos = enemy->owner->GetPosition();
		float3 playerPos = player->owner->GetPosition();

		enemy->FreezeEnemy(1.5f);

		//ram it into the enemy - Animation

		//Slightly less focused on damage
		//enemy->takeDamage(3);

		// ------ Set a hitbox to active here ------ //
		hitboxCol->SetActive(true);
		cout << "Enable hitbox" << endl;

		//Small chance to cause confusion
		if (rand() % (maxLevel - (ability.level - 1)) == 0)
		{
			enemy->ChangeStatus(Status::Confusion);
		}

		//knocks away enemies in front of the warrior, Move E in Update
		timer.StartTimer();
		deltaTimer.StartTimer();
		deltaTimerMove.StartTimer();

		active = true;
	}
}

void PlayerSkillNode::ManaBurstSkill()
{
	//Increases magic attack and magic defense for a short duration
	float percentageBuff = (ability.level * 5.0f)/100.0f;
	int buffInt = (int)(player->getBaseIntell() * percentageBuff);
	int buffWis = (int)(player->getBaseWis() * percentageBuff);
	CapMin(&buffInt);
	CapMin(&buffWis);
	buffTime = ability.level * 5.0f;
	buff = { 0, 0, 0, buffInt, buffWis, 0, 0 };
	BuffStatsTeam(AbilityName::Mana_Burst);
	ability.onCooldown = true;
	ToggleBoolBuffSkillTeam(AbilityName::Mana_Burst);
	cout << "Mana Burst: +" << buff.intell << " INT +" << buff.wis << " WIS" << endl;

#pragma region Buff effect
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "skill" + to_string(numPE++));
	PE_WarriorBuff *peBuff = particleSys->AddComponent<PE_WarriorBuff>();
	peBuff->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("buffStar"));
	peBuff->SetFixedColor({ 0, 0, 1 });
	peBuff->SetMaxParticle(1);
	peBuff->SetAliveTimePS(0.5f);
	peBuff->SetAliveTimeEachParticle(0.5f);
	peBuff->InitializeParticle(LeRenderer::device);
	particleSys->SetPosition(player->owner->GetGlobalPosition());
#pragma endregion
}

void PlayerSkillNode::RestorationSkill()
{
	//Heals allies in a large AoE for a bit of health
	float percentageBuff = ((ability.level+2) * 10)/100.0f;
	int hp = (int)(player->getMaxHealth() * percentageBuff);
	ability.onCooldown = true;

	float aoeBuff = buffDis * 2;
	player->RestoreHP(hp);
	
	if (ability.level == maxLevel)
	{
		player->StopAllStatuses();
	}

#pragma region PE
	Object *pS1 = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE AOE Restoration" + std::to_string(numPE++));
	PE_Status* peAOE = pS1->AddComponent<PE_Status>(new PE_Status(player, pS1));
	peAOE->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("magicCircle2"));
	peAOE->SetFixedColor({ 0.5f, 1.0f, 0.5f });
	peAOE->SetHavePlayer(true);
	peAOE->SetCanRotate(true);
	peAOE->SetRotateSpeed(200.0f);
	peAOE->SetPSCanDie(true);
	peAOE->SetAliveTimePS(1.0f);
	peAOE->SetMinMaxDistance(4.5f, 4.5f);
	peAOE->SetPlayerIdChangeColor(false);
	peAOE->SetMaxParticle(1);
	peAOE->SetOffset({ 0, 0.2f, 0 });
	peAOE->SetMinMaxTextureSize({ 0, 0 }, { aoeBuff, aoeBuff });
	peAOE->SetTransparent(0.1f);
	peAOE->InitializeParticle(LeRenderer::device);

	Object *pS = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE Restoration" + std::to_string(numPE++));
	PE_Status* peCircle = pS->AddComponent<PE_Status>(new PE_Status(player, pS));
	peCircle->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("magicCircle2"));
	peCircle->SetFixedColor({0.5f, 1.0f, 0.5f});
	peCircle->SetHavePlayer(true);
	peCircle->SetCanRotate(true);
	peCircle->SetRotateSpeed(400.0f);
	peCircle->SetPSCanDie(true);
	peCircle->SetAliveTimePS(2.0f);
	peCircle->SetMinMaxDistance(4.5f, 4.5f);
	peCircle->SetPlayerIdChangeColor(false);
	peCircle->SetMaxParticle(1);
	peCircle->SetOffset({ 0, 0.2f, 0 });
	peCircle->SetMinMaxTextureSize({ 0, 0 }, { 2.0f, 2.0f });
	peCircle->SetTransparent(0.7f);
	peCircle->InitializeParticle(LeRenderer::device);
#pragma endregion

	for (int i = 0; i < 3; i++)
	{
		if (team[i] != nullptr)
		{
			float dis = player->owner->GetPosition().GetDistance(team[i]->owner->GetPosition());
			if (dis <= aoeBuff)
			{
				team[i]->RestoreHP(hp);

				if (ability.level == maxLevel)
				{
					team[i]->StopAllStatuses();
				}
#pragma region PE
				Object *pS = team[i]->owner->owningGameState->AddObject(team[i]->owner->GetName() + "PE Restoration" + std::to_string(numPE++));
				PE_Status* peTeam = pS->AddComponent<PE_Status>(new PE_Status(team[i], pS));
				peTeam->SetMaterial(team[i]->owner->owningGameState->Renderer->GetMaterial("magicCircle2"));
				peTeam->SetFixedColor({ 0.5f, 1.0f, 0.5f });
				peTeam->SetHavePlayer(true);
				peTeam->SetCanRotate(true);
				peTeam->SetRotateSpeed(400.0f);
				peTeam->SetPSCanDie(true);
				peTeam->SetAliveTimePS(2.0f);
				peTeam->SetMinMaxDistance(4.5f, 4.5f);
				peTeam->SetPlayerIdChangeColor(false);
				peTeam->SetMaxParticle(1);
				peTeam->SetOffset({ 0, 0.2f, 0 });
				peTeam->SetMinMaxTextureSize({ 0, 0 }, { 2.0f, 2.0f });
				peTeam->SetTransparent(0.7f);
				peTeam->InitializeParticle(LeRenderer::device);
#pragma endregion
			}
		}
		else
		{
			break;
		}
	}
}

void PlayerSkillNode::FireballSkill()
{
	activeTime = 1.5f;
	//The mage launches a long range fire projectile at a single enemy
	float fireballHigh = 5.0f;
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "Fireball" + std::to_string(numPE++));
	float3 playerPos = player->owner->GetTransform()->position;
	//PE_Shot *peShot = nullptr;
	if (target)
	{
		float3 enemyPos = target->owner->GetPosition();
		// ------ Set a hitbox to active here ------ //
		if (ability.level < maxLevel)
		{
			peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, enemyPos + float3{ 0, 1, 0 }*fireballHigh, enemyPos, ability.name, true, player));
			//peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, enemyPos + float3{ 0, 1, 0 }*fireballHigh, enemyPos, true, player));
		}
		else
		{
			peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, enemyPos + float3{ 0, 1, 0 }*fireballHigh, enemyPos, ability.name, true, player, true));
			//peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, enemyPos + float3{ 0, 1, 0 }*fireballHigh, enemyPos, true, player, true));
		}
		
		active = true;		
	}
	else
	{
		//aim Forward for now when there is no Target
		float3 target = playerPos + player->owner->GetForward() * 5;
		float3 startPos = target + float3{ 0, 1, 0 }*fireballHigh;
		if (ability.level < maxLevel)
		{
			peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, target + float3{ 0, 1, 0 }*fireballHigh, target));
		}
		else
		{
			peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, target + float3{ 0, 1, 0 }*fireballHigh, target, ability.name, true, player, true));
			//peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, target + float3{ 0, 1, 0 }*fireballHigh, target, true, player, true));
		}
	}
	peShot->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("fireball"));
	peShot->SetTransparent(0.7f);
	peShot->SetMaxDistance(fireballHigh);
	peShot->SetDirection(3);
	peShot->SetFixedColor({1, 1, 1});
	peShot->SetAliveTimePS(activeTime);
	peShot->InitializeParticle(LeRenderer::device);
}

void PlayerSkillNode::LightningBolt()
{
	activeTime = 0.5f;
	float3 playerPos = player->owner->GetPosition();

	float aimDis = aimPos.GetDistance(playerPos);
	float3 targetDir = aimPos - playerPos;

	//peLightning = particleSys->AddComponent<PE_Lightning>(new PE_Lightning(particleSys, playerPos, target->owner->GetPosition()));
		////target->takeDamage(5);
		//// ------ Set a hitbox to active here ------ //
		//hitboxCol->owner->SetPosition(target->owner->GetPosition() - playerPos);
		//hitboxCol->SetActive(true);
		//target->ChangeStatus(Status::Paralysis);
		//active = true;

	targets.clear();
	DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), 6.0f + ability.level, targets);
	float haftArc = 80.0f;

	if (targets.size() == 0)
	{
		//aim Forward for now when there is no Target
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "Lightning" + std::to_string(numPE++));
		PE_Lightning *peLightning = particleSys->AddComponent<PE_Lightning>(new PE_Lightning(particleSys, playerPos, aimPos));
		peLightning->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("lightning"));
		peLightning->SetTransparent(0.7f);
		peLightning->InitializeParticle(LeRenderer::device);
		return;
	}

	for (unsigned int i = 0; i < targets.size(); i++)
	{
		if ((i+1) > (unsigned int)ability.level)
		{
			break;
		}

		if (!targets[i] || !targets[i]->owner->GetIsAlive()) {
			targets[i] = targets.back();
			targets.pop_back();
			continue;
		}

		float3 enemyPos = targets[i]->owner->GetPosition();
		float3 dir = enemyPos - player->owner->GetPosition();
		dir.Normalize();
		float degree = XMConvertToDegrees(acos(LeMath::Dot(targetDir, dir) / (targetDir.GetLength() * dir.GetLength())));

		if (degree >= -haftArc && degree <= haftArc)
		{
			Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "Lightning" + std::to_string(numPE++));
			PE_Lightning *peLightning = particleSys->AddComponent<PE_Lightning>(new PE_Lightning(particleSys, playerPos, targets[i]->owner->GetPosition(), true, player));
			peLightning->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("lightning"));
			peLightning->SetTransparent(0.7f);
			peLightning->InitializeParticle(LeRenderer::device);

			targets[i]->ChangeStatus(Status::Paralysis);				
			active = true;
		}

		++i;
	}


}

void PlayerSkillNode::BlizzardSkill()
{	
	timer.StartTimer(); //use to increase Damage over time

	bool effect = false;
	deltaTimer.StartTimer();
	hbDeltaTime = 800.0f;
	hitboxRad = ability.level*0.5f + 2.0f;
	((SphereCollider*)hitboxCol)->SetRadius(hitboxRad);

#pragma region PE
	Object *pS = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE Blizzard" + std::to_string(numPE++));
	PE_Status* peCircle = pS->AddComponent<PE_Status>(new PE_Status(player, pS));
	peCircle->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("snowHorizontal"));
	peCircle->SetHavePlayer(true);
	peCircle->SetPSCanDie(true);
	peCircle->SetAliveTimePS(1.0f);
	peCircle->SetPlayerIdChangeColor(false);
	peCircle->SetMaxParticle(1);
	peCircle->SetOffset({ 0, 0.2f, 0 });
	peCircle->SetMinMaxTextureSize({ 0, 0 }, { hitboxRad*2.0f, hitboxRad*2.0f });
	peCircle->SetTransparent(0.3f);
	peCircle->InitializeParticle(LeRenderer::device);
#pragma endregion

	DSoundSystem->StartSoundByPath(L"freeze.wav");

	targets.clear();
	DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), hitboxRad, targets);
	for (unsigned int i = 0; i < targets.size(); i++)
	{
		//TODO: has a decent chance (increases over time) of freezing the targets
		if (ability.level > (rand() % maxLevel))
		{
			targets[i]->ChangeStatus(Status::Frozen);
		}
	}

	hitboxCol->SetActive(true);
	active = true;
}

void PlayerSkillNode::HuntersMarkSkill()
{
	//Increases accuracy and agility of allies within a wide range for a short duration
	float percentageBuff = (ability.level * 5.0f)/100.0f;
	int buffDex = (int)(player->getBaseDex() * percentageBuff);
	int buffAli = (int)(player->getBaseAgi() * percentageBuff);
	//increase crit after few update
	if (ability.level == maxLevel)
	{
		buffDex  = (int)(buffDex * 1.5f);
	}
	CapMin(&buffDex);
	CapMin(&buffAli);

	buffTime = ability.level * 5.0f;
	buff = { 0, 0, 0, 0, 0, buffDex, buffAli};
	BuffStatsTeam(AbilityName::Hunters_Mark);
	ability.onCooldown = true;
	ToggleBoolBuffSkillTeam(AbilityName::Hunters_Mark);
	cout << "HuntersMark: +" << buff.dex << " DEX +" << buff.agility << " ALI" << endl;

#pragma region Buff effect
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "skill" + to_string(numPE++));
	PE_WarriorBuff *peBuff = particleSys->AddComponent<PE_WarriorBuff>();
	peBuff->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("buffStar"));
	peBuff->SetFixedColor({ 0, 1, 0 });
	peBuff->SetMaxParticle(1);
	peBuff->SetAliveTimePS(0.5f);
	peBuff->SetAliveTimeEachParticle(0.5f);
	peBuff->InitializeParticle(LeRenderer::device);
	particleSys->SetPosition(player->owner->GetGlobalPosition());
#pragma endregion
}

void PlayerSkillNode::EntangleSkill()
{
	activeTime = 1.5f;
	//LeComp_BattleMageBehavior *battleMage = dynamic_cast<LeComp_BattleMageBehavior *> (player);
	//if (battleMage && battleMage->GetTarget())
	if(target)
	{
		float3 playerPos = player->owner->GetPosition();
		float3 enemyPos = target->owner->GetPosition();
		float dis = enemyPos.GetDistance(playerPos);
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "EntangleArrow" + std::to_string(numPE++));
		//PE_Shot *peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos));
		peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos));
		peShot->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot->SetOffset({0, 0.2f, 0});
		peShot->SetFixedColor({0, 1, 0});
		peShot->SetMaxDistance(dis);
		peShot->SetAliveTimePS(activeTime);
		peShot->InitializeParticle(LeRenderer::device);

		//arrow deals less damage
		EnemyAI *enemy = dynamic_cast<EnemyAI*>(target);
		//enemy->takeDamage(3);
		// ------ Set a hitbox to active here ------ //
		hitboxCol->SetActive(true);
		active = true;
	
		//snares the target for a short duration(unaffected by paralysis resistance / vulnerability)
		enemy->FreezeEnemy((float)ability.level);

		//After a few upgrades, the entangle can create thorny vines that hurt the enemy over time
		if (ability.level == maxLevel)
		{
			enemy->ChangeStatus(Status::Bleed, ability.level);
		}

		Object *pS = enemy->owner->owningGameState->AddObject(enemy->owner->GetName() + "PE Vine" + std::to_string(numPE++));
		PE_Status* peVine = pS->AddComponent<PE_Status>(new PE_Status(nullptr, pS));
		peVine->SetMaterial(enemy->owner->owningGameState->Renderer->GetMaterial("vine"));
		peVine->SetHavePlayer(false);
		peVine->SetPSCanDie(true);
		peVine->SetAliveTimePS(1.0f);
		peVine->SetMinMaxDistance(4.5f, 4.5f);
		peVine->SetPlayerIdChangeColor(false);
		peVine->SetMaxParticle(1);
		peVine->SetOffset({ 0, 0.5f, 0 });
		peVine->SetMinMaxTextureSize({ 0, 0 }, { 3.0f, 3.0f });
		peVine->InitializeParticle(LeRenderer::device);
		pS->SetPosition(enemyPos);
	}
	else
	{
		float3 playerPos = player->owner->GetPosition();
		float3 targetPos = player->owner->GetPosition() + player->owner->GetForward() * 7.0f;
		float dis = targetPos.GetDistance(playerPos);
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "EntangleArrow" + std::to_string(numPE++));
		PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, targetPos));
		peShot1->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot1->SetOffset({ 0, 0.2f, 0 });
		peShot1->SetFixedColor({ 0, 1, 0 });
		peShot1->SetMaxDistance(dis);
		peShot1->SetAliveTimePS(activeTime);
		peShot1->InitializeParticle(LeRenderer::device);
	}
}

void PlayerSkillNode::MultishotSkill()
{
	//fires multiple arrows in a fan with a white aura around them
	//strike multiple enemies directly in front of the ranger
	
	//this skill doesn't use active, but can set back after time if neceessary
	if (target)
	{
		targets.clear();
		DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), 10.0f, targets);

		float3 playerPos = player->owner->GetPosition();
		float3 targetDir = target->owner->GetPosition() - playerPos;

		float haftArc = 45.0f + ability.level * 5.0f;

		for (unsigned int i = 0; i < targets.size(); i++)
		{		
			float3 enemyPos = targets[i]->owner->GetPosition();
			float3 dir = enemyPos - player->owner->GetPosition();
			dir.Normalize();
			float degree = XMConvertToDegrees(acos(LeMath::Dot(targetDir, dir) / (targetDir.GetLength() * dir.GetLength())));

			if (degree >= -haftArc && degree <= haftArc)
			{
#pragma region Arrow
				float dis = enemyPos.GetDistance(playerPos);
				Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "MultiArrow" + std::to_string(player->owner->GetPlayerNumber()) + std::to_string(numPE++));
				// ------ Set a hitbox to active here ------ //
				PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos, ability.name, true, player));
				//PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos, true, player));
				peShot1->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
				peShot1->SetOffset({ 0, 0.2f, 0 });
				peShot1->SetMaxDistance(dis);
				peShot1->SetFixedColor({ 1, 1, 1 });
				peShot1->InitializeParticle(LeRenderer::device);
#pragma endregion
				active = true;
			}
		}
	}
	else
	{
		float3 playerPos = player->owner->GetPosition();		
		float dis = 7.0f;
		float3 targetPos = player->owner->GetPosition() + player->owner->GetForward() *dis;
		
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "MultiShot No Target" + std::to_string(player->owner->GetPlayerNumber()) + std::to_string(numPE++));
		PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, targetPos));
		peShot1->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot1->SetOffset({ 0, 0.2f, 0 });
		peShot1->SetMaxDistance(dis);
		peShot1->SetFixedColor({ 1, 1, 1 });
		peShot1->InitializeParticle(LeRenderer::device);
		
		active = false;
	}
}

void PlayerSkillNode::ArrowVolleySkill()
{
	activeTime = 2.0f;
	hbDeltaTime = 800.0f;

	float3 playerPos = player->owner->GetTransform()->position;
	float aimDis = aimPos.GetDistance(playerPos);	
	//if (aimPos == float3{0, 0, 0})
	//{
	//	aimPos = playerPos + player->owner->GetForward()*7.0f;
	//}

#pragma region PE
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "ArrowVolley" + std::to_string(player->owner->GetPlayerNumber()) + std::to_string(numPE++));
	PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, aimPos));
	peShot1->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
	peShot1->SetOffset({ 0, 0.2f, 0 });
	peShot1->SetMaxDistance(aimDis);
	peShot1->SetAliveTimePS(0.3f);
	peShot1->SetFixedColor({1, 1, 1});
	peShot1->InitializeParticle(LeRenderer::device);

	//TODO: delay this for a while util arrow hit
	Object *pS = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE Vortex" + std::to_string(numPE++));
	PE_Status* peVortex = pS->AddComponent<PE_Status>(new PE_Status(nullptr, pS));
	peVortex->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("vortex"));
	peVortex->SetHavePlayer(false);
	peVortex->SetCanRotate(true);
	peVortex->SetRotateSpeed(500.0f);
	peVortex->SetPSCanDie(true);
	peVortex->SetAliveTimePS(activeTime);
	peVortex->SetMinMaxDistance(4.5f, 4.5f);
	peVortex->SetPlayerIdChangeColor(false);
	peVortex->SetMaxParticle(1);
	peVortex->SetOffset({ 0, 0.2f, 0 });
	peVortex->SetMinMaxTextureSize({ 0, 0 }, { hitboxRad*2.0f, hitboxRad*2.0f });
	peVortex->InitializeParticle(LeRenderer::device);
	pS->SetPosition(aimPos);
#pragma endregion

	//find out which enemy in Vortex
	//*2 to get move E
	targets.clear();
	DCurrentGameState->CollisionSystem->ComponentSphereCast(aimPos, hitboxRad * 2.0f, targets);
	for (unsigned int i = 0; i < targets.size(); )
	{
		if (!targets[i] || !targets[i]->owner->GetIsAlive()) {
			targets[i] = targets.back();
			targets.pop_back();
			continue;
		}
		//making them take damage as well as become vulnerable to AoE attacks from other players
		//targets[i]->takeDamage(5);

		//inflict confusion
		targets[i]->ChangeStatus(Status::Confusion);
		//drawing enemies in
		targets[i]->FreezeEnemy(activeTime);
		++i;
	}
	
	timer.StartTimer();
	deltaTimer.StartTimer();
	deltaTimerMove.StartTimer();

	// ------ Set a hitbox to active here ------ //
	hitboxCol->owner->SetPosition(aimPos - playerPos);
	hitboxCol->SetActive(true);
	
	active = true;
}

void PlayerSkillNode::PoisonShotSkill()
{
	activeTime = 1.5f;

	//LeComp_BattleMageBehavior *battleMage = dynamic_cast<LeComp_BattleMageBehavior *> (player);
	//if (battleMage && battleMage->GetTarget())
	if(target)
	{
		float3 playerPos = player->owner->GetPosition();
		//float3 enemyPos = battleMage->GetTarget()->owner->GetPosition();
		float3 enemyPos = target->owner->GetPosition();
		float dis = playerPos.GetDistance(enemyPos);
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "Poison" + std::to_string(numPE++));
		//PE_Shot *peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos));
		peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos));
		peShot->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot->SetOffset({ 0, 0.2f, 0 });
		peShot->SetMaxDistance(dis);
		peShot->SetFixedColor(float3{1.0f, 0, 1.0f});
		peShot->SetAliveTimePS(activeTime);
		peShot->InitializeParticle(LeRenderer::device);

		//arrow deals less damage
		//target->takeDamage(3);
		// ------ Set a hitbox to active here ------ //
		hitboxCol->SetActive(true);
		active = true;

		//chance of poisoning the target
		if (rand() % (maxLevel - (ability.level - 1)) == 0)
		{
			target->ChangeStatus(Status::Poison, ability.level);
		}
	}
	else
	{
		float dis = 7.0f;
		float3 playerPos = player->owner->GetPosition();
		float3 enemyPos = player->owner->GetPosition() + player->owner->GetForward() * dis;
		Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "Poison" + std::to_string(numPE++));
		PE_Shot *peShot1 = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, playerPos, enemyPos));
		peShot1->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot1->SetOffset({ 0, 0.2f, 0 });
		peShot1->SetMaxDistance(dis);
		peShot1->SetFixedColor(float3{ 1.0f, 0, 1.0f });
		peShot1->SetAliveTimePS(activeTime);
		peShot1->InitializeParticle(LeRenderer::device);
	}
}

void PlayerSkillNode::ShadowSneakSkill()
{
	//Increases auto attack speed
	BuffAutoAttackTeam();

	//Increases agility for allies within range
	float percentageBuff = (ability.level * 5.0f)/100.0f;
	int buffAli = (int)(player->getBaseAgi() * percentageBuff);
	CapMin(&buffAli);

	buffTime = ability.level * 5.0f;
	buff = { 0, 0, 0, 0, 0, 0, buffAli };
	BuffStatsTeam(AbilityName::Shadow_Sneak);
	ability.onCooldown = true;
	ToggleBoolBuffSkillTeam(AbilityName::Shadow_Sneak);
	cout << "Shadow Sneak: +" << buff.agility << " ALI" << endl;

	//TODO: add double damage after have auto atk function (in maxLevel)
	if (ability.level == maxLevel)
	{

	}

#pragma region Buff effect
	Object *particleSys = player->owner->owningGameState->AddObject(player->owner->GetName() + "skill" + to_string(numPE++));
	PE_WarriorBuff *peBuff = particleSys->AddComponent<PE_WarriorBuff>();
	peBuff->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("buffStar"));
	peBuff->SetFixedColor({ 1, 0, 1 });
	peBuff->SetMaxParticle(1);
	peBuff->SetAliveTimePS(0.5f);
	peBuff->SetAliveTimeEachParticle(0.5f);
	peBuff->InitializeParticle(LeRenderer::device);
	particleSys->SetPosition(player->owner->GetGlobalPosition());
#pragma endregion
}

void PlayerSkillNode::FlurrySkill()
{
	//Multi-hit sword slash combo.
	//A multi - hit combo that hits a single enemy with a large finishing blow

	//TODO: connect hitbox to sword after have animation
	activeTime = 1.0f;
	hitboxCol->SetActive(true);
	active = true;
}

void PlayerSkillNode::CloakDaggerSkill()
{
	activeTime = 2.0f + ability.level;
	//The rogue cloaks himself in an aura that greatly reduces aggro
	Object *pS = player->owner->owningGameState->AddObject(player->owner->GetName() + "PE CloakDagger" + std::to_string(numPE++));
	PE_Status* peDust = pS->AddComponent<PE_Status>(new PE_Status(player, pS));
	peDust->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("cloudDust"));
	peDust->SetPSCanDie(true);
	peDust->SetAliveTimePS(2.0f);
	peDust->SetMinMaxDistance(4.5f, 4.5f);
	peDust->SetPlayerIdChangeColor(false);
	peDust->SetMaxParticle(1);
	peDust->SetOffset({ 0, 0.5f, 0 });
	peDust->SetMinMaxTextureSize({ 0, 0 }, { 2.0f, 2.0f });
	peDust->SetTransparent(0.5f);
	peDust->InitializeParticle(LeRenderer::device);

	targets.clear();
	DCurrentGameState->CollisionSystem->ComponentSphereCast(player->owner->GetPosition(), 10.0f, targets);
	for (unsigned int i = 0; i < targets.size(); i++)
	{
		targets[i]->SetAggro(player->owner);
	}

	//auto attack

	//may opt(before the aura has disappeared) for the second half of the ability, which is a backstab - ability that deals heavy damage but ends the aura
	active = true;
}

void PlayerSkillNode::DisableCloak(float dt)
{
	curTime += dt;
	if (curTime >= activeTime)
	{
		for (unsigned int i = 0; i < targets.size(); i++)
		{
			targets[i]->SetAggro(player->owner, 1);
		}
		active = false;
	}
}

void PlayerSkillNode::DaggerSkill()
{
	LeComp_BattleMageBehavior *battleMage = dynamic_cast<LeComp_BattleMageBehavior*>(player);
	if (battleMage)
	{
		//TODO: may use trigger instead of button
		//have to change code if want to use trigger, this will be trigger inmmedially
		//if (DInput->GetControllerAxis(battleMage->GetCurController(), EControllerAxis::RIGHT_TRIGGER).magnitude > 0.f)
		if(DInput->GetControllerButtonUp(battleMage->GetCurController(), EControllerButton::B))
		{
			hitboxCol->SetActive(true);

			//cancel Cloak
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				targets[i]->SetAggro(player->owner, 1);
			}
			active = false;
		}
	}
}

void PlayerSkillNode::BombtrapSkill()
{
	float3 bombPos = player->owner->GetPosition() + player->owner->GetForward() * 2.0f;

	string tempName = player->owner->GetName() + "Bomb" + std::to_string(numPE++);
	Object *obj = player->owner->owningGameState->AddObject(tempName);
	LeComp_StaticMesh *objMesh = obj->AddComponent<LeComp_StaticMesh>();
	objMesh->LoadObject("Boom.lme");
	objMesh->mat = player->owner->owningGameState->Renderer->GetMaterial("basicBoom");
	obj->SetPosition(bombPos);
	bombObj.push_back(tempName);

	QueryPerformanceTimer tempTimer;
	tempTimer.StartTimer();
	bombTimer.push_back(tempTimer);
}

void PlayerSkillNode::DashAttackSkill()
{
	activeTime = 0.5f;
	//The rogue quickly dashes forward a good distance and does a cross slash at the end
	if(target)
	{
		EnemyAI* enemy = dynamic_cast<EnemyAI*>(target);
		float3 enemyPos = enemy->owner->GetPosition();
		float3 playerPos = player->owner->GetPosition();
		float3 dir = enemyPos - playerPos;

		enemy->FreezeEnemy(1.0f);
		// ------ Set a hitbox to active here ------ //

		//hitboxCol->SetActive(true);

		timer.StartTimer();
		deltaTimerMove.StartTimer();

		active = true;
	}
}


PlayerSkillNode::PlayerSkillNode(LeComp_MOB * p, int _unlockPoints[5], Ability _ability, LeComp_MOB *_team[3], ControllerAxis _stickDir)
{
	player = p;
	memcpy(unlockPoints, _unlockPoints, sizeof(int)*5);
	ability = _ability;
	buffTime = 0;
	buffDis = 20.0f;
	numPE = 0;
	aimPos = {0, 0, 0};
	target = nullptr;
	active = false;
	curTime = 0;
	hitboxRad = 0;
	restTime.StartTimer();
	for (int i = 0; i < 3; i++)
	{
		team[i] = _team[i];
	}
	stickDir = _stickDir;
	
	enumString = {
		"Battle_Rally",
		"Cleave",
		"Ironclad",
		"Two_Handed_Strike",
		"Shield_Bash",
		//Mage
		"Mana_Burst",
		"Restoration",
		"Fireball",
		"Lightning_Bolt",
		"Blizzard",
		//Ranger
		"Hunters_Mark",
		"Entangle",
		"Multishot",
		"Arrow_Volley",
		"Poison_Shot",
		//Rogue
		"Shadow_Sneak",
		"Flurry",
		"Cloak_and_Dagger",
		"Bomb_trap",
		"Dash_Attack " };

	statusString = {
		"Bleed",
		"Poison",
		"Frozen",
		"Confusion",
		"Paralysis" };

	CreateHitbox();
}

PlayerSkillNode::~PlayerSkillNode()
{
	//if (childs.size() > 0)
	//{
	//	for (int i = 0; i < childs.size(); i++)
	//	{
	//		delete childs[i];
	//	}
	//	childs.clear();
	//}
}

void PlayerSkillNode::DisableBuffEarly()
{
	buffTime = 0.0f;
}

void PlayerSkillNode::ActiveSkill(LeComp_MOB* skillTarget, float3 _aimPos)
{
	target = skillTarget;
	aimPos = _aimPos;
	if (restTime.GetCurTimeSecond() > ability.cooldown)
	{
		ability.onCooldown = false;
	}
	if (!ability.onCooldown)// && (restTime.GetCurTimeSecond() > ability.cooldown))
	{
		cout << "Active " << enumString[(int)ability.name] << endl;
		switch (ability.name)
		{
		case AbilityName::Null:
			break;
		case AbilityName::Battle_Rally:
			BattleRallySkill();
			active = true;
		break;
		case AbilityName::Cleave:
			CleaveSkill();
			break;
		case AbilityName::Ironclad:
			IroncladSkill();
			active = true;
			break;
		case AbilityName::Two_Handed_Strike:
			TwoHandedStrikeSkill();
			break;
		case AbilityName::Shield_Bash:
			ShieldBashSkill();
			break;
		case AbilityName::Mana_Burst:
			ManaBurstSkill();
			active = true;
			break;
		case AbilityName::Restoration:
			RestorationSkill();
			break;
		case AbilityName::Fireball:
			FireballSkill();
			break;
		case AbilityName::Lightning_Bolt:
			LightningBolt();
			break;
		case AbilityName::Blizzard:
			BlizzardSkill();
			break;
		case AbilityName::Hunters_Mark:
			HuntersMarkSkill();
			active = true;
			break;
		case AbilityName::Entangle:
			EntangleSkill();
			break;
		case AbilityName::Multishot:
			MultishotSkill();
			break;
		case AbilityName::Arrow_Volley:
			ArrowVolleySkill();
			break;
		case AbilityName::Poison_Shot:
			PoisonShotSkill();
			break;
		case AbilityName::Shadow_Sneak:
			ShadowSneakSkill();
			active = true;
			break;
		case AbilityName::Flurry:
			FlurrySkill();
			break;
		case AbilityName::Cloak_and_Dagger:
			CloakDaggerSkill();
			break;
		case AbilityName::Bomb_trap:
			BombtrapSkill();
			break;
		case AbilityName::Dash_Attack:
			DashAttackSkill();
			break;
		default:
			break;
		}

		curTime = 0;
	}
	else
	{
		cout << "Skill is still on cooldown" << endl;
	}

	if (active &&( ability.name == AbilityName::Battle_Rally || 
		ability.name == AbilityName::Ironclad || 
		ability.name == AbilityName::Mana_Burst ||
		ability.name == AbilityName::Hunters_Mark ||
		ability.name == AbilityName::Shadow_Sneak))
	{
		timer.StartTimer();
		if (ability.name == AbilityName::Shadow_Sneak)
		{
			timer2.StartTimer();
		}
	}
}

void PlayerSkillNode::Update(float dt)
{
	if (bombObj.size() > 0 && ability.name == AbilityName::Bomb_trap)
	{
		BombExplode();
	}

	if (active && ability.name == AbilityName::Two_Handed_Strike && !hitboxCol->GetActive())
	{
		EnableHitboxAfterDelay(dt);
	}

	//NOTE: don't check active here because inactive skill may still have active hitbox
	if (hitboxCol->GetActive())
	{
		UpdateHitboxPosition();
		//flicking hitbox is deactived in diff. way
		if (ability.name != AbilityName::Blizzard && ability.name != AbilityName::Arrow_Volley)
		{
			DisableHitbox(dt);
		}
	}

	if (active && ability.name == AbilityName::Cloak_and_Dagger)
	{
		DisableCloak(dt);
		DaggerSkill();
	}
}

void PlayerSkillNode::PostUpdate(float dt)
{
	for (unsigned i = 0; i < targets.size();) {
		if (!targets[i] || !targets[i]->getAlive() || !targets[i]->owner->GetIsAlive()) {
			targets[i] = targets.back();
			targets.pop_back();
		}
		else {
			++i;
		}
	}
	if (active && (ability.name == AbilityName::Battle_Rally ||
		ability.name == AbilityName::Ironclad ||
		ability.name == AbilityName::Mana_Burst ||
		ability.name == AbilityName::Hunters_Mark ||
		ability.name == AbilityName::Shadow_Sneak))
	{
		DebuffStatsTeam(ability.name);
		if (ability.name == AbilityName::Shadow_Sneak)
		{
			DebuffAutoAttackTeam();
		}
	}

	if (active && (ability.name == AbilityName::Blizzard || ability.name == AbilityName::Arrow_Volley))
	{
		FlickingHitbox();
	}

	if (active && ability.name == AbilityName::Shield_Bash)
	{
		if (!target || !target->getAlive() || !target->owner->GetIsAlive())
		{
			active = false;
		}
		else
		{
			float3 dir = (target->owner->GetPosition() - player->owner->GetPosition()).Normalize();
			MoveObjectOverTime(target->owner, target->owner->GetPosition() + dir * 1.5f, 200.0f, 0.1f*ability.level);
		}
	}

	if (active && ability.name == AbilityName::Dash_Attack)
	{
		if (!target || !target->getAlive() || !target->owner->GetIsAlive())
		{
			active = false;
		}
		else
		{
			MoveObjectOverTime(player->owner, target->owner->GetPosition(), 500.0f, 0.1f);
		}
	}

	if (active && ability.name == AbilityName::Arrow_Volley)
	{
		MoveGroupTargetOverTime(aimPos, 0.4f, dt);
	}
}




SkillTreePlayer::SkillTreePlayer(LeComp_MOB * _player, PlayerAbilities* abi)
{
	player = _player;
	ability = abi;
}

SkillTreePlayer::SkillTreePlayer(SkillTreePlayer & other)
{
	skills.resize(other.skills.size());
	for (unsigned int i = 0; i < skills.size(); i++)
	{
		skills[i] = other.skills[i];
	}
	player = other.player;
	ability = other.ability;
	for (int i = 0; i < 3; i++)
	{
		team[i] = other.team[i];
	}
}

SkillTreePlayer::~SkillTreePlayer()
{
	for (unsigned int i = 0; i < skills.size(); i++)
	{
		delete skills[i];
	}
	skills.clear();
}

void SkillTreePlayer::ActiveSkill(AbilityName skillName, LeComp_MOB* skillTarget, float3 _aimPos)
{
	//disable IronClad when other skill is active
	if ((AbilityName::Battle_Rally == skillName
		|| AbilityName::Cleave == skillName
		|| AbilityName::Two_Handed_Strike == skillName
		|| AbilityName::Shield_Bash == skillName)
		&& skills[2]->IsActive()
		)
	{
		skills[2]->DisableBuffEarly();
	}
	
	//pass both vars but don't need to use both, some skills use 0 or 1 vars
	switch (skillName)
	{
	case AbilityName::Null:
		break;
	case AbilityName::Battle_Rally: //buff
		skills[0]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Cleave:
		skills[1]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Ironclad:
		skills[2]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Two_Handed_Strike:
		skills[3]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Shield_Bash:
		skills[4]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Mana_Burst: //buff
		skills[0]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Restoration:
		skills[1]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Fireball:
		skills[2]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Lightning_Bolt:
		skills[3]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Blizzard:
		skills[4]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Hunters_Mark: //buff
		skills[0]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Entangle:
		skills[1]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Multishot:
		skills[2]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Arrow_Volley:
		skills[3]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Poison_Shot:
		skills[4]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Shadow_Sneak: //buff
		skills[0]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Flurry:
		skills[1]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Cloak_and_Dagger:
		skills[2]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Bomb_trap:
		skills[3]->ActiveSkill(skillTarget, _aimPos);
		break;
	case AbilityName::Dash_Attack:
		skills[4]->ActiveSkill(skillTarget, _aimPos);
		break;
	default:
		break;
	}
}

void SkillTreePlayer::Update(float dt)
{
	for (int i = 0; i < skills.size(); i++)
	{
		skills[i]->Update(dt);
	}
}

void SkillTreePlayer::PostUpdate(float dt)
{
	for (int i = 0; i < skills.size(); i++)
	{
		skills[i]->PostUpdate(dt);
	}
}

Object * SkillTreePlayer::GetTeamMember(int i) const
{
	if ((i<3 && i >= 0) && team[i])
	{
		return team[i]->owner;
	}
	else
	{
		return nullptr;
	}
}

void SkillTreePlayer::SetTeam(Object * _team[4])
{
	int count = 0;
	string playerName = player->owner->GetName();
	string memberName = "/0";
	for (int i = 0; i < 4; i++)
	{
		if (_team[i])
		{
			memberName = _team[i]->GetName();
			//if not null & not player -> add in team
			if (playerName != memberName)
			{
				team[count++] = _team[i]->GetComponent<LeComp_MOB>();
			}
		}
	}	

	for (int i = 0; i < 5; i++)
	{
		//TODO: change the valid value later
		int unlockPoints[5] = { 0, 5, 10, 15, 25 }; //this is an example
		PlayerSkillNode *skill = new PlayerSkillNode(player, unlockPoints, ability->GetActiveAbility(i), team, stickDir);
		skills.push_back(skill);
	}
}

SkillTreePlayer & SkillTreePlayer::operator=(SkillTreePlayer & other)
{
	skills.resize(other.skills.size());
	for (unsigned int i = 0; i < skills.size(); i++)
	{
		skills[i] = new PlayerSkillNode(*other.skills[i]);
	}
	player = other.player;
	ability = other.ability;

	LeComp_MOB* tempMember = nullptr;
	for (int i = 0; i < 3; i++)
	{
		tempMember = other.team[i];
		team[i] = tempMember;
		//team[i] = other.team[i];
	}
	return *this;
}



SkillTreeEnemy::SkillTreeEnemy(LeComp_MOB * _ai)
{
	ai = _ai;
	numPE = 0;

}

SkillTreeEnemy::SkillTreeEnemy(SkillTreeEnemy & other)
{
	//skills.resize(other.skills.size());
	//for (int i = 0; i < skills.size(); i++)
	//	skills[i] = other.skills[i];
	ai = other.ai;
}

SkillTreeEnemy & SkillTreeEnemy::operator=(SkillTreeEnemy & other)
{
	//skills.resize(other.skills.size());
	//for (int i = 0; i < skills.size(); i++)
	//	skills[i] = other.skills[i];
	ai = other.ai;
	return *this;
}

SkillTreeEnemy::~SkillTreeEnemy()
{
	//for (int i = 0; i < skills.size(); i++)
	//{
	//	delete skills[i];
	//}5
	//skills.clear();
}

void SkillTreeEnemy::ActiveSkill(DamageType damageType)
{
	switch (damageType)
	{
	case Melee:
		break;
	case Ranged:
		Shot();
		break;
	case Magic:
		break;
	default:
		break;
	}
}

void SkillTreeEnemy::Shot()
{
	if (ai->GetTarget())
	{
		Object *particleSys = ai->owner->owningGameState->AddObject(ai->owner->GetName() + "Arrow" + std::to_string(ai->owner->GetPlayerNumber()) + std::to_string(numPE++));
		PE_Shot *peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, ai->owner->GetTransform()->position, ai->GetTarget()->GetPosition()));
		peShot->SetMaterial(ai->owner->owningGameState->Renderer->GetMaterial("arrow"));
		peShot->SetOffset({ 0, 0.2f, 0 });
		peShot->InitializeParticle(LeRenderer::device);
	}
	//SphereCollider *collider = particleSys->AddComponent<SphereCollider>();
	//collider->Init(peShot->GetSize().x);
	////make collision don't push other things
	//collider->isTrigger = true; 
}




//void SkillTreeFamiliar::Heal()
//{
//	if (deltaHealTimer.GetCurTimeSecond() > deltaTime)
//	{
//		if (player->getAlive() && player->getCurHealth() < player->getMaxHealth()*0.7f)
//		{
//			Object *obj = player->owner->owningGameState->AddObject(player->owner->GetName() + std::string("PE Heal") + std::to_string(numPE++));
//			PE_Respawn *pe = obj->AddComponent<PE_Respawn>();
//			pe->SetMaterial(player->owner->owningGameState->Renderer->GetMaterial("cross"));
//			pe->SetRandomColor(false);
//			pe->SetEmissionRate(0.3f);
//			pe->SetAliveTimePS(1.2f);
//			pe->SetDirection(2); //+y
//			pe->SetTransparent(0.7f);
//			pe->SetMovingSpeed(5.0f);
//			pe->SetFixedColor({1, 0, 0});
//			pe->SetMinMaxTextureSize({ 0.3f, 0.3f }, {0.3f, 0.3f});
//			
//			pe->SetCanRotate(false);
//			pe->InitializeParticle(LeRenderer::device);
//
//			obj->SetPosition(player->owner->GetPosition());
//
//			//use Heal skill
//			int hp = (int)(player->getMaxHealth()*0.1f);
//			player->RestoreHP(hp);
//
//
//			deltaHealTimer.StartTimer();
//		}
//	}
//}

void SkillTreeFamiliar::AttackTarget()
{
	if (deltaAtkTimer.GetCurTimeSecond() > deltaTime)
	{
		LeComp_MOB *target = player->GetTarget();
		if (target)
		{
			float3 famPos = fam->GetPos();
			float3 targetPos = target->owner->GetPosition();
			float dis = famPos.GetDistance(targetPos);

			Object *particleSys = fam->owner->owningGameState->AddObject(fam->owner->GetName() + "Familiar Fireball" + std::to_string(numPE++));
			PE_Shot *peShot = particleSys->AddComponent<PE_Shot>(new PE_Shot(particleSys, famPos, targetPos, AbilityName::Null, true, (LeComp_MOB*)fam));
			peShot->SetMaterial(fam->owner->owningGameState->Renderer->GetMaterial("fireball1"));
			peShot->SetMaxDistance(dis);
			peShot->SetFixedColor(float3{ 1.0f, 1.0f, 1.0f });
			peShot->SetMinMaxTextureSize({ 0, 0 }, {0.5f, 0.5f});
			peShot->SetTransparent(0.5f);
			peShot->InitializeParticle(LeRenderer::device);

			deltaAtkTimer.StartTimer();
		}
	}
}

void SkillTreeFamiliar::RecoverMp()
{
	if (deltaMpTimer.GetCurTimeSecond() > deltaTime)
	{
		player->RestoreMP((int)(player->getMaxMP() * 0.05f));
		deltaMpTimer.StartTimer();
	}
}

void SkillTreeFamiliar::RecoverHp()
{
	if (deltaHpTimer.GetCurTimeSecond() > deltaTime)
	{
		player->RestoreHP((int)(player->getMaxHealth() * 0.05f));
		deltaHpTimer.StartTimer();
	}
}

void SkillTreeFamiliar::StealMoney()
{
	if (deltaMoneyTimer.GetCurTimeSecond() > deltaTime)
	{
		player->AddGP(1);
		deltaMoneyTimer.StartTimer();
	}
}

SkillTreeFamiliar::SkillTreeFamiliar(LeComp_BattleMageBehavior * _player, LeComp_Familiar *_fam)
{
	player = _player;
	fam = _fam;
	numPE = 0;
	deltaHealTimer.StartTimer();
	deltaAtkTimer.StartTimer();
	deltaMpTimer.StartTimer();
	deltaHpTimer.StartTimer();
	deltaMoneyTimer.StartTimer();
}

