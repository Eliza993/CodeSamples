#include "stdafx.h"
#include "PE_Respawn.h"

void PE_Respawn::UpdateParticle(float time)
{
	for (auto iter = particleList.begin(); iter != particleList.end(); iter++)
	{
		Particle *temp = iter._Ptr->_Myval;
		temp->age += time;
		if (temp->age > temp->aliveTime)
		{
			temp->isAlive = false;
		}
		if (direction == 2) //+y
		{
			temp->pos.y += temp->velocity.y*time;
		}
		else if (direction == 3) //-y
		{
			temp->pos.y -= temp->velocity.y*time;
		}

		if (canRotate)
		{
			temp->rotateZ += 200.0f*time;
			if (temp->rotateZ >= 360)
			{
				temp->rotateZ = 0;
			}
		}
	}
}