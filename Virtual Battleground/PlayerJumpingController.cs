using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using CodeStage.AntiCheat.ObscuredTypes;

//try to start jump by check hands
//once start, start measuring total Vel
//once hand get down near end, make player jump, speed based on total Vel
//reset

namespace Oneiric.Shooter
{
	public class PlayerJumpingController : PlayerBase
	{
		enum JumpState
		{
			NONE,
			PREPARE,
			JUMPING
		}

		[SerializeField] private ObscuredFloat startUpDis = 20.0f;
		[SerializeField] private ObscuredFloat speed = 7.0f;
		[SerializeField] private ObscuredFloat upMulDirStand = 3.0f;
		//[SerializeField] private ObscuredFloat upMulDirRun = 2.0f;
		[SerializeField] private ObscuredFloat forwardMulDirStand = 1.0f;
		[SerializeField] private ObscuredFloat forwardMulDirRun = 2.0f;
		[SerializeField] private ObscuredFloat reduceSpeed = 0.2f; //each frame
		[SerializeField] [Range(0, 1.0f)] private ObscuredFloat mulRunSpeed = 0.15f; //walk speed is about 8, only add partial, avoid jump too far

		[SerializeField] [Range(0, 3.0f)] private ObscuredFloat triggerHeight;// = 0.95f;
		[SerializeField] [Range(0, 3.0f)] private ObscuredFloat jumpHeight;// = 1.0f;
		[SerializeField] private ObscuredFloat minCamVel = 1.3f;

		private JumpState jumpState = JumpState.NONE;
		private ObscuredFloat startTime = 0.0f;
		private ObscuredFloat expireTime = 3.0f;

		[SerializeField]
		private PlayerControllerValues controller; //use PlayerController
		[SerializeField]
		private Transform cam; //OVR cam

		public void UpdateJump()
		{
			ActiveJump();
			CheckCancel();
			CheckHandFinish();
		}

		//active when 2 hands over head & infront of player & move down
		private void ActiveJump()
		{
			if (jumpState != JumpState.NONE)
				return;

			if (controller.GetHeadVel().y >= minCamVel && playerAnimationController.currentHeight >= triggerHeight)
			{
				jumpState = JumpState.PREPARE;
				startTime = Time.time;
			}
		}

		//true when hands are near player's back
		private bool CheckHandFinish()
		{
			if (jumpState != JumpState.PREPARE)
				return false;

			if (playerAnimationController.currentHeight >= jumpHeight)
			{
				startTime = Time.time - startTime;
				jumpState = JumpState.JUMPING;
				StartCoroutine("Jump");
				return true;
			}
			return false;
		}

		IEnumerator Jump()
		{
			float tempSpeed = speed;
			Vector3 jumpDir = Vector3.zero;
			Vector3 finalDir = Vector3.zero;

			if (playerController.currentVelocity == 0)
			{
				jumpDir = playerController.referenceObjects.animatorMesh.transform.forward; //stand still -> jump forward
				finalDir = jumpDir * forwardMulDirStand + Vector3.up * upMulDirStand;
			}
			else
			{
				jumpDir = playerController.newMovement; //jump to moving dir
				finalDir = jumpDir * forwardMulDirRun + Vector3.up * forwardMulDirRun;
				tempSpeed += (mulRunSpeed * playerController.finalSpeed); //add moving speed while running to jumping
			}

			int count = 5;
			RaycastHit hit;
			playerController.playerState = PlayerController.PlayerState.Jumping;
			//playerAnimationController.SwitchAnimation(PlayerAnimationController.AnimationState.Jumping);

			while (true)
			{
				//move character up 1st few frames to avoid jumping stops when colliding with uneven terrain
				if (count > 0)
				{
                    playerController.referenceObjects.characterController.Move(Vector3.up * startUpDis * Time.deltaTime);
					count--;
				}
				else
				{
                    playerController.referenceObjects.characterController.Move(finalDir * tempSpeed * Time.deltaTime);
					tempSpeed -= reduceSpeed;

					//stop jump when already near ground
					float offset = 0.1f; //ray cast won't work when it is inside collider
					if (Physics.Raycast(playerController.referenceObjects.animatorMesh.transform.position + new Vector3(0, offset, 0), Vector3.down, out hit, offset + 0.2f, playerController.layer))
					{
						//Debug.Log("Cancel jump, touch the ground");
						break;
					}
					//stop jump when hit wall
					if (Physics.Raycast(controller.GetHeadTransform().position, jumpDir, out hit, 0.5f, playerController.layer))
					{
						//Debug.Log("Cancel jump, hit the wall");
						break;
					}
				}

				if (tempSpeed <= 2.0f)
				{
					break;
				}

				yield return 0;
			}
			ResetJump();
			playerController.playerState = PlayerController.PlayerState.Standing;
		}

		private void CheckCancel()
		{
			if (jumpState != JumpState.PREPARE)
				return;

			float timePass = Time.time - startTime;
			if (timePass > expireTime)
			{
				ResetJump();
			}
			else if (controller.GetHeadVel().y <= -0.1f)
			{
				ResetJump();
			}
		}

		private void ResetJump()
		{
			jumpState = JumpState.NONE;
			startTime = 0.0f;
		}
	}
}
