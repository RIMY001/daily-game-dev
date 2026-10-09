using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;
using UnityEngine.InputSystem;

public class PlayerController : MonoBehaviour
{
    private enum Direction
    { 
        Up, Right, Left
    }
    private Rigidbody2D rb;
    private Animator anim;
    private SpriteRenderer sr;
    [Header("得分")]
    public int stepPoint;
    private int pointResult;
    [Header("跳跃")]
    public float jumpDistance;
    private float moveDistance;
    private Vector2 destination;
    private Vector2 touchPosition;
    private Direction dir;

    private bool buttonHeld;
    private bool isJump;
    private bool canJump;

    private bool isDead;
    
    private RaycastHit2D[] result = new RaycastHit2D[2];

    private void Awake()
    {
        rb = GetComponent<Rigidbody2D>();
        anim = GetComponent<Animator>();
        sr = GetComponent<SpriteRenderer>();
    }

    private void Update()
    {
        if (canJump)
        {
            TriggerJump();     
        }
    }

    private void FixedUpdate()
    {
        if(isJump)
            rb.position = Vector2.Lerp(transform.position, destination, 0.134f);
    }

    private void OnTriggerStay2D(Collider2D other)
    {
        if (other.CompareTag("Water") && !isJump)
        {
            Physics2D.RaycastNonAlloc(transform.position + Vector3.up * 0.1f, Vector2.zero, result);
            bool inWater = true;

            foreach (var hit in result)
            {
                if (hit.collider == null) continue;

                if (hit.collider.CompareTag("Wood"))
                {
                    //  TODO:跟随木板移动
                    transform.parent = hit.collider.transform;
                    inWater = false;
                }        
            }

            //  没有木板游戏结束
            if (inWater && !isJump)
            {
                isDead = true;
                UnityEngine.Debug.Log("GAME OVER!");
            }
        }

        if (other.CompareTag("Border") || other.CompareTag("Car"))
        {
            isDead = true;
        }

        if (!isJump && other.CompareTag("Obstacle"))
        {
            isDead = true;
        }
    }

    //private void OnTriggerExit2D(Collider2D other)
    //{
    //    if (other.CompareTag("Wood"))
    //    {
    //        transform.parent = null;
    //    }
    //}

    #region INPUT 输入回调函数
    public void Jump(InputAction.CallbackContext context)
    {
        //TODO:执行跳跃，跳跃的距离，记录分数，播放跳跃的音效
        if (context.phase == InputActionPhase.Performed && !isJump)
        {
            moveDistance = jumpDistance;
            //Debug.Log("JUMP!" +" "+ moveDistance);
            //执行跳跃
            
            canJump = true;
        }

        if (dir == Direction.Up && context.performed && !isJump)
        {
            pointResult += stepPoint;
        }
    }

    public void LongJump(InputAction.CallbackContext context)
    {
        if (context.performed && !isJump)
        {
            moveDistance = jumpDistance * 2;
            buttonHeld = true;
        }

        if (context.canceled && buttonHeld && !isJump)
        {
            //执行跳跃
            //Debug.Log("LONG JUMP!" + " " + moveDistance);
            if (dir == Direction.Up)
                pointResult += stepPoint * 2;
            buttonHeld = false;
            
            canJump = true;
        }
    }

    public void GetTouchPosition(InputAction.CallbackContext context)
    {
        if (context.performed)
        {
            //Debug.Log(context.ReadValue<Vector2>());
            touchPosition = Camera.main.ScreenToWorldPoint(context.ReadValue<Vector2>());
            //Debug.Log(touchPosition);
            var offset = ((Vector3)touchPosition - transform.position).normalized;
            if (Mathf.Abs(offset.x) <= 0.7f)
            {
                dir = Direction.Up;
            }
            else if (offset.x < 0)
            {
                dir = Direction.Left;
            }
            else if (offset.x > 0)
            {
                dir = Direction.Right;
            }
        }
    }

    #endregion

    /// <summary>
    /// 触发执行跳跃动画
    /// </summary>
    private void TriggerJump()
    {
        // 获得移动方向，播放动画
        canJump = false;
        switch (dir)
        {
            //  触发切换左右方向动画
            case Direction.Up:
                anim.SetBool("isSide", false);
                destination = new Vector2(transform.position.x, transform.position.y + moveDistance);
                transform.localScale = Vector3.one;
                break;
            case Direction.Right:
                anim.SetBool("isSide", true);
                destination = new Vector2(transform.position.x + moveDistance, transform.position.y);
                transform.localScale = new Vector3(-1,1,1);
                break;
            case Direction.Left:
                anim.SetBool("isSide", true);
                destination = new Vector2(transform.position.x - moveDistance, transform.position.y);
                transform.localScale = Vector3.one;
                break;
        }
        anim.SetTrigger("Jump");
    }

    #region Animation Even
    public void JumpAnimationEven()
    {
        //改变状态
        isJump = true;

        //修改排序图层
        sr.sortingLayerName = "Front";

        transform.parent = null;
        
    }

    public void FinishJumpAnimationEven()
    {
        isJump = false;
        //修改排序图层
        sr.sortingLayerName = "Middle";

        if (dir == Direction.Up && !isDead)
        {
            //  TODO:得分，触发地图检测
            //  FIXME
            //terrainManager.CheckPosition();

            EventHandler.CallGetPointEvent(pointResult);

            UnityEngine.Debug.Log("总得分：" + pointResult);
        }
    }

    #endregion
}
