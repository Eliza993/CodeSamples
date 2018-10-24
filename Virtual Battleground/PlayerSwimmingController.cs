using System.Collections;
using System.Collections.Generic;
using CodeStage.AntiCheat.ObscuredTypes;
using UnityEngine;

namespace Oneiric.Shooter
{
	public class PlayerSwimmingController : PlayerBase
	{
		[SerializeField] private ObscuredFloat maxSpeed = 15.0f;
		[SerializeField] private ObscuredFloat maxUp = 5.0f; // higher means faster each move
		[SerializeField] private ObscuredFloat downSpeed = 15.0f; //reduce over time
		[SerializeField] private ObscuredFloat divideMagnitude = 6.0f; //make magnitude smaller, original value is too large

		private ObscuredFloat curSpeed = 0.0f;
		private ObscuredFloat upSpeed = 0.0f; //base on how fast arms move

		[SerializeField]
		private PlayerControllerValues controller; //use PlayerController

		[SerializeField]
		private Transform eye; //eye inside OVR cam

		private float timeL = 0.0f;
		private float timeR = 0.0f;
		private float disL = 0.0f;
		private float disR = 0.0f;

		// Update is called once per frame
		public void UpdateSwim()
		{
			if (playerController.inWater)
			{
				if (RightHandSwim() || LeftHandSwim())
				{
					curSpeed += upSpeed;
					curSpeed = Mathf.Clamp(curSpeed, 0, maxSpeed);
					//Debug.Log("Swim speed: " + curSpeed);

				}
				else
				{
					SpeedReduceOverTime();
				}

				if (curSpeed > 0.0f)
				{
					Vector3 heading = eye.forward * curSpeed;
					playerController.referenceObjects.characterController.Move(heading * playerController.enhancedSpeed * Time.deltaTime);
				}
				SwimGravity();

				if (playerController.aboardVehicle)
				{
					playerController.DismountVehicle();
				}
			}
		}

		#region Check movement of right/left controller
		//NOTE: may need to flip the dir if the set up is different
		private bool LeftHandSwim()
		{
			Vector3 angVel = controller.GetAngularVel(true);
			Vector3 velL = controller.GetVelWithCamRotation(true);
			float angleDown = Vector3.Angle(velL, playerController.referenceObjects.animatorMesh.transform.up);
			float angleBack = Vector3.Angle(velL, playerController.referenceObjects.animatorMesh.transform.forward);

			if (angVel.magnitude > 0.5f && angleDown > 60.0f && angleBack > 30.0f)
			{
				timeL += Time.deltaTime;
				disL += (angVel.magnitude / divideMagnitude) * Time.deltaTime;
				upSpeed = disL * disL / timeL;
				upSpeed = Mathf.Clamp(upSpeed, 0.0f, maxUp);
				//Debug.Log(disL + " " + timeL + " " + upSpeed);
				return true;
			}
			timeL = 0.0f;
			disL = 0.0f;
			return false;
		}

		private bool RightHandSwim()
		{
			Vector3 angVel = controller.GetAngularVel(false);
			Vector3 velR = controller.GetVelWithCamRotation(false);
			float angleDown = Vector3.Angle(velR, playerController.referenceObjects.animatorMesh.transform.up);
			float angleBack = Vector3.Angle(velR, playerController.referenceObjects.animatorMesh.transform.forward);

			if (angVel.magnitude > 0.5f && angleDown > 60.0f && angleBack > 30.0f)
			{
				timeR += Time.deltaTime;
				disR += (angVel.magnitude / divideMagnitude) * Time.deltaTime;
				upSpeed = disR * disR / timeR;
				upSpeed = Mathf.Clamp(upSpeed, 0.0f, maxUp);
				//Debug.Log(disR + " " + timeR + " " + upSpeed);
				return true;
			}
			timeR = 0.0f;
			disR = 0.0f;
			return false;
		}
		#endregion

		public void SetUnderWater(bool _underWater)
		{

			if (playerController.playerState == PlayerController.PlayerState.Climbing)
				return;


			if (playerController.inWater)
			{
				playerController.playerState = PlayerController.PlayerState.Swimming;
				playerController.ignoreGravity = true;
			}
			else
			{
				ResetValues();

				playerController.playerState = PlayerController.PlayerState.Standing;
				playerController.ignoreGravity = false;
			}
		}

		private void SpeedReduceOverTime()
		{
			curSpeed -= Time.deltaTime * downSpeed;
			if (curSpeed < 0.0f)
			{
				curSpeed = 0;
			}
		}

		//make player float on the water
		public void SwimGravity()
		{
			if (playerController.characterGravity.y < -.4f)
			{
				playerController.characterGravity = new Vector3(0, (playerController.characterGravity.y + (4 * Time.deltaTime)), 0);
			}
			this.playerController.referenceObjects.characterController.Move(playerController.characterGravity * Time.deltaTime);
		}

		private void ResetValues()
		{
			curSpeed = 0.0f;
			upSpeed = 0.0f;
			timeL = 0.0f;
			timeR = 0.0f;
			disL = 0.0f;
			disR = 0.0f;
		}
	}
}
