using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using CodeStage.AntiCheat.ObscuredTypes;


//hold on wall when Sphere Col on hand touch wall
//get Wall normal, hit pos, hand start pos (normally inside wall)
//always can move to side, only move for/backward when hand touch wall
//can use wallCollider.raycast from outside (- offset) to cal dis hand to wall, in/outside

namespace Oneiric.Shooter
{
	public class PlayerClimbingController : PlayerBase
	{
		//hand with priority can controll body
		enum HandPriority
		{
			NONE,
			LEFT,
			RIGHT
		}

		private HandPriority handPriority = HandPriority.NONE;
		private ObscuredFloat grabSize = 0.36f;
		private bool lHandHolding = false;
		private bool rHandHolding = false;
#pragma warning disable 414
		private bool pull = false;
		private bool push = false;
#pragma warning restore 414

		private Vector3 controllerVel = Vector3.zero;
		public ObscuredFloat speed = 2.5f;

		private RaycastHit hitL;
		private RaycastHit hitR;

		[SerializeField] private Transform cam; //use OVR, player moves base on Cam's rotation
		[SerializeField] private PlayerControllerValues controller; //PlayerController
		[SerializeField] private Transform genAssault_LeftHand;
		[SerializeField] private Transform genAssault_RightHand;

		public LayerMask climbingMask;

		//for final movement after removing both hands off the wall
		[SerializeField] private LayerMask dropDownLayer;
		[SerializeField] private float extraFinalUp = 1.0f; //make more up a little easier
		[SerializeField] private ObscuredFloat finalSpeed = 10.0f;
		[SerializeField] [Range(0.1f, 2.0f)] private float finalSpeedReduce = 0.05f;
		private float timeL;
		private float timeR;
		private Vector3 startPosL;
		private Vector3 startPosR;
		private Vector3 finalDirL;
		private Vector3 finalDirR;
		private bool finalMove = true;

		//Checking to Grab, is called when Grip is pressed
		public bool AttemptToGrab(bool leftHand)
		{
			bool grabbedSomething = false;

			Collider[] hitObjects = new Collider[0];
			RaycastHit hit;
			float tempGrabSize = grabSize;

			//makes it easier to climb when already climbing
			if (playerController.playerState == PlayerController.PlayerState.Climbing)
			{
				tempGrabSize *= 1.5f;
			}
			if (leftHand)
			{
				hitObjects = Physics.OverlapSphere(ReferenceManager.Instance.LocalVRPlayerLeftHand.transform.position, tempGrabSize, climbingMask);
			}
			else
			{
				hitObjects = Physics.OverlapSphere(ReferenceManager.Instance.LocalVRPlayerRightHand.transform.position, tempGrabSize, climbingMask);
			}

			for (int i = 0; i < hitObjects.Length; i++)
			{
				if (leftHand)
				{
					if (Debug.isDebugBuild)
					{
						Debug.Log("HEY: " + ReferenceManager.Instance.LocalVRPlayerLeftHand.transform + " YO: " + playerController.referenceObjects.animatorMesh.transform);
					}

					//use Collider raycast to get Wall normal
					Ray ray = new Ray(genAssault_LeftHand.position - genAssault_LeftHand.right * tempGrabSize * 2.0f, genAssault_LeftHand.right);
					if (hitObjects[i].Raycast(ray, out hit, 2.0f))
					{
						hitL = hit;
						lHandHolding = true;
						handPriority = HandPriority.LEFT;
						playerController.leftHandState = PlayerController.LeftHandState.Climbing;
						ResetFinalForce(true);
						SetToClimbing();
						grabbedSomething = true;
						break;
					}
				}
				else
				{
					//left and right hand's Right directions are opposite
					Ray ray = new Ray(genAssault_RightHand.position + genAssault_RightHand.right * tempGrabSize * 2.0f, -genAssault_RightHand.right);
					if (hitObjects[i].Raycast(ray, out hit, 2.0f))
					{
						hitR = hit;
						rHandHolding = true;
						handPriority = HandPriority.RIGHT;
						playerController.rightHandState = PlayerController.RightHandState.Climbing;
						ResetFinalForce(false);
						SetToClimbing();
						grabbedSomething = true;
						break;
					}
				}
			}
			//if (!grabbedSomething)
			//{
			//	Debug.Log(hitObjects.Length + " get wall normal failed " + playerController.playerState);
			//}
			return grabbedSomething;
		}

		private void SetToClimbing()
		{
			//playerController.ignoreGravity = true;

			playerController.playerState = PlayerController.PlayerState.Climbing;
			//remove offset
			playerController.referenceObjects.footFollow.target = ReferenceManager.Instance.LocalVRPlayerCamera.transform;  //playerAnimationController.vrIK.transform;

			//	playerController.referenceObjects.footFollow.lerpToPos = true;
		}

		private void StopClimbing()
		{
			playerController.playerState = PlayerController.PlayerState.Falling;
			playerController.referenceObjects.footFollow.target = playerAnimationController.vrIK.transform;
			//remove offset
			playerController.referenceObjects.footFollow.offset = new Vector3(0.0f, 0.4f, 0.0f);
			//playerController.referenceObjects.footFollow.lerpToPos = true;
		}

		IEnumerator PushUpPlayerWhenfinish()
		{
			playerController.ignoreGravity = true;

			Debug.Log("PushUpPlayerWhenfinish");
			RaycastHit hitdown;
			float playerHeight = Vector3.Distance(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, playerController.referenceObjects.animatorMesh.transform.position) + 0.01f;
            float curTime = 0.0f;
			while (Physics.Raycast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, Vector3.down, out hitdown, playerHeight, climbingMask))
			{
                curTime += Time.deltaTime;
				float moveDis = playerHeight - hitdown.distance;
				playerController.referenceObjects.characterController.Move(Vector3.up * moveDis * 30.0f * Time.deltaTime);
				//Debug.Log(playerHeight + " " + moveDis);
				if (moveDis <= 0.01f && curTime > 2.0f)
				{
					break;
				}
				yield return 1;
			}

			playerController.referenceObjects.footCollider.transform.position = playerAnimationController.vrIK.transform.position + new Vector3(0.0f, 0.4f, 0.0f);
			playerController.ignoreGravity = false;

			StopClimbing();
			yield return 0;
		}

		#region Happen after player remove both hands from the wall, move the player 1 last time, speed/dir depend on hand's vel before remove
		private void UpdateFinalForce()
		{
			if (lHandHolding)
			{
				timeL += Time.deltaTime;
				float angleL = Vector3.Angle(finalDirL, controller.GetVelWithCamRotation(true).normalized);
				if (angleL > 90.0f)
				{
					ResetFinalForce(true);
				}
			}

			if (rHandHolding)
			{
				timeR += Time.deltaTime;
				float angleR = Vector3.Angle(finalDirR, controller.GetVelWithCamRotation(false).normalized);
				if (angleR > 90.0f)
				{
					ResetFinalForce(false);
				}
			}
		}

		private void ResetFinalForce(bool left)
		{
			if (left)
			{
				timeL = 0.0f;
				startPosL = controller.GetLeftTransform().position;
				finalDirL = controller.GetVelWithCamRotation(true).normalized;
			}
			else
			{
				timeR = 0.0f;
				startPosR = controller.GetRightTransform().position;
				finalDirR = controller.GetVelWithCamRotation(false).normalized;
			}
		}
        
		IEnumerator LerpFinalMove(bool left)
		{
            if (finalSpeedReduce == 0.0f)
            {
                finalSpeedReduce = 0.5f; //safety check
            }

			bool movedFootColliderDown = false;
			Vector3 startPos;
			Vector3 curPos;
			Vector3 finalDir;
			Vector3 wallNormal;
			float time;
			float playerHeight = Vector3.Distance(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, playerController.referenceObjects.animatorMesh.transform.position);

			if (left)
			{
				startPos = startPosL;
				curPos = controller.GetLeftTransform().position;
				finalDir = -controller.GetVelWithCamRotation(true).normalized;
				wallNormal = hitL.normal;
				time = Mathf.Clamp(timeL, 0.1f, 1.0f);
			}
			else
			{
				startPos = startPosR;
				curPos = controller.GetRightTransform().position;
				finalDir = -controller.GetVelWithCamRotation(false).normalized;
				wallNormal = hitR.normal;
				time = Mathf.Clamp(timeR, 0.1f, 1.0f);
			}

			float dis = Vector3.Distance(startPos, curPos);

			//make go up easier
			if (Vector3.Angle(finalDir, Vector3.up) < 90.0f)
			{
				finalDir += new Vector3(0, extraFinalUp, 0);
			}

			//if dir come into wall
			if (Vector3.Angle(finalDir, wallNormal) >= 92.0f)
			{
				finalDir = Vector3.Project(finalDir, wallNormal);
			}

			if (Physics.Raycast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, Vector3.down, playerHeight, climbingMask))
			{
				if (finalDir.y < 0.0f)
				{
					finalDir.y = 0.0f;
				}
			}
			else
			{
				playerController.referenceObjects.footCollider.transform.position = playerAnimationController.vrIK.transform.position + new Vector3(0.0f, 0.4f, 0.0f);
				StopClimbing();
				movedFootColliderDown = true;
				Debug.Log("On air, moved footCol down");
			}

			float tempSpeed = (dis / time) * finalSpeed;
			tempSpeed = Mathf.Clamp(tempSpeed, 0.0f, finalSpeed);
			float curColTimer = 0.0f;

			while (tempSpeed > 1.0f)
			{
				Vector3 tempDir = finalDir;

				curColTimer += Time.deltaTime;

                //this remove drag, however, it make push more difficult if doesnt give enought time for push up footCol
                if (curColTimer >= 0.5f) 
                {
                    if (Physics.Raycast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, Vector3.down, playerHeight + 0.1f, climbingMask) || curColTimer >= 3.0f)
                    {
                        Debug.Log("Cancel early ");
                        break;
                    }
                }
                //if (curColTimer >= 3.0f)
                //{
                //    Debug.Log("Cancel early ");
                //    break;
                //}

                //combine 2 forces if half player body was under the ground
                if (!movedFootColliderDown)
                {
                    if (Physics.Raycast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, Vector3.down, playerHeight, climbingMask))
                    {
                        tempDir += new Vector3(0.0f, extraFinalUp, 0.0f);
                        Debug.Log("Combine force");
                    }
                    else
                    {
                        playerController.referenceObjects.footCollider.transform.position = playerAnimationController.vrIK.transform.position + new Vector3(0.0f, 0.4f, 0.0f);
                        StopClimbing();
                        movedFootColliderDown = true;
                    }
                }
                else //for ceiling
                {
                    RaycastHit ceilingHit;
                    if (tempDir.y > 0.0f && Physics.SphereCast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, 0.2f, Vector3.up, out ceilingHit, 0.2f, climbingMask))
                    {
                        Debug.Log("Touch ceiling, stop");
                        break;
                    }
                }
                //can make dis longer and put this with time check
                if (Physics.Raycast(playerController.referenceObjects.footCollider.transform.position, tempDir.normalized, playerController.referenceObjects.characterController.radius, climbingMask))
				{
					Debug.Log("FC hit wall in the dir, cancel");
					break;
				}
                
                playerController.referenceObjects.characterController.Move(tempDir * tempSpeed * Time.deltaTime);
				tempSpeed -= finalSpeedReduce;
				yield return 1;
			}

			if (!movedFootColliderDown)
			{
				if (Physics.Raycast(ReferenceManager.Instance.LocalVRPlayerCamera.transform.position, Vector3.down, playerHeight, climbingMask))
				{
					StartCoroutine("PushUpPlayerWhenfinish");
					Debug.Log("Still underground after final move, pushUp more");
				}
			}
			ResetValues();
			yield return null;
		}
		#endregion

		//is called from FreeHandController when grip is released
		public void ReleaseFromWall(bool left, bool _finalMove = true)
		{
			finalMove = _finalMove;
			//Debug.Log("ReleaseFromWall " + left);
			if (left)
			{
				lHandHolding = false;
				playerController.leftHandState = PlayerController.LeftHandState.Open;
			}
			else
			{
				rHandHolding = false;
				playerController.rightHandState = PlayerController.RightHandState.Open;
			}

			if (lHandHolding)
			{
				handPriority = HandPriority.LEFT;
			}
			else if (rHandHolding)
			{
				handPriority = HandPriority.RIGHT;
			}
			else
			{
				handPriority = HandPriority.NONE;

				if (finalMove)
				{
					StartCoroutine("LerpFinalMove", left);
				}
				else
				{
					StartCoroutine("PushUpPlayerWhenfinish");
					ResetValues();
				}
			}
		}

		private void FixedUpdate()
		{
			if (!photonView.isMine && PhotonNetwork.connected)
				return;

			if (playerController.playerState == PlayerController.PlayerState.Climbing)
			{
				playerController.referenceObjects.characterController.center = playerController.referenceObjects.footCollider.transform.localPosition;
			}
		}

		//These are things that happen when you do grab
		public void Climbing()
		{
			if (handPriority == HandPriority.NONE)
			{
				return;
			}

			playerController.referenceObjects.characterController.center = playerController.referenceObjects.footCollider.transform.localPosition;
			playerController.playerState = PlayerController.PlayerState.Climbing;

			UpdateControllerVel();
			UpdatePullPush();
			UpdateFinalForce();
			switch (handPriority)
			{
				case HandPriority.NONE:
					break;
				case HandPriority.LEFT:
					//remove offset
					playerController.referenceObjects.footFollow.offset = Vector3.zero + hitL.normal * 0.2f;
					playerController.leftHandState = PlayerController.LeftHandState.Climbing;
					MoveCharacter(true);
					break;
				case HandPriority.RIGHT:
					playerController.referenceObjects.footFollow.offset = Vector3.zero + hitR.normal * 0.2f;
					playerController.rightHandState = PlayerController.RightHandState.Climbing;
					MoveCharacter(false);
					break;
				default:
					break;
			}
		}

		//Moving your character after grabbing, CharacterController prevents you from moving into the wall
		private void MoveCharacter(bool left)
		{
			Vector3 relativeMove = cam.rotation * -controllerVel;

			float disHandWall = SignedDistancePriorityHandToWall();

			//project vector when hand outside wall
			//problem: when hand come back to wall, it push immediately -> impossble to pull back (hand will be too far from wall)
			//solution: only allow push when hand touch wall
			if (push && disHandWall > 0.0f)
			{
				if (handPriority == HandPriority.LEFT)
				{
					relativeMove = Vector3.ProjectOnPlane(relativeMove, hitL.normal);
				}
				else if (handPriority == HandPriority.RIGHT)
				{
					relativeMove = Vector3.ProjectOnPlane(relativeMove, hitR.normal);
				}
			}

            playerController.referenceObjects.characterController.Move(relativeMove * speed * playerController.enhancedSpeed * Time.deltaTime);
		}

		// -/+ means hand inside/outside wall
		private bool HandOutsideWall(bool left)
		{
			bool outsise = true;
			if (left)
			{
				if (Vector3.Angle(hitL.point - ReferenceManager.Instance.LocalVRPlayerLeftHand.transform.position, hitL.normal) < 90.0f)
				{
					outsise = false;
				}
			}
			else
			{
				if (Vector3.Angle(hitR.point - ReferenceManager.Instance.LocalVRPlayerRightHand.transform.position, hitR.normal) < 90.0f)
				{
					outsise = false;
				}
			}
			//Debug.Log("outside " + outsise);
			return outsise;
		}

		//work with small tree and large wall
		private float SignedDistancePriorityHandToWall()
		{
			float dis = 0.0f;
			if (handPriority == HandPriority.LEFT)
			{
				dis = Vector3.Distance(ReferenceManager.Instance.LocalVRPlayerLeftHand.transform.position, hitL.point);
				if (!HandOutsideWall(true))
				{
					dis = -dis;
				}
			}
			else if (handPriority == HandPriority.RIGHT)
			{
				dis = Vector3.Distance(ReferenceManager.Instance.LocalVRPlayerRightHand.transform.position, hitR.point);
				if (!HandOutsideWall(false))
				{
					dis = -dis;
				}
			}
			return dis;
		}

		private void UpdatePullPush()
		{
			float angle = 0.0f;

			if (handPriority == HandPriority.LEFT)
			{
				angle = Vector3.Angle(cam.rotation * controller.GetVel(true), hitL.normal);
			}
			else if (handPriority == HandPriority.RIGHT)
			{
				angle = Vector3.Angle(cam.rotation * controller.GetVel(false), hitR.normal);
			}

			if (angle < 90.0f)
			{
				push = false;
				pull = true;
			}
			else if (angle > 90.0f)
			{
				push = true;
				pull = false;
			}
			else //move to side
			{
				push = false;
				pull = false;
			}
		}

		private void UpdateControllerVel()
		{
			if (handPriority == HandPriority.LEFT)
			{
				controllerVel = controller.GetVel(true);
			}
			else if (handPriority == HandPriority.RIGHT)
			{
				controllerVel = controller.GetVel(false);
			}
			else
			{
				controllerVel = Vector3.zero;
			}
		}

		private void ResetValues()
		{
			lHandHolding = false;
			rHandHolding = false;
			pull = false;
			push = false;

			controllerVel = Vector3.zero;

			hitL = new RaycastHit();
			hitR = new RaycastHit();

			timeL = 0.0f;
			timeR = 0.0f;
			startPosL = Vector3.zero;
			startPosR = Vector3.zero;
			finalDirL = Vector3.zero;
			finalDirR = Vector3.zero;
		}
	}
}