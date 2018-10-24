using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//grab ziplining by Left or Right hand using Grip
//use same hand to control ziplining, other hand can use gun
//Grip again to drop off
//press trigger to flip forward/backward

namespace Oneiric.Shooter
{
	public class PlayerZiplining : PlayerBase
	{
		[SerializeField] private CharacterController chaController;
		[SerializeField] private LayerMask zipliningMask;
		[SerializeField] private PlayerControllerValues controller;
		[SerializeField] private float sphereRadius = 0.1f;

		[SerializeField]
		private GameObject zipliningCheck;
		[SerializeField]
		private ZipliningBorderCheck zipliningScript;

		private bool useLeftHand = false;

		private float curTime = 0.0f;
		private void Update()
		{
			// added null reference check to avoid errors
			if (zipliningCheck != null && zipliningCheck.GetActive() == false)
			{
				curTime += Time.deltaTime;
				if (curTime >= 0.3f)
				{
					if (controller.GetControlerDevice(true).GetPressDown(SteamVR_Controller.ButtonMask.Grip)
					&& playerController.leftHandState == PlayerController.LeftHandState.Open)
					{
						Collider[] col = Physics.OverlapSphere(controller.GetLeftPos(), sphereRadius, zipliningMask);
						if (col.Length > 0)
						{
							RelocateBorderCheck(controller.GetLeftPos());
							playerController.ClimbingToZiplining(false); //make other hand stop climbing
							playerController.leftHandState = PlayerController.LeftHandState.Ziplining;
							useLeftHand = true;
						}
					}
					else if (controller.GetControlerDevice(false).GetPressDown(SteamVR_Controller.ButtonMask.Grip)
						&& playerController.rightHandState == PlayerController.RightHandState.Open)
					{
						Collider[] col = Physics.OverlapSphere(controller.GetRightPos(), sphereRadius, zipliningMask);
						if (col.Length > 0)
						{
							RelocateBorderCheck(controller.GetRightPos());
							playerController.ClimbingToZiplining(true);
							playerController.rightHandState = PlayerController.RightHandState.Ziplining;
							useLeftHand = false;
						}
					}
				}
			}

			if (zipliningCheck != null && zipliningCheck.GetActive())
			{
				curTime += Time.deltaTime;

				if (useLeftHand)
				{
					if (controller.GetControlerDevice(true).GetPressDown(SteamVR_Controller.ButtonMask.Trigger))
					{
						zipliningScript.moveForward = !zipliningScript.moveForward;
					}

					//check time to avoid ziplining and drop down at the same time due to PressDown
					if (curTime >= 0.5f)
					{
						if (controller.GetControlerDevice(true).GetPressDown(SteamVR_Controller.ButtonMask.Grip)
							|| zipliningScript.getTarget)
						{
							GetOff();
						}
					}
				}
				else
				{
					if (controller.GetControlerDevice(false).GetPressDown(SteamVR_Controller.ButtonMask.Trigger))
					{
						zipliningScript.moveForward = !zipliningScript.moveForward;
					}

					if (curTime >= 0.5f)
					{
						if (controller.GetControlerDevice(false).GetPressDown(SteamVR_Controller.ButtonMask.Grip)
							|| zipliningScript.getTarget)
						{
							GetOff();
						}
					}
				}
			}
		}

		private void RelocateBorderCheck(Vector3 pos)
		{
			zipliningCheck.transform.position = pos;
			zipliningScript.usingLeftHand = useLeftHand;
			zipliningCheck.SetActive(true);

			playerController.ignoreGravity = true;
			playerController.playerState = PlayerController.PlayerState.Standing;
			curTime = 0.0f;
		}

		private void GetOff()
		{
			playerController.playerState = PlayerController.PlayerState.Falling;
			playerController.ignoreGravity = false;
			zipliningCheck.SetActive(false);
			curTime = 0.0f;

			if (useLeftHand)
			{
				playerController.leftHandState = PlayerController.LeftHandState.Open;
			}
			else
			{
				playerController.rightHandState = PlayerController.RightHandState.Open;
			}
		}
	}
}