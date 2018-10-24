#pragma once
#include <vector>
#include "./QueryPerformanceTimer.h"
#include <thread>

#include "PE_WarriorBuff.h"
#include "PE_Level.h"
#include "./PlayerAbilities.h"

//class LeComp_MOB;
//class TestGameState;
//class Collision;
class EnemyAI;
class LeComp_Familiar;
class LeComp_MOB;
class LeComp_Hitbox;
class Collider;
class SphereCollider;
class PE_Shot;
class LeComp_SkeletalMesh;
class LeComp_Socket;

using namespace std;

class PlayerSkillNode
{
	PE_Shot *peShot;
	//make sure buff min is 1
	void CapMin(int* x) {if (*x <= 0) *x = 1; }
	void CreateHitbox();
	void DisableHitbox(float dt);
	void EnableHitboxAfterDelay(float time);
	//move hitbox with peShot (arrow, ...)
	void UpdateHitboxPosition();
	
	//buff for each player in a wide radius
	void BuffStatsTeam(AbilityName skillName);
	void BuffAutoAttackTeam();
	//call when in buff time is over
	void DebuffStatsTeam(AbilityName skillName);
	void DebuffAutoAttackTeam();

	//move player/enemy to Target with Speed, time in milisecond
	void MoveObjectOverTime(Object* obj, float3 targetPos, float moveTimeMilli = 2000.0f, float speed = 0.02f);
	void MoveGroupTargetOverTime(float3 targetPos,float speed = 0.02f, float dt = 0);
	//helper f for Blizzard(), Vortex
	void FlickingHitbox();
	//toggle bool to avoid buff same skill twice by 2 players
	void ToggleBoolBuffSkill(LeComp_MOB* player, AbilityName skillName);	
	void ToggleBoolBuffSkillTeam(AbilityName skillName);	//use this to toggle whole team
	bool AlreadyBuff(LeComp_MOB* player, AbilityName skillName);

	//will explode bomb when E get close or time over
	void BombExplode();

	//Warrior
	void BattleRallySkill();	//buff
	void CleaveSkill();
	void IroncladSkill();	//buff
	void TwoHandedStrikeSkill();
	void ShieldBashSkill();
	//Mage
	void ManaBurstSkill();	//buff
	void RestorationSkill();
	void FireballSkill();
	void LightningBolt();
	void BlizzardSkill();
	//Ranger
	void HuntersMarkSkill();	//buff
	void EntangleSkill();
	void MultishotSkill();		//only skill create new Hitbox for arrow
	void ArrowVolleySkill();
	void PoisonShotSkill();	
	//Rogue
	void ShadowSneakSkill();	//buff
	void FlurrySkill();
	void CloakDaggerSkill();
	void DisableCloak(float dt);
	void DaggerSkill();	//2nd stage of CloakDagger, can use if Cloak is still active, will cancel out Cloak
	void BombtrapSkill();
	void DashAttackSkill();

	Ability ability;
	LeComp_MOB *player;
	LeComp_MOB *team[3] = {nullptr};
	LeComp_MOB *target;
	//within radius
	vector<EnemyAI *>targets; 

	LeComp_Hitbox *hitbox;
	Collider *hitboxCol;
	float hitboxRad;

	LeComp_SkeletalMesh *mesh = nullptr;
	LeComp_Socket *swordTip = nullptr;
	LeComp_Socket *swordEnd = nullptr;
	float3 midSword;

	ControllerAxis stickDir;

	int maxLevel = 5;

	bool buffMember[3] = {0};
	float defaultStat[4] = {0}; //player is index 3, team is 0-2
	int unlockPoints[5]; //max lv, and point to unlock skill
	Stats buff; //only need this if it is Buff skill
	float buffTime; //use to Debuff in Seconds
	float buffDis; //compare with dis. from player to team
	float3 aimPos; //for Vortex skill

	QueryPerformanceTimer timer; //skill run time
	QueryPerformanceTimer timer2; //skill run time
	QueryPerformanceTimer deltaTimer;
	QueryPerformanceTimer deltaTimerMove;

	//use to enable/disable Hitbox
	float activeTime;
	float curTime;
	float curDelayHBTime;
	float delayHitboxTime;

	vector<QueryPerformanceTimer> bombTimer;	//for bombtrap skill
	//way1: doesnt work well with cur engine	 vector<Object *>bombObj;	
	//way2: store string and call GetObjectByName
	vector<string>bombObj;

	float hbDeltaTime;
	QueryPerformanceTimer restTime; //cooldown
	//vector<PlayerSkillNode*> childs; // use this when Skill are linked
	static int numPE; //use to create unique name
	vector<string> enumString;
	vector<string> statusString;
	bool active = false;

public:
	//TODO: remove Control if dont use it
	PlayerSkillNode(LeComp_MOB *p, int _unlockPoints[5], Ability _ability, LeComp_MOB *_team[3], ControllerAxis _stickDir);
	~PlayerSkillNode();

	bool IsActive() { return active; }

	void DisableBuffEarly();

	//choose skill function
	void ActiveSkill(LeComp_MOB* skillTarget = nullptr, float3 _aimPos = { 0, 0, 0 });
	void Update(float dt);
	void PostUpdate(float dt);	
};

class SkillTreePlayer //: public SkillTree
{
	LeComp_MOB *player;
	LeComp_MOB *team[3] = {nullptr};
	PlayerAbilities* ability;
	//all Buff in 1st slot
	vector<PlayerSkillNode*> skills;
	ControllerAxis stickDir;

public:
	SkillTreePlayer() {};
	SkillTreePlayer(LeComp_MOB *_player, PlayerAbilities* abi);

	//make sure to copy pointer in
	SkillTreePlayer(SkillTreePlayer&);
	SkillTreePlayer& operator=(SkillTreePlayer&);
	~SkillTreePlayer();

	//check which skill in [] to active
	void ActiveSkill(AbilityName skillName, LeComp_MOB* skillTarget = nullptr, float3 _aimPos = {0, 0, 0});
	void Update(float dt);
	void PostUpdate(float dt);
	//Object* GetTeamMember(int i) const { return (i<3 && i >= 0) ? (Object*)team[i] : nullptr; }
	Object* GetTeamMember(int i) const; /*{ return (i<3 && i>=0)?(team[i]->owner):nullptr; }*/
	//have to SetTeam() before can use any skill
	void SetTeam(Object *_team[4]); //add skills in after have team
	void SetStickDir(ControllerAxis _stick) { stickDir = _stick; }

};


//this keep skills for all Enemy instead of each class like with Player
class SkillTreeEnemy //: public SkillTree
{
	//vector<PlayerSkillNode*> skills;
	LeComp_MOB *ai;
	unsigned int numPE = 0; //use to create unique name
							//vector<SphereCollider*> skillCol;
	void Shot();

public:
	SkillTreeEnemy() {};
	SkillTreeEnemy(LeComp_MOB *_ai);
	//make sure to copy pointer in
	SkillTreeEnemy(SkillTreeEnemy&);
	SkillTreeEnemy& operator=(SkillTreeEnemy&);
	~SkillTreeEnemy();

	void ActiveSkill(DamageType damageType);
};


class SkillTreeFamiliar //: public SkillTree
{
	LeComp_BattleMageBehavior *player;
	LeComp_Familiar *fam;

	//use milliSecond
	QueryPerformanceTimer deltaHealTimer;
	QueryPerformanceTimer deltaAtkTimer;
	QueryPerformanceTimer deltaMpTimer;
	QueryPerformanceTimer deltaHpTimer;
	QueryPerformanceTimer deltaMoneyTimer;
	float deltaTime = 10.0f;

	int numPE;

	PE_Shot *peShot;
	Collider *hitboxCol;

public:
	SkillTreeFamiliar() {};
	SkillTreeFamiliar(LeComp_BattleMageBehavior *_player, LeComp_Familiar *_fam);
	~SkillTreeFamiliar() {};

	//Heal Player when Hp is too low
	//void Heal();

	//attack cur target
	void AttackTarget();
	void RecoverMp();
	void RecoverHp();
	void StealMoney();
};