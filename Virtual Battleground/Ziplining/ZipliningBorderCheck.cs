using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//How does this work:
//BoderCheck will offset translation and rotation 
//player is moved by distance between Left1 and Left2, don't use speed

//NOTE: don't use too high mulSpeed in Move, because it may move too far and push BorderCheck away from spline -> can't check for SphereCast anymore
//to increase speed, can move Left2, Right2, ... far away from Left, Right, ... in prefab OR increase mulSpeed
//may need to increase width of prefab, in case spline has extreme curve

//1.0f = maxSpeed

namespace Oneiric.Shooter
{
    public class ZipliningBorderCheck : MonoBehaviour
    {
        [SerializeField] private Transform left;
        [SerializeField] private Transform right;
        [SerializeField] private Transform up;
        [SerializeField] private Transform down;

        [SerializeField] private Transform left2;
        [SerializeField] private Transform right2;
        [SerializeField] private Transform up2;
        [SerializeField] private Transform down2;

        [SerializeField] private LayerMask zipliningMask;

        [SerializeField] private float defaultUpRate = 0.001f; //always increase up to Medium speed
        [SerializeField] private float slopeUpDownRate = 0.01f; //increase/decrease depend on slope
        [SerializeField] [Range(0, 1.0f)] private float minSpeed = 0.6f;
        [SerializeField] [Range(0, 1.0f)] private float mediumSpeed = 0.8f; //1.0f is maxSpeed
        [SerializeField] private float startLerpTime = 1.5f;
        [SerializeField] private float delayTime = 0.5f; //sometimes need a short time to adjust offset, don't move player during this time
        private float curTime = 0.0f;


        private bool needLerpSpeed = true;
        private float curSpeed = 0.0f;
        [SerializeField] private float mulSpeed;

        #region Variables to move
        //move forward/backward dir
        Vector3 generalDir = Vector3.zero;
        //offset translate dir
        Vector3 rightDir = Vector3.zero;
        Vector3 upDir = Vector3.zero;

        float upTranslate = 0.0f;
        float rightTranslate = 0.0f;
        float upRotate = 0.0f;
        float rightRotate = 0.0f;

        //for translate and rotate offset
        private float leftDis = 0.0f;
        private float rightDis = 0.0f;
        private float upDis = 0.0f;
        private float downDis = 0.0f;

        //for rotate offset
        private float rightDis2 = 0.0f;
        private float upDis2 = 0.0f;

        //dis bet. front and back pos, used to cal. angle and get rotate offset
        private float length = 0.0f;
        private Vector3 rotateCenter;
        private bool rotateRight = false;
        private bool rotateUp = false;

        private float width;
        private float sphereRadius;
        #endregion

        [HideInInspector] public bool getTarget = false;

        public CharacterController chaControler;
        [HideInInspector] public bool usingLeftHand = true;
        [HideInInspector]
        public bool moveForward = true;

        //adjust at the start to correct rotate
        private bool adjusted = false;
        private float adjustTime = 0.0f; //cancel if take too long, avoid stuck here

        private void Start()
        {
            length = Vector3.Distance(left2.position, left.position);
            width = Vector3.Distance(right.position, left.position);
            sphereRadius = width / 4.0f;
        }

        private void OnEnable()
        {
            getTarget = false;
            adjusted = false;
            adjustTime = 0.0f;
            curTime = 0.0f;
            curSpeed = 0.0f;
            needLerpSpeed = true;
        }

        private void OnDisable()
        {
            transform.eulerAngles = Vector3.zero;  
        }
        
        private void Update()
        {
            if (!getTarget && chaControler != null)
            {
                if (!adjusted)
                {
                    StartAdjusting();
                }
                else
                {
                    #region Translate by offset
                    UpdateTranslateOffsets();
                    rightDir = (left.position - right.position).normalized;
                    upDir = (down.position - up.position).normalized;

                    Vector3 tempVec = rightDir * rightTranslate + upDir * upTranslate;
                    Vector3 characterOffset = Vector3.zero;
                    if (usingLeftHand)
                    {
                        characterOffset = (this.transform.position - ReferenceManager.Instance.LocalVRPlayerLeftHand.transform.position);
                    }
                    else
                    {
                        characterOffset = (this.transform.position - ReferenceManager.Instance.LocalVRPlayerRightHand.transform.position);
                    }
                    chaControler.transform.position = chaControler.transform.position + characterOffset;
                    transform.Translate(tempVec, Space.World);
                    chaControler.Move(tempVec * Time.deltaTime);
                    #endregion

                    #region Rotate by offset
                    UpdateRotateOffsets();

                    rotateCenter = left.position + (right.position - left.position) / 2.0f; //mid point of L and R

                    if (rightRotate != 0.0f)
                    {
                        if (rotateRight)
                        {
                            transform.RotateAround(rotateCenter, (down.position - up.position).normalized, rightRotate);
                        }
                        else
                        {
                            transform.RotateAround(rotateCenter, (up.position - down.position).normalized, rightRotate);
                        }
                    }

                    if (upRotate != 0.0f)
                    {
                        if (rotateUp)
                        {
                            transform.RotateAround(rotateCenter, (right.position - left.position).normalized, upRotate);
                        }
                        else
                        {
                            transform.RotateAround(rotateCenter, (left.position - right.position).normalized, upRotate);
                        }
                    }

                    #endregion

                    #region Move Forward/Backward
                    UpdateSpeed();

                    generalDir = up2.position - up.position;
                    if (!moveForward)
                    {
                        generalDir = -generalDir;
                    }

                    transform.Translate(generalDir * curSpeed * mulSpeed * Time.deltaTime, Space.World);
                    chaControler.Move(generalDir * curSpeed * mulSpeed * Time.deltaTime);
                    #endregion
                }
            }
        }

        private void StartAdjusting()
        {
            adjustTime += Time.deltaTime;
            if (adjustTime >= 2.0f)
            {
                getTarget = true;
            }

            RaycastHit hitR;
            if (Physics.SphereCast(right.position, sphereRadius, left.position - right.position, out hitR, width, zipliningMask))
            {
                float offsetY = Vector3.Angle(hitR.normal, transform.right);
                rotateCenter = left.position + (right.position - left.position) / 2.0f; //mid point of L and R
                if (Vector3.Angle(transform.forward, hitR.normal) < 90.0f)
                {
                    transform.RotateAround(rotateCenter, (down.position - up.position).normalized, offsetY);
                }
                else
                {
                    transform.RotateAround(rotateCenter, (up.position - down.position).normalized, offsetY);
                }

                adjusted = true;
                float angle = Vector3.Angle(transform.forward, Camera.main.transform.forward);
                if (angle > 90.0f)
                {
                    moveForward = true;
                }
                else
                {
                    moveForward = false;
                }
            }
            else
            {
                transform.eulerAngles = new Vector3(0.0f, transform.rotation.eulerAngles.y + 5.0f, 0.0f);
            }
        }

        private void UpdateTranslateOffsets()
        {
            rightTranslate = 0.0f;
            upTranslate = 0.0f;

            RaycastHit hit;
            bool checkL, checkR, checkU, checkD;
            checkL = Physics.SphereCast(left.position, sphereRadius, right.position - left.position, out hit, width, zipliningMask);
            if (checkL)
            {
                leftDis = hit.distance;
            }
            checkR = Physics.SphereCast(right.position, sphereRadius, left.position - right.position, out hit, width, zipliningMask);
            if (checkR)
            {
                rightDis = hit.distance;
            }
            checkU = Physics.SphereCast(up.position, sphereRadius, down.position - up.position, out hit, width, zipliningMask);
            if (checkU)
            {
                upDis = hit.distance;
            }
            checkD = Physics.SphereCast(down.position, sphereRadius, up.position - down.position, out hit, width, zipliningMask);
            if (checkD)
            {
                downDis = hit.distance;
            }

            rightTranslate = (rightDis - leftDis) / 2.0f;
            upTranslate = (upDis - downDis) / 2.0f;
            if (!checkL || !checkR || !checkU || !checkD)
            {
                getTarget = true;
            }
        }

        private void UpdateRotateOffsets()
        {
            rightTranslate = 0.0f;
            upTranslate = 0.0f;
            upRotate = 0.0f;
            rightRotate = 0.0f;

            RaycastHit hit;
            RaycastHit hit2;
            if (Physics.SphereCast(right.position, sphereRadius, left.position - right.position, out hit, width, zipliningMask))
            {
                rightDis = hit.distance;
                if (Physics.SphereCast(right2.position, sphereRadius, left2.position - right2.position, out hit2, width, zipliningMask))
                {
                    rightDis2 = hit2.distance;
                    float tempDis = Mathf.Abs(rightDis2 - rightDis);
                    if (rightDis > rightDis2) //rotate L
                    {
                        rotateRight = false;
                    }
                    else //rotate R
                    {
                        rotateRight = true;
                    }
                    rightRotate = Mathf.Atan2(tempDis, length) * Mathf.Rad2Deg;
                }
                else
                {
                    getTarget = true;
                }
            }

            if (Physics.SphereCast(up.position, sphereRadius, down.position - up.position, out hit, width, zipliningMask))
            {
                upDis = hit.distance;
                if (Physics.SphereCast(up2.position, sphereRadius, down2.position - up2.position, out hit2, width, zipliningMask))
                {
                    upDis2 = hit2.distance;
                    float tempDis = Mathf.Abs(upDis2 - upDis);
                    if (upDis > upDis2) //rotate Down
                    {
                        rotateUp = false;
                    }
                    else //rotate Up
                    {
                        rotateUp = true;
                    }
                    upRotate = Mathf.Atan2(tempDis, length) * Mathf.Rad2Deg;
                }
                else
                {
                    getTarget = true;
                }
            }
        }

        private void UpdateSpeed()
        {
            //in beginning
            if (needLerpSpeed)
            {
                curTime += Time.deltaTime;
                if (curTime >= delayTime) //stop a little bit, sometimes isn't accurate 1st few frame
                {
                    curSpeed = mediumSpeed * (curTime - delayTime) / startLerpTime;
                }
                if ((curTime - delayTime) >= startLerpTime)
                {
                    needLerpSpeed = false;
                }
                curSpeed = Mathf.Clamp(curSpeed, 0.0f, mediumSpeed);
            }
            //inscrease/decrease speed when move up/down
            else
            {
                if (curSpeed < mediumSpeed)
                {
                    curSpeed += defaultUpRate;
                }

                if (moveForward)
                {
                    if (up.position.y < up2.position.y)
                    {
                        curSpeed -= slopeUpDownRate;
                    }
                    else
                    {
                        curSpeed += slopeUpDownRate;
                    }
                }
                else
                {
                    if (up.position.y > up2.position.y)
                    {
                        curSpeed -= slopeUpDownRate;
                    }
                    else
                    {
                        curSpeed += slopeUpDownRate;
                    }
                }
                curSpeed = Mathf.Clamp(curSpeed, minSpeed, 1.0f);
            }
        }
    }
}